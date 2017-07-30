/* Copyright 2017-present Samsung Electronics Co., Ltd. and other contributors
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


#ifndef IOTJS_MODULE_UART_TIZEN_GENERAL_INL_H
#define IOTJS_MODULE_UART_TIZEN_GENERAL_INL_H

#include <errno.h>
#include <fcntl.h>
#include <peripheral_io.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include "modules/iotjs_module_uart.h"


struct _iotjs_uart_module_platform_t {
  uv_thread_t thread;
  int pipedes[2];
  peripheral_uart_h native_uart;
};


static int baud_to_constant(int baudRate) {
  switch (baudRate) {
    case 0:
      return PERIPHERAL_UART_BAUDRATE_0;
    case 50:
      return PERIPHERAL_UART_BAUDRATE_50;
    case 75:
      return PERIPHERAL_UART_BAUDRATE_75;
    case 110:
      return PERIPHERAL_UART_BAUDRATE_110;
    case 134:
      return PERIPHERAL_UART_BAUDRATE_134;
    case 150:
      return PERIPHERAL_UART_BAUDRATE_150;
    case 200:
      return PERIPHERAL_UART_BAUDRATE_200;
    case 300:
      return PERIPHERAL_UART_BAUDRATE_300;
    case 600:
      return PERIPHERAL_UART_BAUDRATE_600;
    case 1200:
      return PERIPHERAL_UART_BAUDRATE_1200;
    case 1800:
      return PERIPHERAL_UART_BAUDRATE_1800;
    case 2400:
      return PERIPHERAL_UART_BAUDRATE_2400;
    case 4800:
      return PERIPHERAL_UART_BAUDRATE_4800;
    case 9600:
      return PERIPHERAL_UART_BAUDRATE_9600;
    case 19200:
      return PERIPHERAL_UART_BAUDRATE_19200;
    case 38400:
      return PERIPHERAL_UART_BAUDRATE_38400;
    case 57600:
      return PERIPHERAL_UART_BAUDRATE_57600;
    case 115200:
      return PERIPHERAL_UART_BAUDRATE_115200;
    case 230400:
      return PERIPHERAL_UART_BAUDRATE_230400;
  }

  return -1;
}


static int databits_to_constant(int dataBits) {
  switch (dataBits) {
    case 8:
      return PERIPHERAL_UART_BYTESIZE_8BIT;
    case 7:
      return PERIPHERAL_UART_BYTESIZE_7BIT;
    case 6:
      return PERIPHERAL_UART_BYTESIZE_6BIT;
    case 5:
      return PERIPHERAL_UART_BYTESIZE_5BIT;
  }
  return -1;
}


void uart_thread(void* arg) {
  struct _iotjs_uart_module_platform_t* platform =
      (struct _iotjs_uart_module_platform_t*)arg;

  int output = platform->pipedes[1];

  while (true) {
    uint8_t ch;
    // FIXME: Peripheral_uart_read fails to return number of read characters
    //        and it blocks until more characters is received.
    //        It will hang up until all characters ware obtained.
    //        The one character read was used to prevent this behaviour.
    //        It is very slow.
    // TODO: Request Peripheral UART API redesign.
    int ret = peripheral_uart_read(platform->native_uart, &ch, 1);
    if (ret != PERIPHERAL_ERROR_NONE) {
      return;
    }

    write(output, &ch, 1);
  }
}


void iotjs_uart_platform_open_worker(uv_work_t* work_req) {
  iotjs_worker_t worker;
  iotjs_uart_worker_init(work_req, &worker);
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_uart_t, worker.uart);

  _this->platform = IOTJS_ALLOC(struct _iotjs_uart_module_platform_t);

  int port = -1;
  const char* path = iotjs_string_data(&_this->device_path);
  // /dev/ttySxx is translated to port xx in Peripheral API
  const char* pathPrefix = "/dev/ttyS";

  if (strncmp(path, pathPrefix, strlen(pathPrefix)) == 0) {
    sscanf(path + strlen(pathPrefix), "%d", port);
  }

  if (-1 == port) {
    worker.req_data->result = false;
    IOTJS_RELEASE(_this->platform);
    return;
  }

  if (pipe(_this->platform->pipedes)) {
    worker.req_data->result = false;
    IOTJS_RELEASE(_this->platform);
    return;
  }

  int ret = peripheral_uart_open(port, &(_this->platform->native_uart));
  if (ret != PERIPHERAL_ERROR_NONE) {
    worker.req_data->result = false;
    close(_this->platform->pipedes[0]);
    close(_this->platform->pipedes[1]);
    IOTJS_RELEASE(_this->platform);
    return;
  }

  peripheral_uart_bytesize_e bytesize =
      (peripheral_uart_bytesize_e)databits_to_constant(_this->data_bits);
  // FIXME: IoT.js lacks API parameter for basic UART settings.
  //        so instead hard coded preferences are used.
  //        See: https://github.com/Samsung/iotjs/issues/656
  peripheral_uart_parity_e parity = PERIPHERAL_UART_PARITY_NONE;
  peripheral_uart_stopbits_e stopbits = PERIPHERAL_UART_STOPBITS_1BIT;
  ret = peripheral_uart_set_mode(_this->platform->native_uart, bytesize, parity,
                                 stopbits);

  if (ret != PERIPHERAL_ERROR_NONE) {
    worker.req_data->result = false;
    peripheral_uart_close(_this->platform->native_uart);
    close(_this->platform->pipedes[0]);
    close(_this->platform->pipedes[1]);
    IOTJS_RELEASE(_this->platform);
    return;
  }

  peripheral_uart_baudrate_e baud =
      (peripheral_uart_baudrate_e)baud_to_constant(_this->baud_rate);
  ret = peripheral_uart_set_baudrate(_this->platform->native_uart, baud);

  if (ret != PERIPHERAL_ERROR_NONE) {
    worker.req_data->result = false;
    peripheral_uart_close(_this->platform->native_uart);
    close(_this->platform->pipedes[0]);
    close(_this->platform->pipedes[1]);
    IOTJS_RELEASE(_this->platform);
    return;
  }
  // FIXME: IoT.js lacks API parameter for basic UART settings.
  //        so instead hard coded preferences are used.
  //        See: https://github.com/Samsung/iotjs/issues/656
  ret = peripheral_uart_set_flowcontrol(_this->platform->native_uart, false,
                                        false);

  if (ret != PERIPHERAL_ERROR_NONE) {
    worker.req_data->result = false;
    peripheral_uart_close(_this->platform->native_uart);
    close(_this->platform->pipedes[0]);
    close(_this->platform->pipedes[1]);
    IOTJS_RELEASE(_this->platform);
    return;
  }

  _this->device_fd = _this->platform->pipedes[0];
  uv_poll_t* poll_handle = &_this->poll_handle;
  uv_loop_t* loop = iotjs_environment_loop(iotjs_environment_get());

  uv_poll_init(loop, poll_handle, _this->device_fd);
  poll_handle->data = worker.uart;
  uv_poll_start(poll_handle, UV_READABLE, iotjs_uart_read_cb);
  uv_thread_create(&(_this->platform->thread), uart_thread, _this->platform);
  worker.req_data->result = true;
}


bool iotjs_uart_platform_write(iotjs_uart_t* uart) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_uart_t, uart);

  const char* buf_data = iotjs_string_data(&_this->buf_data);
  uint8_t buffer[_this->buf_len];
  memcpy(buffer, buf_data, _this->buf_len);
  // This function declares second argument as char* so copy is mandatory
  int ret = peripheral_uart_write(_this->platform->native_uart, buffer,
                                  (int)_this->buf_len);

  if (ret != PERIPHERAL_ERROR_NONE) {
    return false;
  }
  return true;
}


bool iotjs_uart_platform_close(iotjs_uart_t* uart) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_uart_t, uart);
  peripheral_uart_close(_this->platform->native_uart);
  IOTJS_RELEASE(_this->platform);
  return iotjs_uart_libtuv_close(uart);
}


void iotjs_uart_platform_close_worker(uv_work_t* work_req) {
  iotjs_uart_libtuv_close_worker(work_req);
}


#endif /* IOTJS_MODULE_UART_TIZEN_GENERAL_INL_H */
