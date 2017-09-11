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

static iotjs_i2c_t* iotjs_i2c_create(iotjs_jhandler_t* jhandler,
                                     const iotjs_jval_t* ji2c) {
  iotjs_i2c_t* i2c = IOTJS_ALLOC(iotjs_i2c_t);
  IOTJS_VALIDATED_STRUCT_CONSTRUCTOR(iotjs_i2c_t, i2c);
  i2c_create_platform_data(jhandler, i2c, &_this->platform_data);
  iotjs_jobjectwrap_initialize(&_this->jobjectwrap, ji2c,
                               &this_module_native_info);
  return i2c;
}

static iotjs_i2c_reqwrap_t* iotjs_i2c_reqwrap_create(
    const iotjs_jval_t* jcallback, iotjs_i2c_t* i2c, I2cOp op) {
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

const iotjs_jval_t* iotjs_i2c_reqwrap_jcallback(THIS) {
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

iotjs_i2c_t* iotjs_i2c_instance_from_jval(const iotjs_jval_t* ji2c) {
  iotjs_jobjectwrap_t* jobjectwrap = iotjs_jobjectwrap_from_jobject(ji2c);
  return (iotjs_i2c_t*)jobjectwrap;
}

void AfterI2CWork(uv_work_t* work_req, int status) {
  iotjs_i2c_reqwrap_t* req_wrap = iotjs_i2c_reqwrap_from_request(work_req);
  iotjs_i2c_reqdata_t* req_data = iotjs_i2c_reqwrap_data(req_wrap);

  iotjs_jargs_t jargs = iotjs_jargs_create(2);

  if (status) {
    iotjs_jval_t error = iotjs_jval_create_error("System error");
    iotjs_jargs_append_jval(&jargs, &error);
    iotjs_jval_destroy(&error);
  } else {
    switch (req_data->op) {
      case kI2cOpOpen: {
        if (req_data->error == kI2cErrOpen) {
          iotjs_jval_t error =
              iotjs_jval_create_error("Failed to open I2C device");
          iotjs_jargs_append_jval(&jargs, &error);
          iotjs_jval_destroy(&error);
        } else {
          iotjs_jargs_append_null(&jargs);
        }
        break;
      }
      case kI2cOpWrite: {
        if (req_data->error == kI2cErrWrite) {
          iotjs_jval_t error =
              iotjs_jval_create_error("Cannot write to device");
          iotjs_jargs_append_jval(&jargs, &error);
          iotjs_jval_destroy(&error);
        } else {
          iotjs_jargs_append_null(&jargs);
        }
        break;
      }
      case kI2cOpRead: {
        if (req_data->error == kI2cErrRead) {
          iotjs_jval_t error =
              iotjs_jval_create_error("Cannot read from device");
          iotjs_jargs_append_jval(&jargs, &error);
          iotjs_jargs_append_null(&jargs);
          iotjs_jval_destroy(&error);
        } else {
          iotjs_jargs_append_null(&jargs);
          iotjs_jval_t result =
              iotjs_jval_create_byte_array(req_data->buf_len,
                                           req_data->buf_data);
          iotjs_jargs_append_jval(&jargs, &result);
          iotjs_jval_destroy(&result);

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

  const iotjs_jval_t* jcallback = iotjs_i2c_reqwrap_jcallback(req_wrap);
  iotjs_make_callback(jcallback, iotjs_jval_get_undefined(), &jargs);

  iotjs_jargs_destroy(&jargs);
  iotjs_i2c_reqwrap_dispatched(req_wrap);
}

static void GetI2cArray(const iotjs_jval_t* jarray,
                        iotjs_i2c_reqdata_t* req_data) {
  // FIXME
  // Need to implement a function to get array info from iotjs_jval_t Array.
  iotjs_jval_t jlength =
      iotjs_jval_get_property(jarray, IOTJS_MAGIC_STRING_LENGTH);
  IOTJS_ASSERT(!iotjs_jval_is_undefined(&jlength));

  req_data->buf_len = iotjs_jval_as_number(&jlength);
  req_data->buf_data = iotjs_buffer_allocate(req_data->buf_len);

  for (uint8_t i = 0; i < req_data->buf_len; i++) {
    iotjs_jval_t jdata = iotjs_jval_get_property_by_index(jarray, i);
    req_data->buf_data[i] = iotjs_jval_as_number(&jdata);
    iotjs_jval_destroy(&jdata);
  }

  iotjs_jval_destroy(&jlength);
}

#define I2C_ASYNC(op)                                                  \
  do {                                                                 \
    uv_loop_t* loop = iotjs_environment_loop(iotjs_environment_get()); \
    uv_work_t* req = iotjs_i2c_reqwrap_req(req_wrap);                  \
    uv_queue_work(loop, req, op##Worker, AfterI2CWork);                \
  } while (0)

JHANDLER_FUNCTION(I2cCons) {
  DJHANDLER_CHECK_THIS(object);
  // Create I2C object
  const iotjs_jval_t* ji2c = JHANDLER_GET_THIS(object);
  iotjs_i2c_t* i2c = iotjs_i2c_create(jhandler, ji2c);
  IOTJS_ASSERT(i2c ==
               (iotjs_i2c_t*)(iotjs_jval_get_object_native_handle(ji2c)));

  // Create I2C request wrap
  const iotjs_jval_t* jcallback = JHANDLER_GET_ARG(1, function);
  iotjs_i2c_reqwrap_t* req_wrap =
      iotjs_i2c_reqwrap_create(jcallback, i2c, kI2cOpOpen);

  I2C_ASYNC(Open);
}

JHANDLER_FUNCTION(SetAddress) {
  JHANDLER_DECLARE_THIS_PTR(i2c, i2c);
  DJHANDLER_CHECK_ARGS(1, number);

  I2cSetAddress(i2c, JHANDLER_GET_ARG(0, number));

  iotjs_jhandler_return_null(jhandler);
}

JHANDLER_FUNCTION(Close) {
  JHANDLER_DECLARE_THIS_PTR(i2c, i2c);
  DJHANDLER_CHECK_ARGS(0);

  I2cClose(i2c);
  iotjs_i2c_destroy(i2c);

  iotjs_jhandler_return_null(jhandler);
}

JHANDLER_FUNCTION(Write) {
  JHANDLER_DECLARE_THIS_PTR(i2c, i2c);
  DJHANDLER_CHECK_ARGS(2, array, function);

  const iotjs_jval_t* jcallback = JHANDLER_GET_ARG(1, function);

  iotjs_i2c_reqwrap_t* req_wrap =
      iotjs_i2c_reqwrap_create(jcallback, i2c, kI2cOpWrite);
  iotjs_i2c_reqdata_t* req_data = iotjs_i2c_reqwrap_data(req_wrap);

  GetI2cArray(JHANDLER_GET_ARG(0, array), req_data);

  I2C_ASYNC(Write);

  iotjs_jhandler_return_null(jhandler);
}

JHANDLER_FUNCTION(Read) {
  JHANDLER_DECLARE_THIS_PTR(i2c, i2c);
  DJHANDLER_CHECK_ARGS(2, number, function);

  const iotjs_jval_t* jcallback = JHANDLER_GET_ARG(1, function);

  iotjs_i2c_reqwrap_t* req_wrap =
      iotjs_i2c_reqwrap_create(jcallback, i2c, kI2cOpRead);

  iotjs_i2c_reqdata_t* req_data = iotjs_i2c_reqwrap_data(req_wrap);
  req_data->buf_len = JHANDLER_GET_ARG(0, number);
  req_data->delay = 0;

  I2C_ASYNC(Read);

  iotjs_jhandler_return_null(jhandler);
}

iotjs_jval_t InitI2c() {
  iotjs_jval_t jI2cCons = iotjs_jval_create_function_with_dispatch(I2cCons);

  iotjs_jval_t prototype = iotjs_jval_create_object();

  iotjs_jval_set_method(&prototype, IOTJS_MAGIC_STRING_SETADDRESS, SetAddress);
  iotjs_jval_set_method(&prototype, IOTJS_MAGIC_STRING_CLOSE, Close);
  iotjs_jval_set_method(&prototype, IOTJS_MAGIC_STRING_WRITE, Write);
  iotjs_jval_set_method(&prototype, IOTJS_MAGIC_STRING_READ, Read);

  iotjs_jval_set_property_jval(&jI2cCons, IOTJS_MAGIC_STRING_PROTOTYPE,
                               &prototype);

  iotjs_jval_destroy(&prototype);

  return jI2cCons;
}
