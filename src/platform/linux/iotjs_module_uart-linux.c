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


#ifndef IOTJS_MODULE_UART_LINUX_GENERAL_INL_H
#define IOTJS_MODULE_UART_LINUX_GENERAL_INL_H

#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

#include "modules/iotjs_module_uart.h"

static int baud_to_constant(int baudRate) {
  switch (baudRate) {
    case 0:
      return B0;
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
  return -1;
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


void iotjs_uart_open_worker(uv_work_t* work_req) {
  UART_WORKER_INIT;
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_uart_t, uart);

  int fd = open(iotjs_string_data(&_this->device_path),
                O_RDWR | O_NOCTTY | O_NDELAY);
  if (fd < 0) {
    req_data->result = false;
    return;
  }

  struct termios options;
  tcgetattr(fd, &options);
  options.c_cflag = CLOCAL | CREAD;
  options.c_cflag |= (tcflag_t)baud_to_constant(_this->baud_rate);
  options.c_cflag |= (tcflag_t)databits_to_constant(_this->data_bits);
  options.c_iflag = IGNPAR;
  options.c_oflag = 0;
  options.c_lflag = 0;
  tcflush(fd, TCIFLUSH);
  tcsetattr(fd, TCSANOW, &options);

  _this->device_fd = fd;
  uv_poll_t* poll_handle = &_this->poll_handle;

  uv_loop_t* loop = iotjs_environment_loop(iotjs_environment_get());
  uv_poll_init(loop, poll_handle, fd);
  poll_handle->data = uart;
  uv_poll_start(poll_handle, UV_READABLE, iotjs_uart_read_cb);

  req_data->result = true;
}


bool iotjs_uart_write(iotjs_uart_t* uart) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_uart_t, uart);
  int bytesWritten = 0;
  unsigned offset = 0;
  int fd = _this->device_fd;
  const char* buf_data = iotjs_string_data(&_this->buf_data);

  DDDLOG("%s - data: %s", __func__, buf_data);

  do {
    errno = 0;
    bytesWritten = write(fd, buf_data + offset, _this->buf_len - offset);
    tcdrain(fd);

    DDDLOG("%s - size: %d", __func__, _this->buf_len - offset);

    if (bytesWritten != -1) {
      offset += (unsigned)bytesWritten;
      continue;
    }

    if (errno == EINTR) {
      continue;
    }

    return false;

  } while (_this->buf_len > offset);

  return true;
}


#endif /* IOTJS_MODULE_UART_LINUX_GENERAL_INL_H */
