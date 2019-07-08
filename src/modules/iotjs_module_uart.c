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
#include "iotjs_uv_handle.h"
#include "iotjs_uv_request.h"


static void iotjs_uart_object_destroy(uv_handle_t* handle);

static const jerry_object_native_info_t this_module_native_info = {
  .free_cb = (jerry_object_native_free_callback_t)iotjs_uart_object_destroy,
};


void iotjs_uart_object_destroy(uv_handle_t* handle) {
  iotjs_uart_t* uart = (iotjs_uart_t*)IOTJS_UV_HANDLE_EXTRA_DATA(handle);

  iotjs_uart_destroy_platform_data(uart->platform_data);
  IOTJS_RELEASE(handle);
}


static void uart_worker(uv_work_t* work_req) {
  iotjs_periph_data_t* worker_data =
      (iotjs_periph_data_t*)IOTJS_UV_REQUEST_EXTRA_DATA(work_req);
  uv_handle_t* uart_poll_handle = (uv_handle_t*)worker_data->data;

  switch (worker_data->op) {
    case kUartOpOpen:
      worker_data->result = iotjs_uart_open(uart_poll_handle);
      break;
    case kUartOpWrite:
      worker_data->result = iotjs_uart_write(uart_poll_handle);
      break;
    case kUartOpClose:
      iotjs_uv_handle_close(uart_poll_handle, iotjs_uart_handle_close_cb);
      worker_data->result = true;
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
    jerry_value_t juart = IOTJS_UV_HANDLE_DATA(req)->jobject;
    jerry_value_t jemit =
        iotjs_jval_get_property(juart, IOTJS_MAGIC_STRING_EMIT);
    IOTJS_ASSERT(jerry_value_is_function(jemit));

    jerry_value_t str =
        jerry_create_string((const jerry_char_t*)IOTJS_MAGIC_STRING_DATA);

    jerry_value_t jbuf = iotjs_bufferwrap_create_buffer((size_t)i);
    iotjs_bufferwrap_t* buf_wrap = iotjs_bufferwrap_from_jbuffer(jbuf);
    iotjs_bufferwrap_copy(buf_wrap, buf, (size_t)i);

    jerry_value_t jargs[] = { str, jbuf };
    jerry_value_t jres =
        jerry_call_function(jemit, IOTJS_UV_HANDLE_DATA(req)->jobject, jargs,
                            2);
    IOTJS_ASSERT(!jerry_value_is_error(jres));

    jerry_release_value(jres);
    jerry_release_value(str);
    jerry_release_value(jbuf);
    jerry_release_value(jemit);
  }
}

void iotjs_uart_register_read_cb(uv_poll_t* uart_poll_handle) {
  iotjs_uart_t* uart =
      (iotjs_uart_t*)IOTJS_UV_HANDLE_EXTRA_DATA(uart_poll_handle);
  uv_loop_t* loop = iotjs_environment_loop(iotjs_environment_get());
  uv_poll_init(loop, uart_poll_handle, uart->device_fd);
  uv_poll_start(uart_poll_handle, UV_READABLE, iotjs_uart_read_cb);
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

JS_FUNCTION(uart_constructor) {
  DJS_CHECK_THIS();
  DJS_CHECK_ARGS(1, object);
  DJS_CHECK_ARG_IF_EXIST(1, function);

  // Create UART object
  const jerry_value_t juart = JS_GET_THIS();
  uv_handle_t* uart_poll_handle =
      iotjs_uv_handle_create(sizeof(uv_poll_t), juart, &this_module_native_info,
                             sizeof(iotjs_uart_t));
  iotjs_uart_t* uart =
      (iotjs_uart_t*)IOTJS_UV_HANDLE_EXTRA_DATA(uart_poll_handle);
  // TODO: merge platform data allocation into the handle allocation.
  iotjs_uart_create_platform_data(uart);
  uart->device_fd = -1;

  jerry_value_t jconfig = JS_GET_ARG(0, object);

  // set configuration
  jerry_value_t res = iotjs_uart_set_platform_config(uart, jconfig);
  if (jerry_value_is_error(res)) {
    jerry_release_value(juart);
    return res;
  }

  res = uart_set_configuration(uart, jconfig);
  if (jerry_value_is_error(res)) {
    jerry_release_value(juart);
    return res;
  }

  DDDLOG("%s - baudRate: %d, dataBits: %d", __func__, uart->baud_rate,
         uart->data_bits);

  jerry_value_t jcallback = JS_GET_ARG_IF_EXIST(1, function);

  // If the callback doesn't exist, it is completed synchronously.
  // Otherwise, it will be executed asynchronously.
  if (!jerry_value_is_null(jcallback)) {
    iotjs_periph_call_async(uart_poll_handle, jcallback, kUartOpOpen,
                            uart_worker);
  } else if (!iotjs_uart_open(uart_poll_handle)) {
    return JS_CREATE_ERROR(COMMON, iotjs_periph_error_str(kUartOpOpen));
  }

  return jerry_create_undefined();
}

JS_FUNCTION(uart_write) {
  JS_DECLARE_PTR(jthis, uv_poll_t, uart_poll_handle);
  DJS_CHECK_ARGS(1, string);
  DJS_CHECK_ARG_IF_EXIST(1, function);

  iotjs_uart_t* uart =
      (iotjs_uart_t*)IOTJS_UV_HANDLE_EXTRA_DATA(uart_poll_handle);
  uart->buf_data = JS_GET_ARG(0, string);
  uart->buf_len = iotjs_string_size(&uart->buf_data);

  iotjs_periph_call_async(uart_poll_handle, JS_GET_ARG_IF_EXIST(1, function),
                          kUartOpWrite, uart_worker);

  return jerry_create_undefined();
}

JS_FUNCTION(uart_write_sync) {
  JS_DECLARE_PTR(jthis, uv_handle_t, uart_poll_handle);
  DJS_CHECK_ARGS(1, string);

  iotjs_uart_t* uart =
      (iotjs_uart_t*)IOTJS_UV_HANDLE_EXTRA_DATA(uart_poll_handle);
  uart->buf_data = JS_GET_ARG(0, string);
  uart->buf_len = iotjs_string_size(&uart->buf_data);

  bool result = iotjs_uart_write(uart_poll_handle);
  iotjs_string_destroy(&uart->buf_data);

  if (!result) {
    return JS_CREATE_ERROR(COMMON, iotjs_periph_error_str(kUartOpWrite));
  }

  return jerry_create_undefined();
}

JS_FUNCTION(uart_close) {
  JS_DECLARE_PTR(jthis, uv_poll_t, uart_poll_handle);
  DJS_CHECK_ARG_IF_EXIST(0, function);

  iotjs_periph_call_async(uart_poll_handle, JS_GET_ARG_IF_EXIST(0, function),
                          kUartOpClose, uart_worker);

  return jerry_create_undefined();
}

JS_FUNCTION(uart_close_sync) {
  JS_DECLARE_PTR(jthis, uv_handle_t, uart_poll_handle);

  iotjs_uv_handle_close(uart_poll_handle, iotjs_uart_handle_close_cb);
  return jerry_create_undefined();
}

jerry_value_t iotjs_init_uart(void) {
  jerry_value_t juart_cons = jerry_create_external_function(uart_constructor);

  jerry_value_t prototype = jerry_create_object();
  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_WRITE, uart_write);
  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_WRITESYNC,
                        uart_write_sync);
  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_CLOSE, uart_close);
  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_CLOSESYNC,
                        uart_close_sync);

  iotjs_jval_set_property_jval(juart_cons, IOTJS_MAGIC_STRING_PROTOTYPE,
                               prototype);

  jerry_release_value(prototype);

  return juart_cons;
}
