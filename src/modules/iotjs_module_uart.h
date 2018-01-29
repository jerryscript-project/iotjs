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


#ifndef IOTJS_MODULE_UART_H
#define IOTJS_MODULE_UART_H

#include "iotjs_def.h"
#include "iotjs_handlewrap.h"
#include "iotjs_module_periph_common.h"
#include "iotjs_reqwrap.h"


#define UART_WRITE_BUFFER_SIZE 512

typedef struct {
  iotjs_handlewrap_t handlewrap;
  int device_fd;
  unsigned baud_rate;
  uint8_t data_bits;
  iotjs_string_t device_path;
  iotjs_string_t buf_data;
  unsigned buf_len;
  uv_poll_t poll_handle;
} iotjs_uart_t;

void iotjs_uart_register_read_cb(iotjs_uart_t* uart);
bool iotjs_uart_open(iotjs_uart_t* uart);
bool iotjs_uart_write(iotjs_uart_t* uart);

#endif /* IOTJS_MODULE_UART_H */
