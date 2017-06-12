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

#if defined(__TIZENRT__)


#include "modules/iotjs_module_uart.h"

void iotjs_uart_open_worker(uv_work_t* work_req) {
  UART_WORKER_INIT;
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_uart_t, uart);

  int fd = open(iotjs_string_data(&_this->device_path),
                O_RDWR | O_NOCTTY | O_NDELAY);

  if (fd < 0) {
    req_data->result = false;
    return;
  }

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


#endif // __TIZENRT__
