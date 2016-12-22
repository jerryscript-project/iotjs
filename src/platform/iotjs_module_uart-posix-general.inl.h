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


#ifndef IOTJS_MODULE_UART_POSIX_GENERAL_INL_H
#define IOTJS_MODULE_UART_POSIX_GENERAL_INL_H

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include "module/iotjs_module_uart.h"


#define UART_WORKER_INIT_TEMPLATE                                             \
  iotjs_uart_reqwrap_t* req_wrap = iotjs_uart_reqwrap_from_request(work_req); \
  iotjs_uart_reqdata_t* req_data = iotjs_uart_reqwrap_data(req_wrap);


void read_cb(uv_poll_t* req, int status, int events) {
  iotjs_uart_t* uart = (iotjs_uart_t*)req->data;
  int fd = iotjs_uart_get_device_fd(uart);

  char buf[512];
  int i = read(fd, buf, 511);
  if (i > 0) {
    buf[i] = '\0';
    iotjs_uart_onread(iotjs_uart_get_jthis(uart), buf);
  }
}


void UartClose(iotjs_uart_t* uart) {
  uv_poll_t* poll_handle = iotjs_uart_get_poll_handle(uart);
  int fd = iotjs_uart_get_device_fd(uart);

  if (fd > 0) {
    if (!uv_is_closing((uv_handle_t*)poll_handle)) {
      uv_close((uv_handle_t*)poll_handle, NULL);
    }
    close(fd);
  }
}


#endif /* IOTJS_MODULE_UART_POSIX_GENERAL_INL_H */
