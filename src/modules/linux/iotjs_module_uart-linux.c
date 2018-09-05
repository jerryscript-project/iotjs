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

#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

#include "iotjs_uv_handle.h"
#include "modules/iotjs_module_uart.h"

struct iotjs_uart_platform_data_s {
  iotjs_string_t device_path;
};

static unsigned baud_to_constant(unsigned baudRate) {
  switch (baudRate) {
    case 50:
      return B50;
    case 75:
      return B75;
    case 110:
      return B110;
    case 134:
      return B134;
    case 150:
      return B150;
    case 200:
      return B200;
    case 300:
      return B300;
    case 600:
      return B600;
    case 1200:
      return B1200;
    case 1800:
      return B1800;
    case 2400:
      return B2400;
    case 4800:
      return B4800;
    case 9600:
      return B9600;
    case 19200:
      return B19200;
    case 38400:
      return B38400;
    case 57600:
      return B57600;
    case 115200:
      return B115200;
    case 230400:
      return B230400;
  }
  return B0;
}

static int databits_to_constant(int dataBits) {
  switch (dataBits) {
    case 8:
      return CS8;
    case 7:
      return CS7;
    case 6:
      return CS6;
    case 5:
      return CS5;
  }
  return -1;
}

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

  struct termios options;
  tcgetattr(fd, &options);
  options.c_cflag = CLOCAL | CREAD;
  options.c_cflag |= (tcflag_t)baud_to_constant(uart->baud_rate);
  options.c_cflag |= (tcflag_t)databits_to_constant(uart->data_bits);
  options.c_iflag = IGNPAR;
  options.c_oflag = 0;
  options.c_lflag = 0;
  tcflush(fd, TCIFLUSH);
  tcsetattr(fd, TCSANOW, &options);

  uart->device_fd = fd;
  iotjs_uart_register_read_cb((uv_poll_t*)uart_poll_handle);

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
    tcdrain(fd);

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
