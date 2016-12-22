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

#if defined(__NUTTX__)


#include "../iotjs_module_uart-posix-general.inl.h"


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


#endif // __NUTTX__
