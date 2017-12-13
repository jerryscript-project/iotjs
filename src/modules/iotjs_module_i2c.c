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
#include "iotjs_objectwrap.h"


IOTJS_DEFINE_NATIVE_HANDLE_INFO_THIS_MODULE(i2c);

static iotjs_i2c_t* iotjs_i2c_create(const jerry_value_t ji2c) {
  iotjs_i2c_t* i2c = IOTJS_ALLOC(iotjs_i2c_t);
  IOTJS_VALIDATED_STRUCT_CONSTRUCTOR(iotjs_i2c_t, i2c);
  i2c_create_platform_data(i2c);
  iotjs_jobjectwrap_initialize(&_this->jobjectwrap, ji2c,
                               &this_module_native_info);
  return i2c;
}

static iotjs_i2c_reqwrap_t* iotjs_i2c_reqwrap_create(
    const jerry_value_t jcallback, iotjs_i2c_t* i2c, I2cOp op) {
  iotjs_i2c_reqwrap_t* i2c_reqwrap = IOTJS_ALLOC(iotjs_i2c_reqwrap_t);
  IOTJS_VALIDATED_STRUCT_CONSTRUCTOR(iotjs_i2c_reqwrap_t, i2c_reqwrap);

  iotjs_reqwrap_initialize(&_this->reqwrap, jcallback, (uv_req_t*)&_this->req);

  _this->req_data.op = op;
  _this->i2c_data = i2c;
  return i2c_reqwrap;
}

static void iotjs_i2c_reqwrap_destroy(iotjs_i2c_reqwrap_t* i2c_reqwrap) {
  IOTJS_VALIDATED_STRUCT_DESTRUCTOR(iotjs_i2c_reqwrap_t, i2c_reqwrap);
  iotjs_reqwrap_destroy(&_this->reqwrap);
  IOTJS_RELEASE(i2c_reqwrap);
}

void iotjs_i2c_reqwrap_dispatched(iotjs_i2c_reqwrap_t* i2c_reqwrap) {
  IOTJS_VALIDATABLE_STRUCT_METHOD_VALIDATE(iotjs_i2c_reqwrap_t, i2c_reqwrap);
  iotjs_i2c_reqwrap_destroy(i2c_reqwrap);
}

uv_work_t* iotjs_i2c_reqwrap_req(iotjs_i2c_reqwrap_t* i2c_reqwrap) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_i2c_reqwrap_t, i2c_reqwrap);
  return &_this->req;
}

jerry_value_t iotjs_i2c_reqwrap_jcallback(iotjs_i2c_reqwrap_t* i2c_reqwrap) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_i2c_reqwrap_t, i2c_reqwrap);
  return iotjs_reqwrap_jcallback(&_this->reqwrap);
}

iotjs_i2c_reqwrap_t* iotjs_i2c_reqwrap_from_request(uv_work_t* req) {
  return (iotjs_i2c_reqwrap_t*)(iotjs_reqwrap_from_request((uv_req_t*)req));
}

iotjs_i2c_reqdata_t* iotjs_i2c_reqwrap_data(iotjs_i2c_reqwrap_t* i2c_reqwrap) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_i2c_reqwrap_t, i2c_reqwrap);
  return &_this->req_data;
}

iotjs_i2c_t* iotjs_i2c_instance_from_reqwrap(iotjs_i2c_reqwrap_t* i2c_reqwrap) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_i2c_reqwrap_t, i2c_reqwrap);
  return _this->i2c_data;
}
#undef THIS

void iotjs_i2c_destroy(iotjs_i2c_t* i2c) {
  IOTJS_VALIDATED_STRUCT_DESTRUCTOR(iotjs_i2c_t, i2c);
  iotjs_jobjectwrap_destroy(&_this->jobjectwrap);
  i2c_destroy_platform_data(_this->platform_data);
  IOTJS_RELEASE(i2c);
}

iotjs_i2c_t* iotjs_i2c_instance_from_jval(const jerry_value_t ji2c) {
  iotjs_jobjectwrap_t* jobjectwrap = iotjs_jobjectwrap_from_jobject(ji2c);
  return (iotjs_i2c_t*)jobjectwrap;
}

static void i2c_worker(uv_work_t* work_req) {
  iotjs_i2c_reqwrap_t* req_wrap = iotjs_i2c_reqwrap_from_request(work_req);
  iotjs_i2c_reqdata_t* req_data = iotjs_i2c_reqwrap_data(req_wrap);
  iotjs_i2c_t* i2c = iotjs_i2c_instance_from_reqwrap(req_wrap);

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
      return "Open error, cannot to open I2C";
    case kI2cOpWrite:
      return "Write error, cannot to write I2C";
    case kI2cOpRead:
      return "Read error, cannot to read I2C";
    case kI2cOpClose:
      return "Close error, cannot to close I2C";
    default:
      return "Unknown error";
  }
}

static void after_i2c_worker(uv_work_t* work_req, int status) {
  iotjs_i2c_reqwrap_t* req_wrap = iotjs_i2c_reqwrap_from_request(work_req);
  iotjs_i2c_reqdata_t* req_data = iotjs_i2c_reqwrap_data(req_wrap);

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
          iotjs_i2c_t* i2c = iotjs_i2c_instance_from_reqwrap(req_wrap);
          IOTJS_VALIDATED_STRUCT_METHOD(iotjs_i2c_t, i2c);

          iotjs_jargs_append_null(&jargs);
          jerry_value_t result =
              iotjs_jval_create_byte_array(_this->buf_len, _this->buf_data);
          iotjs_jargs_append_jval(&jargs, result);
          jerry_release_value(result);

          if (_this->buf_data != NULL) {
            iotjs_buffer_release(_this->buf_data);
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

  const jerry_value_t jcallback = iotjs_i2c_reqwrap_jcallback(req_wrap);
  iotjs_make_callback(jcallback, jerry_create_undefined(), &jargs);

  iotjs_jargs_destroy(&jargs);
  iotjs_i2c_reqwrap_dispatched(req_wrap);
}

#define I2C_CALL_FUNC(fn, op, jcallback)                                 \
  do {                                                                   \
    if (jerry_value_is_null(jcallback)) {                                \
      if (!iotjs_i2c_##fn(i2c)) {                                        \
        return JS_CREATE_ERROR(COMMON, "I2C Error: " #fn);               \
      }                                                                  \
    } else {                                                             \
      uv_loop_t* loop = iotjs_environment_loop(iotjs_environment_get()); \
      iotjs_i2c_reqwrap_t* req_wrap =                                    \
          iotjs_i2c_reqwrap_create(jcallback, i2c, op);                  \
      uv_work_t* req = iotjs_i2c_reqwrap_req(req_wrap);                  \
      uv_queue_work(loop, req, i2c_worker, after_i2c_worker);            \
    }                                                                    \
  } while (0)

JS_FUNCTION(I2cCons) {
  DJS_CHECK_THIS();

  // Create I2C object
  const jerry_value_t ji2c = JS_GET_THIS();
  iotjs_i2c_t* i2c = iotjs_i2c_create(ji2c);
  IOTJS_ASSERT(i2c == iotjs_i2c_instance_from_jval(ji2c));
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_i2c_t, i2c);

  jerry_value_t jconfig;
  JS_GET_REQUIRED_ARG_VALUE(0, jconfig, IOTJS_MAGIC_STRING_CONFIG, object);

  jerry_value_t res = iotjs_i2c_set_platform_config(i2c, jconfig);
  if (jerry_value_has_error_flag(res)) {
    return res;
  }

  DJS_GET_REQUIRED_CONF_VALUE(jconfig, _this->address,
                              IOTJS_MAGIC_STRING_ADDRESS, number);

  I2C_CALL_FUNC(open, kI2cOpOpen, JS_GET_ARG_IF_EXIST(1, function));

  return jerry_create_undefined();
}

JS_FUNCTION(Close) {
  JS_DECLARE_THIS_PTR(i2c, i2c);

  I2C_CALL_FUNC(close, kI2cOpClose, JS_GET_ARG_IF_EXIST(0, function));

  return jerry_create_undefined();
}

JS_FUNCTION(Write) {
  JS_DECLARE_THIS_PTR(i2c, i2c);
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_i2c_t, i2c);

  jerry_value_t jarray;
  JS_GET_REQUIRED_ARG_VALUE(0, jarray, IOTJS_MAGIC_STRING_DATA, array);

  // Set buffer length and data from jarray
  _this->buf_len = jerry_get_array_length(jarray);
  _this->buf_data =
      iotjs_buffer_allocate_from_number_array(_this->buf_len, jarray);

  I2C_CALL_FUNC(write, kI2cOpWrite, JS_GET_ARG_IF_EXIST(1, function));

  return jerry_create_undefined();
}

JS_FUNCTION(Read) {
  JS_DECLARE_THIS_PTR(i2c, i2c);
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_i2c_t, i2c);

  JS_GET_REQUIRED_ARG_VALUE(0, _this->buf_len, IOTJS_MAGIC_STRING_LENGTH,
                            number);

  jerry_value_t jcallback = JS_GET_ARG_IF_EXIST(1, function);
  I2C_CALL_FUNC(read, kI2cOpRead, jcallback);

  if (jerry_value_is_null(jcallback)) {
    return iotjs_jval_create_byte_array(_this->buf_len, _this->buf_data);
  }
  return jerry_create_undefined();
}

jerry_value_t InitI2c() {
  jerry_value_t jI2cCons = jerry_create_external_function(I2cCons);

  jerry_value_t prototype = jerry_create_object();

  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_CLOSE, Close);
  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_WRITE, Write);
  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_READ, Read);

  iotjs_jval_set_property_jval(jI2cCons, IOTJS_MAGIC_STRING_PROTOTYPE,
                               prototype);

  jerry_release_value(prototype);

  return jI2cCons;
}
