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


#define THIS iotjs_i2c_reqwrap_t* i2c_reqwrap

static JNativeInfoType this_module_native_info = {.free_cb = NULL };

static void i2c_destroy_data(iotjs_i2c_t* i2c) {
  IOTJS_DECLARE_THIS(iotjs_i2c_t, i2c);
  i2c_destroy_platform_data(_this->platform_data);
}

static iotjs_i2c_t* iotjs_i2c_create(void* device, const iotjs_jval_t ji2c) {
  iotjs_i2c_t* i2c = IOTJS_ALLOC(iotjs_i2c_t);
  IOTJS_VALIDATED_STRUCT_CONSTRUCTOR(iotjs_i2c_t, i2c);
  i2c_create_platform_data(device, i2c, &_this->platform_data);
  iotjs_jobjectwrap_initialize(&_this->jobjectwrap, ji2c,
                               &this_module_native_info);
  return i2c;
}

static iotjs_i2c_reqwrap_t* iotjs_i2c_reqwrap_create(
    const iotjs_jval_t jcallback, iotjs_i2c_t* i2c, I2cOp op) {
  iotjs_i2c_reqwrap_t* i2c_reqwrap = IOTJS_ALLOC(iotjs_i2c_reqwrap_t);
  IOTJS_VALIDATED_STRUCT_CONSTRUCTOR(iotjs_i2c_reqwrap_t, i2c_reqwrap);

  iotjs_reqwrap_initialize(&_this->reqwrap, jcallback, (uv_req_t*)&_this->req);

  _this->req_data.op = op;
  _this->i2c_data = i2c;
  return i2c_reqwrap;
}

static void iotjs_i2c_reqwrap_destroy(THIS) {
  IOTJS_VALIDATED_STRUCT_DESTRUCTOR(iotjs_i2c_reqwrap_t, i2c_reqwrap);
  iotjs_reqwrap_destroy(&_this->reqwrap);
  IOTJS_RELEASE(i2c_reqwrap);
}

void iotjs_i2c_reqwrap_dispatched(THIS) {
  IOTJS_VALIDATABLE_STRUCT_METHOD_VALIDATE(iotjs_i2c_reqwrap_t, i2c_reqwrap);
  iotjs_i2c_reqwrap_destroy(i2c_reqwrap);
}

uv_work_t* iotjs_i2c_reqwrap_req(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_i2c_reqwrap_t, i2c_reqwrap);
  return &_this->req;
}

iotjs_jval_t iotjs_i2c_reqwrap_jcallback(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_i2c_reqwrap_t, i2c_reqwrap);
  return iotjs_reqwrap_jcallback(&_this->reqwrap);
}

iotjs_i2c_reqwrap_t* iotjs_i2c_reqwrap_from_request(uv_work_t* req) {
  return (iotjs_i2c_reqwrap_t*)(iotjs_reqwrap_from_request((uv_req_t*)req));
}

iotjs_i2c_reqdata_t* iotjs_i2c_reqwrap_data(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_i2c_reqwrap_t, i2c_reqwrap);
  return &_this->req_data;
}

iotjs_i2c_t* iotjs_i2c_instance_from_reqwrap(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_i2c_reqwrap_t, i2c_reqwrap);
  return _this->i2c_data;
}
#undef THIS

static void iotjs_i2c_destroy(iotjs_i2c_t* i2c) {
  IOTJS_VALIDATED_STRUCT_DESTRUCTOR(iotjs_i2c_t, i2c);
  iotjs_jobjectwrap_destroy(&_this->jobjectwrap);
  i2c_destroy_data(i2c);
  IOTJS_RELEASE(i2c);
}

iotjs_i2c_t* iotjs_i2c_instance_from_jval(const iotjs_jval_t ji2c) {
  iotjs_jobjectwrap_t* jobjectwrap = iotjs_jobjectwrap_from_jobject(ji2c);
  return (iotjs_i2c_t*)jobjectwrap;
}

void AfterI2CWork(uv_work_t* work_req, int status) {
  iotjs_i2c_reqwrap_t* req_wrap = iotjs_i2c_reqwrap_from_request(work_req);
  iotjs_i2c_reqdata_t* req_data = iotjs_i2c_reqwrap_data(req_wrap);

  iotjs_jargs_t jargs = iotjs_jargs_create(2);

  if (status) {
    iotjs_jval_t error = iotjs_jval_create_error("System error");
    iotjs_jargs_append_jval(&jargs, error);
    jerry_release_value(error);
  } else {
    switch (req_data->op) {
      case kI2cOpOpen: {
        if (req_data->error == kI2cErrOpen) {
          iotjs_jval_t error =
              iotjs_jval_create_error("Failed to open I2C device");
          iotjs_jargs_append_jval(&jargs, error);
          jerry_release_value(error);
        } else {
          iotjs_jargs_append_null(&jargs);
        }
        break;
      }
      case kI2cOpWrite: {
        if (req_data->error == kI2cErrWrite) {
          iotjs_jval_t error =
              iotjs_jval_create_error("Cannot write to device");
          iotjs_jargs_append_jval(&jargs, error);
          jerry_release_value(error);
        } else {
          iotjs_jargs_append_null(&jargs);
        }
        break;
      }
      case kI2cOpRead: {
        if (req_data->error == kI2cErrRead) {
          iotjs_jval_t error =
              iotjs_jval_create_error("Cannot read from device");
          iotjs_jargs_append_jval(&jargs, error);
          iotjs_jargs_append_null(&jargs);
          jerry_release_value(error);
        } else {
          iotjs_jargs_append_null(&jargs);
          iotjs_jval_t result =
              iotjs_jval_create_byte_array(req_data->buf_len,
                                           req_data->buf_data);
          iotjs_jargs_append_jval(&jargs, result);
          jerry_release_value(result);

          if (req_data->delay > 0) {
            uv_sleep(req_data->delay);
          }

          if (req_data->buf_data != NULL) {
            iotjs_buffer_release(req_data->buf_data);
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

  const iotjs_jval_t jcallback = iotjs_i2c_reqwrap_jcallback(req_wrap);
  iotjs_make_callback(jcallback, jerry_create_undefined(), &jargs);

  iotjs_jargs_destroy(&jargs);
  iotjs_i2c_reqwrap_dispatched(req_wrap);
}

static void GetI2cArray(const iotjs_jval_t jarray,
                        iotjs_i2c_reqdata_t* req_data) {
  // FIXME
  // Need to implement a function to get array info from iotjs_jval_t Array.
  iotjs_jval_t jlength =
      iotjs_jval_get_property(jarray, IOTJS_MAGIC_STRING_LENGTH);
  IOTJS_ASSERT(!jerry_value_is_undefined(jlength));

  req_data->buf_len = iotjs_jval_as_number(jlength);
  req_data->buf_data = iotjs_buffer_allocate(req_data->buf_len);

  for (uint8_t i = 0; i < req_data->buf_len; i++) {
    iotjs_jval_t jdata = iotjs_jval_get_property_by_index(jarray, i);
    req_data->buf_data[i] = iotjs_jval_as_number(jdata);
    jerry_release_value(jdata);
  }

  jerry_release_value(jlength);
}

#define I2C_ASYNC(op)                                                  \
  do {                                                                 \
    uv_loop_t* loop = iotjs_environment_loop(iotjs_environment_get()); \
    uv_work_t* req = iotjs_i2c_reqwrap_req(req_wrap);                  \
    uv_queue_work(loop, req, op##Worker, AfterI2CWork);                \
  } while (0)

JS_FUNCTION(I2cCons) {
  DJS_CHECK_THIS();
  // Create I2C object
  const iotjs_jval_t ji2c = JS_GET_THIS();
#ifdef __linux__
  DJS_CHECK_ARGS(2, string, function);
  iotjs_string_t device = JS_GET_ARG(0, string);
#else
  DJS_CHECK_ARGS(2, number, function);
  int device = JS_GET_ARG(0, number);
#endif
  iotjs_i2c_t* i2c = iotjs_i2c_create(&device, ji2c);

  IOTJS_ASSERT(i2c ==
               (iotjs_i2c_t*)(iotjs_jval_get_object_native_handle(ji2c)));

  // Create I2C request wrap
  const iotjs_jval_t jcallback = JS_GET_ARG(1, function);
  iotjs_i2c_reqwrap_t* req_wrap =
      iotjs_i2c_reqwrap_create(jcallback, i2c, kI2cOpOpen);

  I2C_ASYNC(Open);

  return jerry_create_undefined();
}

JS_FUNCTION(SetAddress) {
  JS_DECLARE_THIS_PTR(i2c, i2c);
  DJS_CHECK_ARGS(1, number);

  I2cSetAddress(i2c, JS_GET_ARG(0, number));

  return jerry_create_null();
}

JS_FUNCTION(Close) {
  JS_DECLARE_THIS_PTR(i2c, i2c);

  I2cClose(i2c);
  iotjs_i2c_destroy(i2c);

  return jerry_create_null();
}

JS_FUNCTION(Write) {
  JS_DECLARE_THIS_PTR(i2c, i2c);
  DJS_CHECK_ARGS(2, array, function);

  const iotjs_jval_t jcallback = JS_GET_ARG(1, function);

  iotjs_i2c_reqwrap_t* req_wrap =
      iotjs_i2c_reqwrap_create(jcallback, i2c, kI2cOpWrite);
  iotjs_i2c_reqdata_t* req_data = iotjs_i2c_reqwrap_data(req_wrap);

  GetI2cArray(JS_GET_ARG(0, array), req_data);

  I2C_ASYNC(Write);

  return jerry_create_null();
}

JS_FUNCTION(Read) {
  JS_DECLARE_THIS_PTR(i2c, i2c);
  DJS_CHECK_ARGS(2, number, function);

  const iotjs_jval_t jcallback = JS_GET_ARG(1, function);

  iotjs_i2c_reqwrap_t* req_wrap =
      iotjs_i2c_reqwrap_create(jcallback, i2c, kI2cOpRead);

  iotjs_i2c_reqdata_t* req_data = iotjs_i2c_reqwrap_data(req_wrap);
  req_data->buf_len = JS_GET_ARG(0, number);
  req_data->delay = 0;

  I2C_ASYNC(Read);

  return jerry_create_null();
}

iotjs_jval_t InitI2c() {
  iotjs_jval_t jI2cCons = jerry_create_external_function(I2cCons);

  iotjs_jval_t prototype = jerry_create_object();

  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_SETADDRESS, SetAddress);
  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_CLOSE, Close);
  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_WRITE, Write);
  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_READ, Read);

  iotjs_jval_set_property_jval(jI2cCons, IOTJS_MAGIC_STRING_PROTOTYPE,
                               prototype);

  jerry_release_value(prototype);

  return jI2cCons;
}
