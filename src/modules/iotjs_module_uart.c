/* Copyright 2015-present Samsung Electronics Co., Ltd. and other contributors
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

#include <unistd.h>

#include "iotjs_def.h"
#include "iotjs_module_uart.h"


IOTJS_DEFINE_NATIVE_HANDLE_INFO_THIS_MODULE(uart);

static iotjs_uart_t* iotjs_uart_create(jerry_value_t juart) {
  iotjs_uart_t* uart = IOTJS_ALLOC(iotjs_uart_t);
  IOTJS_VALIDATED_STRUCT_CONSTRUCTOR(iotjs_uart_t, uart);

  iotjs_handlewrap_initialize(&_this->handlewrap, juart,
                              (uv_handle_t*)(&_this->poll_handle),
                              &this_module_native_info);

  _this->device_fd = -1;
  return uart;
}

static void iotjs_uart_destroy(iotjs_uart_t* uart) {
  IOTJS_VALIDATED_STRUCT_DESTRUCTOR(iotjs_uart_t, uart);
  iotjs_handlewrap_destroy(&_this->handlewrap);
  iotjs_string_destroy(&_this->device_path);
  IOTJS_RELEASE(uart);
}


#define THIS iotjs_uart_reqwrap_t* uart_reqwrap


static iotjs_uart_reqwrap_t* iotjs_uart_reqwrap_create(jerry_value_t jcallback,
                                                       iotjs_uart_t* uart,
                                                       UartOp op) {
  iotjs_uart_reqwrap_t* uart_reqwrap = IOTJS_ALLOC(iotjs_uart_reqwrap_t);
  IOTJS_VALIDATED_STRUCT_CONSTRUCTOR(iotjs_uart_reqwrap_t, uart_reqwrap);

  iotjs_reqwrap_initialize(&_this->reqwrap, jcallback, (uv_req_t*)&_this->req);

  _this->req_data.op = op;
  _this->uart_instance = uart;

  return uart_reqwrap;
}


static void iotjs_uart_reqwrap_destroy(THIS) {
  IOTJS_VALIDATED_STRUCT_DESTRUCTOR(iotjs_uart_reqwrap_t, uart_reqwrap);
  iotjs_reqwrap_destroy(&_this->reqwrap);
  IOTJS_RELEASE(uart_reqwrap);
}


static void iotjs_uart_reqwrap_dispatched(THIS) {
  IOTJS_VALIDATABLE_STRUCT_METHOD_VALIDATE(iotjs_uart_reqwrap_t, uart_reqwrap);
  iotjs_uart_reqwrap_destroy(uart_reqwrap);
}


static uv_work_t* iotjs_uart_reqwrap_req(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_uart_reqwrap_t, uart_reqwrap);
  return &_this->req;
}


static jerry_value_t iotjs_uart_reqwrap_jcallback(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_uart_reqwrap_t, uart_reqwrap);
  return iotjs_reqwrap_jcallback(&_this->reqwrap);
}


static iotjs_uart_t* iotjs_uart_instance_from_jval(jerry_value_t juart) {
  iotjs_handlewrap_t* handlewrap = iotjs_handlewrap_from_jobject(juart);
  return (iotjs_uart_t*)handlewrap;
}


iotjs_uart_reqwrap_t* iotjs_uart_reqwrap_from_request(uv_work_t* req) {
  return (iotjs_uart_reqwrap_t*)(iotjs_reqwrap_from_request((uv_req_t*)req));
}


iotjs_uart_reqdata_t* iotjs_uart_reqwrap_data(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_uart_reqwrap_t, uart_reqwrap);
  return &_this->req_data;
}


iotjs_uart_t* iotjs_uart_instance_from_reqwrap(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_uart_reqwrap_t, uart_reqwrap);
  return _this->uart_instance;
}


#undef THIS


static void handlewrap_close_callback(uv_handle_t* handle) {
  iotjs_uart_t* uart = (iotjs_uart_t*)handle->data;
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_uart_t, uart);

  if (close(_this->device_fd) < 0) {
    DLOG("UART Close Error");
    IOTJS_ASSERT(0);
  }
}

static bool iotjs_uart_close(iotjs_uart_t* uart) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_uart_t, uart);

  iotjs_handlewrap_close(&_this->handlewrap, handlewrap_close_callback);

  return true;
}


static void iotjs_uart_write_worker(uv_work_t* work_req) {
  UART_WORKER_INIT;

  if (!iotjs_uart_write(uart)) {
    req_data->result = false;
    return;
  }

  req_data->result = true;
}


static void iotjs_uart_close_worker(uv_work_t* work_req) {
  UART_WORKER_INIT;

  if (!iotjs_uart_close(uart)) {
    req_data->result = false;
    return;
  }

  req_data->result = true;
}


static void iotjs_uart_after_worker(uv_work_t* work_req, int status) {
  iotjs_uart_reqwrap_t* req_wrap = iotjs_uart_reqwrap_from_request(work_req);
  iotjs_uart_reqdata_t* req_data = iotjs_uart_reqwrap_data(req_wrap);

  iotjs_jargs_t jargs = iotjs_jargs_create(1);

  if (status) {
    iotjs_jargs_append_error(&jargs, "System error");
  } else {
    switch (req_data->op) {
      case kUartOpOpen: {
        if (!req_data->result) {
          iotjs_jargs_append_error(&jargs, "Failed to open UART device");
        } else {
          iotjs_jargs_append_null(&jargs);
        }
        break;
      }
      case kUartOpWrite: {
        iotjs_uart_t* uart = iotjs_uart_instance_from_reqwrap(req_wrap);
        IOTJS_VALIDATED_STRUCT_METHOD(iotjs_uart_t, uart);

        iotjs_string_destroy(&_this->buf_data);

        if (!req_data->result) {
          iotjs_jargs_append_error(&jargs, "Cannot write to device");
        } else {
          iotjs_jargs_append_null(&jargs);
        }
        break;
      }
      case kUartOpClose: {
        if (!req_data->result) {
          iotjs_jargs_append_error(&jargs, "Failed to close UART device");
        } else {
          iotjs_jargs_append_null(&jargs);
        }
        break;
      }
      default: {
        IOTJS_ASSERT(!"Unreachable");
        break;
      }
    }
  }

  jerry_value_t jcallback = iotjs_uart_reqwrap_jcallback(req_wrap);
  iotjs_make_callback(jcallback, jerry_create_undefined(), &jargs);

  iotjs_jargs_destroy(&jargs);
  iotjs_uart_reqwrap_dispatched(req_wrap);
}


static void iotjs_uart_onread(jerry_value_t jthis, char* buf) {
  jerry_value_t jemit = iotjs_jval_get_property(jthis, "emit");
  IOTJS_ASSERT(jerry_value_is_function(jemit));

  iotjs_jargs_t jargs = iotjs_jargs_create(2);
  jerry_value_t str = jerry_create_string((const jerry_char_t*)"data");
  jerry_value_t data = jerry_create_string((const jerry_char_t*)buf);
  iotjs_jargs_append_jval(&jargs, str);
  iotjs_jargs_append_jval(&jargs, data);
  iotjs_jhelper_call_ok(jemit, jthis, &jargs);

  jerry_release_value(str);
  jerry_release_value(data);
  iotjs_jargs_destroy(&jargs);
  jerry_release_value(jemit);
}


void iotjs_uart_read_cb(uv_poll_t* req, int status, int events) {
  iotjs_uart_t* uart = (iotjs_uart_t*)req->data;
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_uart_t, uart);

  char buf[UART_WRITE_BUFFER_SIZE];
  int i = read(_this->device_fd, buf, UART_WRITE_BUFFER_SIZE - 1);
  if (i > 0) {
    buf[i] = '\0';
    DDDLOG("%s - read data: %s", __func__, buf);
    iotjs_uart_onread(_this->jemitter_this, buf);
  }
}


#define UART_ASYNC(call, this, jcallback, op)                          \
  do {                                                                 \
    uv_loop_t* loop = iotjs_environment_loop(iotjs_environment_get()); \
    iotjs_uart_reqwrap_t* req_wrap =                                   \
        iotjs_uart_reqwrap_create(jcallback, this, op);                \
    uv_work_t* req = iotjs_uart_reqwrap_req(req_wrap);                 \
    uv_queue_work(loop, req, iotjs_uart_##call##_worker,               \
                  iotjs_uart_after_worker);                            \
  } while (0)


JS_FUNCTION(UartConstructor) {
  DJS_CHECK_THIS();
  DJS_CHECK_ARGS(3, object, object, function);

  // Create UART object
  jerry_value_t juart = JS_GET_THIS();
  iotjs_uart_t* uart = iotjs_uart_create(juart);
  IOTJS_ASSERT(uart == iotjs_uart_instance_from_jval(juart));
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_uart_t, uart);

  jerry_value_t jconfiguration = JS_GET_ARG(0, object);
  jerry_value_t jemitter_this = JS_GET_ARG(1, object);
  _this->jemitter_this = jerry_acquire_value(jemitter_this);
  jerry_value_t jcallback = JS_GET_ARG(2, function);

  // set configuration
  jerry_value_t jdevice =
      iotjs_jval_get_property(jconfiguration, IOTJS_MAGIC_STRING_DEVICE);
  jerry_value_t jbaud_rate =
      iotjs_jval_get_property(jconfiguration, IOTJS_MAGIC_STRING_BAUDRATE);
  jerry_value_t jdata_bits =
      iotjs_jval_get_property(jconfiguration, IOTJS_MAGIC_STRING_DATABITS);

  _this->device_path = iotjs_jval_as_string(jdevice);
  _this->baud_rate = iotjs_jval_as_number(jbaud_rate);
  _this->data_bits = iotjs_jval_as_number(jdata_bits);

  DDDLOG("%s - path: %s, baudRate: %d, dataBits: %d", __func__,
         iotjs_string_data(&_this->device_path), _this->baud_rate,
         _this->data_bits);

  jerry_release_value(jdevice);
  jerry_release_value(jbaud_rate);
  jerry_release_value(jdata_bits);

  UART_ASYNC(open, uart, jcallback, kUartOpOpen);

  return jerry_create_undefined();
}


JS_FUNCTION(Write) {
  JS_DECLARE_THIS_PTR(uart, uart);
  DJS_CHECK_ARGS(1, string);
  DJS_CHECK_ARG_IF_EXIST(1, function);

  const jerry_value_t jcallback = JS_GET_ARG_IF_EXIST(1, function);

  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_uart_t, uart);

  _this->buf_data = JS_GET_ARG(0, string);
  _this->buf_len = iotjs_string_size(&_this->buf_data);

  if (!jerry_value_is_null(jcallback)) {
    UART_ASYNC(write, uart, jcallback, kUartOpWrite);
  } else {
    bool result = iotjs_uart_write(uart);
    iotjs_string_destroy(&_this->buf_data);

    if (!result) {
      return JS_CREATE_ERROR(COMMON, "UART Write Error");
    }
  }

  return jerry_create_null();
}


JS_FUNCTION(Close) {
  JS_DECLARE_THIS_PTR(uart, uart);
  DJS_CHECK_ARG_IF_EXIST(0, function);

  const jerry_value_t jcallback = JS_GET_ARG_IF_EXIST(0, function);

  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_uart_t, uart);
  jerry_release_value(_this->jemitter_this);

  if (!jerry_value_is_null(jcallback)) {
    UART_ASYNC(close, uart, jcallback, kUartOpClose);
  } else {
    if (!iotjs_uart_close(uart)) {
      return JS_CREATE_ERROR(COMMON, "UART Close Error");
    }
  }

  return jerry_create_undefined();
}


jerry_value_t InitUart() {
  jerry_value_t juart_constructor =
      jerry_create_external_function(UartConstructor);

  jerry_value_t prototype = jerry_create_object();

  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_WRITE, Write);
  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_CLOSE, Close);

  iotjs_jval_set_property_jval(juart_constructor, IOTJS_MAGIC_STRING_PROTOTYPE,
                               prototype);

  jerry_release_value(prototype);

  return juart_constructor;
}
