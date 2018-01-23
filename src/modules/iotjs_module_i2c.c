/* Copyright 2016-present Samsung Electronics Co., Ltd. and other contributors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#include "iotjs_def.h"
#include "iotjs_module_i2c.h"


IOTJS_DEFINE_NATIVE_HANDLE_INFO_THIS_MODULE(i2c);

static iotjs_i2c_t* i2c_create(const jerry_value_t ji2c) {
  iotjs_i2c_t* i2c = IOTJS_ALLOC(iotjs_i2c_t);
  iotjs_i2c_create_platform_data(i2c);
  i2c->jobject = ji2c;
  jerry_set_object_native_pointer(ji2c, i2c, &this_module_native_info);

  return i2c;
}

static iotjs_i2c_reqwrap_t* i2c_reqwrap_create(const jerry_value_t jcallback,
                                               iotjs_i2c_t* i2c, I2cOp op) {
  iotjs_i2c_reqwrap_t* i2c_reqwrap = IOTJS_ALLOC(iotjs_i2c_reqwrap_t);

  iotjs_reqwrap_initialize(&i2c_reqwrap->reqwrap, jcallback,
                           (uv_req_t*)&i2c_reqwrap->req);

  i2c_reqwrap->req_data.op = op;
  i2c_reqwrap->i2c_data = i2c;
  return i2c_reqwrap;
}

static void i2c_reqwrap_destroy(iotjs_i2c_reqwrap_t* i2c_reqwrap) {
  iotjs_reqwrap_destroy(&i2c_reqwrap->reqwrap);
  IOTJS_RELEASE(i2c_reqwrap);
}

static void iotjs_i2c_destroy(iotjs_i2c_t* i2c) {
  iotjs_i2c_destroy_platform_data(i2c->platform_data);
  IOTJS_RELEASE(i2c);
}

static void i2c_worker(uv_work_t* work_req) {
  iotjs_i2c_reqwrap_t* req_wrap =
      (iotjs_i2c_reqwrap_t*)(iotjs_reqwrap_from_request((uv_req_t*)work_req));
  iotjs_i2c_reqdata_t* req_data = &req_wrap->req_data;
  iotjs_i2c_t* i2c = req_wrap->i2c_data;

  switch (req_data->op) {
    case kI2cOpOpen:
      req_data->result = iotjs_i2c_open(i2c);
      break;
    case kI2cOpWrite:
      req_data->result = iotjs_i2c_write(i2c);
      break;
    case kI2cOpRead:
      req_data->result = iotjs_i2c_read(i2c);
      break;
    case kI2cOpClose:
      req_data->result = iotjs_i2c_close(i2c);
      break;
    default:
      IOTJS_ASSERT(!"Invalid Operation");
  }
}

static const char* i2c_error_str(int op) {
  switch (op) {
    case kI2cOpOpen:
      return "Open error, cannot open I2C";
    case kI2cOpWrite:
      return "Write error, cannot write I2C";
    case kI2cOpRead:
      return "Read error, cannot read I2C";
    case kI2cOpClose:
      return "Close error, cannot close I2C";
    default:
      return "Unknown error";
  }
}

static void i2c_after_worker(uv_work_t* work_req, int status) {
  iotjs_i2c_reqwrap_t* req_wrap =
      (iotjs_i2c_reqwrap_t*)(iotjs_reqwrap_from_request((uv_req_t*)work_req));
  iotjs_i2c_reqdata_t* req_data = &req_wrap->req_data;

  iotjs_jargs_t jargs = iotjs_jargs_create(2);

  if (status) {
    iotjs_jargs_append_error(&jargs, "System error");
  } else {
    switch (req_data->op) {
      case kI2cOpOpen:
      case kI2cOpWrite:
      case kI2cOpClose: {
        if (!req_data->result) {
          iotjs_jargs_append_error(&jargs, i2c_error_str(req_data->op));
        } else {
          iotjs_jargs_append_null(&jargs);
        }
        break;
      }
      case kI2cOpRead: {
        if (!req_data->result) {
          iotjs_jargs_append_error(&jargs, i2c_error_str(req_data->op));
        } else {
          iotjs_i2c_t* i2c = req_wrap->i2c_data;

          iotjs_jargs_append_null(&jargs);
          jerry_value_t result =
              iotjs_jval_create_byte_array(i2c->buf_len, i2c->buf_data);
          iotjs_jargs_append_jval(&jargs, result);
          jerry_release_value(result);

          if (i2c->buf_data != NULL) {
            iotjs_buffer_release(i2c->buf_data);
          }
        }
        break;
      }
      default: {
        IOTJS_ASSERT(!"Unreachable");
        break;
      }
    }
  }

  const jerry_value_t jcallback = iotjs_reqwrap_jcallback(&req_wrap->reqwrap);
  if (jerry_value_is_function(jcallback)) {
    iotjs_make_callback(jcallback, jerry_create_undefined(), &jargs);
  }

  iotjs_jargs_destroy(&jargs);
  i2c_reqwrap_destroy(req_wrap);
}

#define I2C_CALL_ASYNC(op, jcallback)                                       \
  do {                                                                      \
    uv_loop_t* loop = iotjs_environment_loop(iotjs_environment_get());      \
    iotjs_i2c_reqwrap_t* req_wrap = i2c_reqwrap_create(jcallback, i2c, op); \
    uv_work_t* req = &req_wrap->req;                                        \
    uv_queue_work(loop, req, i2c_worker, i2c_after_worker);                 \
  } while (0)

JS_FUNCTION(I2cCons) {
  DJS_CHECK_THIS();
  DJS_CHECK_ARGS(1, object);
  DJS_CHECK_ARG_IF_EXIST(1, function);

  // Create I2C object
  const jerry_value_t ji2c = JS_GET_THIS();
  iotjs_i2c_t* i2c = i2c_create(ji2c);

  jerry_value_t jconfig;
  JS_GET_REQUIRED_ARG_VALUE(0, jconfig, IOTJS_MAGIC_STRING_CONFIG, object);

  jerry_value_t res = iotjs_i2c_set_platform_config(i2c, jconfig);
  if (jerry_value_has_error_flag(res)) {
    return res;
  }

  DJS_GET_REQUIRED_CONF_VALUE(jconfig, i2c->address, IOTJS_MAGIC_STRING_ADDRESS,
                              number);

  jerry_value_t jcallback = JS_GET_ARG_IF_EXIST(1, function);

  // If the callback doesn't exist, it is completed synchronously.
  // Otherwise, it will be executed asynchronously.
  if (!jerry_value_is_null(jcallback)) {
    I2C_CALL_ASYNC(kI2cOpOpen, jcallback);
  } else if (!iotjs_i2c_open(i2c)) {
    return JS_CREATE_ERROR(COMMON, "I2C Error: cannot open I2C");
  }

  return jerry_create_undefined();
}

JS_FUNCTION(Close) {
  JS_DECLARE_THIS_PTR(i2c, i2c);
  DJS_CHECK_ARG_IF_EXIST(1, function);

  I2C_CALL_ASYNC(kI2cOpClose, JS_GET_ARG_IF_EXIST(0, function));

  return jerry_create_undefined();
}

JS_FUNCTION(CloseSync) {
  JS_DECLARE_THIS_PTR(i2c, i2c);

  if (!iotjs_i2c_close(i2c)) {
    return JS_CREATE_ERROR(COMMON, "I2C Error: cannot close");
  }

  return jerry_create_undefined();
}

static jerry_value_t i2c_write(iotjs_i2c_t* i2c, const jerry_value_t jargv[],
                               const jerry_length_t jargc, bool async) {
  jerry_value_t jarray;
  JS_GET_REQUIRED_ARG_VALUE(0, jarray, IOTJS_MAGIC_STRING_DATA, array);

  // Set buffer length and data from jarray
  i2c->buf_len = jerry_get_array_length(jarray);
  i2c->buf_data = iotjs_buffer_allocate_from_number_array(i2c->buf_len, jarray);

  if (async) {
    I2C_CALL_ASYNC(kI2cOpWrite, JS_GET_ARG_IF_EXIST(1, function));
  } else {
    if (!iotjs_i2c_write(i2c)) {
      return JS_CREATE_ERROR(COMMON, "I2C Error: writeSync");
    }
  }

  return jerry_create_undefined();
}

JS_FUNCTION(Write) {
  JS_DECLARE_THIS_PTR(i2c, i2c);
  DJS_CHECK_ARGS(1, array);
  DJS_CHECK_ARG_IF_EXIST(1, function);

  return i2c_write(i2c, jargv, jargc, true);
}

JS_FUNCTION(WriteSync) {
  JS_DECLARE_THIS_PTR(i2c, i2c);
  DJS_CHECK_ARGS(1, array);

  return i2c_write(i2c, jargv, jargc, false);
}

JS_FUNCTION(Read) {
  JS_DECLARE_THIS_PTR(i2c, i2c);
  DJS_CHECK_ARGS(1, number);
  DJS_CHECK_ARG_IF_EXIST(1, function);

  JS_GET_REQUIRED_ARG_VALUE(0, i2c->buf_len, IOTJS_MAGIC_STRING_LENGTH, number);

  jerry_value_t jcallback = JS_GET_ARG_IF_EXIST(1, function);
  I2C_CALL_ASYNC(kI2cOpRead, jcallback);

  return jerry_create_undefined();
}

JS_FUNCTION(ReadSync) {
  JS_DECLARE_THIS_PTR(i2c, i2c);
  DJS_CHECK_ARGS(1, number);

  JS_GET_REQUIRED_ARG_VALUE(0, i2c->buf_len, IOTJS_MAGIC_STRING_LENGTH, number);

  if (!iotjs_i2c_read(i2c)) {
    return JS_CREATE_ERROR(COMMON, "I2C Error: readSync");
  }

  return iotjs_jval_create_byte_array(i2c->buf_len, i2c->buf_data);
}

jerry_value_t InitI2c() {
  jerry_value_t ji2c_cons = jerry_create_external_function(I2cCons);

  jerry_value_t prototype = jerry_create_object();

  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_CLOSE, Close);
  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_CLOSESYNC, CloseSync);
  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_WRITE, Write);
  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_WRITESYNC, WriteSync);
  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_READ, Read);
  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_READSYNC, ReadSync);

  iotjs_jval_set_property_jval(ji2c_cons, IOTJS_MAGIC_STRING_PROTOTYPE,
                               prototype);

  jerry_release_value(prototype);

  return ji2c_cons;
}
