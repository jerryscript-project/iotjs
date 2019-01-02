/* Copyright 2018-present Samsung Electronics Co., Ltd. and other contributors
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

#include <peripheral_io.h>

#include "iotjs_uv_handle.h"
#include "modules/iotjs_module_uart.h"

struct _peripheral_uart_s {
  unsigned handle;
  int fd;
};

struct iotjs_uart_platform_data_s {
  peripheral_uart_h uart_h;
  uint8_t port;
};

static peripheral_uart_baud_rate_e baud_to_constant(unsigned baudRate) {
  switch (baudRate) {
    case 0:
      return PERIPHERAL_UART_BAUD_RATE_0;
    case 50:
      return PERIPHERAL_UART_BAUD_RATE_50;
    case 75:
      return PERIPHERAL_UART_BAUD_RATE_75;
    case 110:
      return PERIPHERAL_UART_BAUD_RATE_110;
    case 134:
      return PERIPHERAL_UART_BAUD_RATE_134;
    case 150:
      return PERIPHERAL_UART_BAUD_RATE_150;
    case 200:
      return PERIPHERAL_UART_BAUD_RATE_200;
    case 300:
      return PERIPHERAL_UART_BAUD_RATE_300;
    case 600:
      return PERIPHERAL_UART_BAUD_RATE_600;
    case 1200:
      return PERIPHERAL_UART_BAUD_RATE_1200;
    case 1800:
      return PERIPHERAL_UART_BAUD_RATE_1800;
    case 2400:
      return PERIPHERAL_UART_BAUD_RATE_2400;
    case 4800:
      return PERIPHERAL_UART_BAUD_RATE_4800;
    case 9600:
      return PERIPHERAL_UART_BAUD_RATE_9600;
    case 19200:
      return PERIPHERAL_UART_BAUD_RATE_19200;
    case 38400:
      return PERIPHERAL_UART_BAUD_RATE_38400;
    case 57600:
      return PERIPHERAL_UART_BAUD_RATE_57600;
    case 115200:
      return PERIPHERAL_UART_BAUD_RATE_115200;
    case 230400:
      return PERIPHERAL_UART_BAUD_RATE_230400;
  }

  IOTJS_ASSERT(!"Invalid baud rate");
  return -1;
}

static peripheral_uart_byte_size_e databits_to_constant(uint8_t dataBits) {
  switch (dataBits) {
    case 8:
      return PERIPHERAL_UART_BYTE_SIZE_8BIT;
    case 7:
      return PERIPHERAL_UART_BYTE_SIZE_7BIT;
    case 6:
      return PERIPHERAL_UART_BYTE_SIZE_6BIT;
    case 5:
      return PERIPHERAL_UART_BYTE_SIZE_5BIT;
  }

  IOTJS_ASSERT(!"Invalid data bits");
  return -1;
}

void iotjs_uart_create_platform_data(iotjs_uart_t* uart) {
  uart->platform_data = IOTJS_ALLOC(iotjs_uart_platform_data_t);
  uart->platform_data->uart_h = NULL;
}

void iotjs_uart_destroy_platform_data(
    iotjs_uart_platform_data_t* platform_data) {
  IOTJS_ASSERT(platform_data);
  IOTJS_RELEASE(platform_data);
}

jerry_value_t iotjs_uart_set_platform_config(iotjs_uart_t* uart,
                                             const jerry_value_t jconfig) {
  JS_GET_REQUIRED_CONF_VALUE(jconfig, uart->platform_data->port,
                             IOTJS_MAGIC_STRING_PORT, number);

  return jerry_create_undefined();
}

bool iotjs_uart_open(uv_handle_t* uart_poll_handle) {
  iotjs_uart_t* uart =
      (iotjs_uart_t*)IOTJS_UV_HANDLE_EXTRA_DATA(uart_poll_handle);
  iotjs_uart_platform_data_t* platform_data = uart->platform_data;
  IOTJS_ASSERT(platform_data);

  int ret = peripheral_uart_open(platform_data->port, &platform_data->uart_h);
  if (ret != PERIPHERAL_ERROR_NONE) {
    DLOG("%s: cannot open(%d)", __func__, ret);
    return false;
  }

  // Set baud rate
  ret = peripheral_uart_set_baud_rate(platform_data->uart_h,
                                      baud_to_constant(uart->baud_rate));
  if (ret != PERIPHERAL_ERROR_NONE) {
    DLOG("%s: cannot set baud rate(%d)", __func__, ret);
    peripheral_uart_close(platform_data->uart_h);
    return false;
  }

  // Set data bits
  ret = peripheral_uart_set_byte_size(platform_data->uart_h,
                                      databits_to_constant(uart->data_bits));
  if (ret != PERIPHERAL_ERROR_NONE) {
    DLOG("%s: cannot set data bits(%d)", __func__, ret);
    peripheral_uart_close(platform_data->uart_h);
    return false;
  }

  uart->device_fd = platform_data->uart_h->fd;
  iotjs_uart_register_read_cb((uv_poll_t*)uart_poll_handle);

  return true;
}

bool iotjs_uart_write(uv_handle_t* uart_poll_handle) {
  iotjs_uart_t* uart =
      (iotjs_uart_t*)IOTJS_UV_HANDLE_EXTRA_DATA(uart_poll_handle);
  iotjs_uart_platform_data_t* platform_data = uart->platform_data;
  IOTJS_ASSERT(platform_data);
  if (!platform_data->uart_h) {
    DLOG("%s: UART is not opened", __func__);
    return false;
  }

  const char* buf_data = iotjs_string_data(&uart->buf_data);
  DDDLOG("%s: data: %s", __func__, buf_data);

  int ret = peripheral_uart_write(platform_data->uart_h, (uint8_t*)buf_data,
                                  uart->buf_len);
  if (ret != PERIPHERAL_ERROR_NONE) {
    DLOG("%s: cannot write(%d)", __func__, ret);
    return false;
  }

  return true;
}

void iotjs_uart_handle_close_cb(uv_handle_t* uart_poll_handle) {
  iotjs_uart_t* uart =
      (iotjs_uart_t*)IOTJS_UV_HANDLE_EXTRA_DATA(uart_poll_handle);

  if (peripheral_uart_close(uart->platform_data->uart_h) !=
      PERIPHERAL_ERROR_NONE) {
    DLOG("%s: error(%s) ", __func__, iotjs_periph_error_str(kUartOpClose));
    IOTJS_ASSERT(0);
  }

  uart->platform_data->uart_h = NULL;
}
