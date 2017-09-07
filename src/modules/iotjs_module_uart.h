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
#include "iotjs_reqwrap.h"


#define UART_WRITE_BUFFER_SIZE 512


typedef enum {
  kUartOpOpen,
  kUartOpClose,
  kUartOpWrite,
} UartOp;


typedef struct {
  iotjs_handlewrap_t handlewrap;
  iotjs_jval_t jemitter_this;
  int device_fd;
  int baud_rate;
  uint8_t data_bits;
  iotjs_string_t device_path;
  iotjs_string_t buf_data;
  unsigned buf_len;
  uv_poll_t poll_handle;
} IOTJS_VALIDATED_STRUCT(iotjs_uart_t);


typedef struct {
  UartOp op;
  bool result;
} iotjs_uart_reqdata_t;


typedef struct {
  iotjs_reqwrap_t reqwrap;
  uv_work_t req;
  iotjs_uart_reqdata_t req_data;
  iotjs_uart_t* uart_instance;
} IOTJS_VALIDATED_STRUCT(iotjs_uart_reqwrap_t);

#define THIS iotjs_uart_reqwrap_t* uart_reqwrap

iotjs_uart_reqwrap_t* iotjs_uart_reqwrap_from_request(uv_work_t* req);
iotjs_uart_reqdata_t* iotjs_uart_reqwrap_data(THIS);

iotjs_uart_t* iotjs_uart_instance_from_reqwrap(THIS);

#undef THIS


#define UART_WORKER_INIT                                                      \
  iotjs_uart_reqwrap_t* req_wrap = iotjs_uart_reqwrap_from_request(work_req); \
  iotjs_uart_reqdata_t* req_data = iotjs_uart_reqwrap_data(req_wrap);         \
  iotjs_uart_t* uart = iotjs_uart_instance_from_reqwrap(req_wrap);


void iotjs_uart_read_cb(uv_poll_t* req, int status, int events);

void iotjs_uart_open_worker(uv_work_t* work_req);
bool iotjs_uart_write(iotjs_uart_t* uart);

#endif /* IOTJS_MODULE_UART_H */
