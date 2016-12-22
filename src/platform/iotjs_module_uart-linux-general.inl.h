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

#include <termios.h>

#include "iotjs_module_uart-posix-general.inl.h"


int toBaudConstant(int baudRate) {
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


int toDataBitsConstant(int dataBits) {
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


void OpenWorkerUart(uv_work_t* work_req) {
  UART_WORKER_INIT_TEMPLATE;

  int fd =
      open(iotjs_string_data(&req_data->path), O_RDWR | O_NOCTTY | O_NDELAY);

  if (fd == -1) {
    req_data->error = kUartErrOpen;
    return;
  } else {
    req_data->error = kUartErrOk;
  }

  struct termios options;
  tcgetattr(fd, &options);
  options.c_cflag = CLOCAL | CREAD;
  options.c_cflag |= toBaudConstant(req_data->baudRate);
  options.c_cflag |= toDataBitsConstant(req_data->dataBits);
  options.c_iflag = IGNPAR;
  options.c_oflag = 0;
  options.c_lflag = 0;
  tcflush(fd, TCIFLUSH);
  tcsetattr(fd, TCSANOW, &options);

  iotjs_uart_set_device_fd(req_data->uart_instance, fd);
  uv_poll_t* poll_handle = iotjs_uart_get_poll_handle(req_data->uart_instance);

  uv_loop_t* loop = iotjs_environment_loop(iotjs_environment_get());
  uv_poll_init(loop, poll_handle, fd);
  poll_handle->data = req_data->uart_instance;
  uv_poll_start(poll_handle, UV_READABLE, read_cb);
}


void WriteWorkerUart(uv_work_t* work_req) {
  UART_WORKER_INIT_TEMPLATE;
  int bytesWritten = 0;
  unsigned offset = 0;
  int fd = iotjs_uart_get_device_fd(req_data->uart_instance);

  do {
    errno = 0;
    bytesWritten =
        write(fd, req_data->buf_data + offset, req_data->buf_len - offset);
    tcdrain(fd);

    if (bytesWritten != -1) {
      offset += bytesWritten;
      continue;
    }

    if (errno == EINTR) {
      continue;
    }

    req_data->error = kUartErrWrite;
    return;

  } while (req_data->buf_len > offset);
}


#endif /* IOTJS_MODULE_UART_LINUX_GENERAL_INL_H */
