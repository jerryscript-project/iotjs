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

#include "modules/iotjs_module_uart.h"

#include "iotjs_uv_handle.h"

struct iotjs_uart_platform_data_s {
  iotjs_string_t device_path;
};

void iotjs_uart_create_platform_data(iotjs_uart_t* uart) {
  uart->platform_data = IOTJS_ALLOC(iotjs_uart_platform_data_t);
}

void iotjs_uart_destroy_platform_data(
    iotjs_uart_platform_data_t* platform_data) {
  IOTJS_ASSERT(platform_data);

  iotjs_string_destroy(&platform_data->device_path);
  IOTJS_RELEASE(platform_data);
}

jerry_value_t iotjs_uart_set_platform_config(iotjs_uart_t* uart,
                                             const jerry_value_t jconfig) {
  JS_GET_REQUIRED_CONF_VALUE(jconfig, uart->platform_data->device_path,
                             IOTJS_MAGIC_STRING_DEVICE, string);

  return jerry_create_undefined();
}

bool iotjs_uart_open(uv_handle_t* uart_poll_handle) {
  iotjs_uart_t* uart =
      (iotjs_uart_t*)IOTJS_UV_HANDLE_EXTRA_DATA(uart_poll_handle);
  int fd = open(iotjs_string_data(&uart->platform_data->device_path),
                O_RDWR | O_NOCTTY | O_NDELAY);

  if (fd < 0) {
    return false;
  }

  uart->device_fd = fd;
  iotjs_uart_register_read_cb((uv_poll_t*)uart);

  return true;
}

bool iotjs_uart_write(uv_handle_t* uart_poll_handle) {
  iotjs_uart_t* uart =
      (iotjs_uart_t*)IOTJS_UV_HANDLE_EXTRA_DATA(uart_poll_handle);
  int bytesWritten = 0;
  unsigned offset = 0;
  int fd = uart->device_fd;
  const char* buf_data = iotjs_string_data(&uart->buf_data);

  DDDLOG("%s - data: %s", __func__, buf_data);

  do {
    errno = 0;
    bytesWritten = write(fd, buf_data + offset, uart->buf_len - offset);

    DDDLOG("%s - size: %d", __func__, uart->buf_len - offset);

    if (bytesWritten != -1) {
      offset += (unsigned)bytesWritten;
      continue;
    }

    if (errno == EINTR) {
      continue;
    }

    return false;

  } while (uart->buf_len > offset);

  return true;
}

void iotjs_uart_handle_close_cb(uv_handle_t* uart_poll_handle) {
  iotjs_uart_t* uart =
      (iotjs_uart_t*)IOTJS_UV_HANDLE_EXTRA_DATA(uart_poll_handle);

  if (close(uart->device_fd) < 0) {
    DLOG(iotjs_periph_error_str(kUartOpClose));
    IOTJS_ASSERT(0);
  }
}
