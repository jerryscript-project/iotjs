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
#include "iotjs_module_buffer.h"
#include "iotjs_module_uart.h"


IOTJS_DEFINE_NATIVE_HANDLE_INFO_THIS_MODULE(uart);

static iotjs_uart_t* uart_create(const jerry_value_t juart) {
  iotjs_uart_t* uart = IOTJS_ALLOC(iotjs_uart_t);
  iotjs_uart_create_platform_data(uart);

  iotjs_handlewrap_initialize(&uart->handlewrap, juart,
                              (uv_handle_t*)(&uart->poll_handle),
                              &this_module_native_info);

  uart->device_fd = -1;
  return uart;
}

static void iotjs_uart_destroy(iotjs_uart_t* uart) {
  iotjs_handlewrap_destroy(&uart->handlewrap);
  iotjs_uart_destroy_platform_data(uart->platform_data);
  IOTJS_RELEASE(uart);
}

static void uart_worker(uv_work_t* work_req) {
  iotjs_periph_reqwrap_t* req_wrap =
      (iotjs_periph_reqwrap_t*)(iotjs_reqwrap_from_request(
          (uv_req_t*)work_req));
  iotjs_uart_t* uart = (iotjs_uart_t*)req_wrap->data;

  switch (req_wrap->op) {
    case kUartOpOpen:
      req_wrap->result = iotjs_uart_open(uart);
      break;
    case kUartOpWrite:
      req_wrap->result = iotjs_uart_write(uart);
      break;
    case kUartOpClose:
      iotjs_handlewrap_close(&uart->handlewrap, iotjs_uart_handlewrap_close_cb);
      req_wrap->result = true;
      break;
    default:
      IOTJS_ASSERT(!"Invalid Operation");
  }
}

static void iotjs_uart_read_cb(uv_poll_t* req, int status, int events) {
  iotjs_uart_t* uart = (iotjs_uart_t*)req->data;
  char buf[UART_WRITE_BUFFER_SIZE];
  int i = read(uart->device_fd, buf, UART_WRITE_BUFFER_SIZE - 1);
  if (i > 0) {
    buf[i] = '\0';
    DDDLOG("%s - read length: %d", __func__, i);
    jerry_value_t jemit =
        iotjs_jval_get_property(iotjs_handlewrap_jobject(&uart->handlewrap),
                                IOTJS_MAGIC_STRING_EMIT);
    IOTJS_ASSERT(jerry_value_is_function(jemit));

    jerry_value_t str =
        jerry_create_string((const jerry_char_t*)IOTJS_MAGIC_STRING_DATA);

    jerry_value_t jbuf = iotjs_bufferwrap_create_buffer((size_t)i);
    iotjs_bufferwrap_t* buf_wrap = iotjs_bufferwrap_from_jbuffer(jbuf);
    iotjs_bufferwrap_copy(buf_wrap, buf, (size_t)i);

    jerry_value_t jargs[] = { str, jbuf };
    jerry_value_t jres =
        jerry_call_function(jemit, iotjs_handlewrap_jobject(&uart->handlewrap),
                            jargs, 2);
    IOTJS_ASSERT(!jerry_value_is_error(jres));

    jerry_release_value(jres);
    jerry_release_value(str);
    jerry_release_value(jbuf);
    jerry_release_value(jemit);
  }
}

void iotjs_uart_register_read_cb(iotjs_uart_t* uart) {
  uv_poll_t* poll_handle = &uart->poll_handle;
  uv_loop_t* loop = iotjs_environment_loop(iotjs_environment_get());
  uv_poll_init(loop, poll_handle, uart->device_fd);
  poll_handle->data = uart;
  uv_poll_start(poll_handle, UV_READABLE, iotjs_uart_read_cb);
}

static jerry_value_t uart_set_configuration(iotjs_uart_t* uart,
                                            jerry_value_t jconfig) {
  jerry_value_t jbaud_rate =
      iotjs_jval_get_property(jconfig, IOTJS_MAGIC_STRING_BAUDRATE);
  if (jerry_value_is_undefined(jbaud_rate)) {
    uart->baud_rate = 9600;
  } else {
    if (!jerry_value_is_number(jbaud_rate)) {
      jerry_release_value(jbaud_rate);
      return JS_CREATE_ERROR(TYPE,
                             "Bad configuration - baud rate must be a Number");
    }
    unsigned br = (unsigned)iotjs_jval_as_number(jbaud_rate);
    jerry_release_value(jbaud_rate);

    if (br != 230400 && br != 115200 && br != 57600 && br != 38400 &&
        br != 19200 && br != 9600 && br != 4800 && br != 2400 && br != 1800 &&
        br != 1200 && br != 600 && br != 300 && br != 200 && br != 150 &&
        br != 134 && br != 110 && br != 75 && br != 50 && br != 0) {
      return JS_CREATE_ERROR(TYPE, "Invalid baud rate");
    }

    uart->baud_rate = br;
  }

  jerry_value_t jdata_bits =
      iotjs_jval_get_property(jconfig, IOTJS_MAGIC_STRING_DATABITS);
  if (jerry_value_is_undefined(jdata_bits)) {
    uart->data_bits = 8;
  } else {
    if (!jerry_value_is_number(jdata_bits)) {
      jerry_release_value(jdata_bits);
      return JS_CREATE_ERROR(TYPE,
                             "Bad configuration - data bits must be a Number");
    }
    uint8_t db = (uint8_t)iotjs_jval_as_number(jdata_bits);
    jerry_release_value(jdata_bits);

    if (db > 8 || db < 5) {
      return JS_CREATE_ERROR(TYPE, "Invalid data bits");
    }

    uart->data_bits = db;
  }

  return jerry_create_undefined();
}

JS_FUNCTION(UartCons) {
  DJS_CHECK_THIS();
  DJS_CHECK_ARGS(1, object);
  DJS_CHECK_ARG_IF_EXIST(1, function);

  // Create UART object
  jerry_value_t juart = JS_GET_THIS();
  iotjs_uart_t* uart = uart_create(juart);

  jerry_value_t jconfig = JS_GET_ARG(0, object);

  // set configuration
  jerry_value_t res = iotjs_uart_set_platform_config(uart, jconfig);
  if (jerry_value_is_error(res)) {
    return res;
  }

  res = uart_set_configuration(uart, jconfig);
  if (jerry_value_is_error(res)) {
    return res;
  }

  DDDLOG("%s - baudRate: %d, dataBits: %d", __func__, uart->baud_rate,
         uart->data_bits);

  jerry_value_t jcallback = JS_GET_ARG_IF_EXIST(1, function);

  // If the callback doesn't exist, it is completed synchronously.
  // Otherwise, it will be executed asynchronously.
  if (!jerry_value_is_null(jcallback)) {
    iotjs_periph_call_async(uart, jcallback, kUartOpOpen, uart_worker);
  } else if (!iotjs_uart_open(uart)) {
    return JS_CREATE_ERROR(COMMON, iotjs_periph_error_str(kUartOpOpen));
  }

  return jerry_create_undefined();
}

JS_FUNCTION(Write) {
  JS_DECLARE_THIS_PTR(uart, uart);
  DJS_CHECK_ARGS(1, string);
  DJS_CHECK_ARG_IF_EXIST(1, function);

  uart->buf_data = JS_GET_ARG(0, string);
  uart->buf_len = iotjs_string_size(&uart->buf_data);

  iotjs_periph_call_async(uart, JS_GET_ARG_IF_EXIST(1, function), kUartOpWrite,
                          uart_worker);

  return jerry_create_undefined();
}

JS_FUNCTION(WriteSync) {
  JS_DECLARE_THIS_PTR(uart, uart);
  DJS_CHECK_ARGS(1, string);

  uart->buf_data = JS_GET_ARG(0, string);
  uart->buf_len = iotjs_string_size(&uart->buf_data);

  bool result = iotjs_uart_write(uart);
  iotjs_string_destroy(&uart->buf_data);

  if (!result) {
    return JS_CREATE_ERROR(COMMON, iotjs_periph_error_str(kUartOpWrite));
  }

  return jerry_create_undefined();
}

JS_FUNCTION(Close) {
  JS_DECLARE_THIS_PTR(uart, uart);
  DJS_CHECK_ARG_IF_EXIST(0, function);

  iotjs_periph_call_async(uart, JS_GET_ARG_IF_EXIST(0, function), kUartOpClose,
                          uart_worker);

  return jerry_create_undefined();
}

JS_FUNCTION(CloseSync) {
  JS_DECLARE_THIS_PTR(uart, uart);

  iotjs_handlewrap_close(&uart->handlewrap, iotjs_uart_handlewrap_close_cb);
  return jerry_create_undefined();
}

jerry_value_t InitUart() {
  jerry_value_t juart_cons = jerry_create_external_function(UartCons);

  jerry_value_t prototype = jerry_create_object();
  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_WRITE, Write);
  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_WRITESYNC, WriteSync);
  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_CLOSE, Close);
  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_CLOSESYNC, CloseSync);

  iotjs_jval_set_property_jval(juart_cons, IOTJS_MAGIC_STRING_PROTOTYPE,
                               prototype);

  jerry_release_value(prototype);

  return juart_cons;
}
