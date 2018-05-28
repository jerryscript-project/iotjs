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

IOTJS_DEFINE_PERIPH_CREATE_FUNCTION(i2c);

static void iotjs_i2c_destroy(iotjs_i2c_t* i2c) {
  iotjs_i2c_destroy_platform_data(i2c->platform_data);
  IOTJS_RELEASE(i2c);
}

static void i2c_worker(uv_work_t* work_req) {
  iotjs_periph_reqwrap_t* req_wrap =
      (iotjs_periph_reqwrap_t*)(iotjs_reqwrap_from_request(
          (uv_req_t*)work_req));
  iotjs_i2c_t* i2c = (iotjs_i2c_t*)req_wrap->data;

  switch (req_wrap->op) {
    case kI2cOpOpen:
      req_wrap->result = iotjs_i2c_open(i2c);
      break;
    case kI2cOpWrite:
      req_wrap->result = iotjs_i2c_write(i2c);
      break;
    case kI2cOpRead:
      req_wrap->result = iotjs_i2c_read(i2c);
      break;
    case kI2cOpClose:
      req_wrap->result = iotjs_i2c_close(i2c);
      break;
    default:
      IOTJS_ASSERT(!"Invalid Operation");
  }
}

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
  if (jerry_value_is_error(res)) {
    return res;
  }

  JS_GET_REQUIRED_CONF_VALUE(jconfig, i2c->address, IOTJS_MAGIC_STRING_ADDRESS,
                             number);

  jerry_value_t jcallback = JS_GET_ARG_IF_EXIST(1, function);

  // If the callback doesn't exist, it is completed synchronously.
  // Otherwise, it will be executed asynchronously.
  if (!jerry_value_is_null(jcallback)) {
    iotjs_periph_call_async(i2c, jcallback, kI2cOpOpen, i2c_worker);
  } else if (!iotjs_i2c_open(i2c)) {
    return JS_CREATE_ERROR(COMMON, iotjs_periph_error_str(kI2cOpOpen));
  }

  return jerry_create_undefined();
}

JS_FUNCTION(Close) {
  JS_DECLARE_THIS_PTR(i2c, i2c);
  DJS_CHECK_ARG_IF_EXIST(1, function);

  iotjs_periph_call_async(i2c, JS_GET_ARG_IF_EXIST(0, function), kI2cOpClose,
                          i2c_worker);

  return jerry_create_undefined();
}

JS_FUNCTION(CloseSync) {
  JS_DECLARE_THIS_PTR(i2c, i2c);

  if (!iotjs_i2c_close(i2c)) {
    return JS_CREATE_ERROR(COMMON, iotjs_periph_error_str(kI2cOpClose));
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
    iotjs_periph_call_async(i2c, JS_GET_ARG_IF_EXIST(1, function), kI2cOpWrite,
                            i2c_worker);
  } else {
    if (!iotjs_i2c_write(i2c)) {
      return JS_CREATE_ERROR(COMMON, iotjs_periph_error_str(kI2cOpWrite));
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

  iotjs_periph_call_async(i2c, JS_GET_ARG_IF_EXIST(1, function), kI2cOpRead,
                          i2c_worker);

  return jerry_create_undefined();
}

JS_FUNCTION(ReadSync) {
  JS_DECLARE_THIS_PTR(i2c, i2c);
  DJS_CHECK_ARGS(1, number);

  JS_GET_REQUIRED_ARG_VALUE(0, i2c->buf_len, IOTJS_MAGIC_STRING_LENGTH, number);

  jerry_value_t result;
  if (iotjs_i2c_read(i2c)) {
    result = iotjs_jval_create_byte_array(i2c->buf_len, i2c->buf_data);
  } else {
    result = JS_CREATE_ERROR(COMMON, iotjs_periph_error_str(kI2cOpRead));
  }

  IOTJS_RELEASE(i2c->buf_data);

  return result;
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
