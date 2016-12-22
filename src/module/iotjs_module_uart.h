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
#include "iotjs_objectwrap.h"
#include "iotjs_reqwrap.h"

typedef enum {
  kUartOpOpen,
  kUartOpClose,
  kUartOpWrite,
} UartOp;

typedef enum {
  kUartErrOk = 0,
  kUartErrOpen = -1,
  kUartErrWrite = -2,
} UartError;

typedef struct {
  iotjs_jobjectwrap_t jobjectwrap;
  iotjs_jval_t jthis;
  int device_fd;
  uv_poll_t poll_handle;
} IOTJS_VALIDATED_STRUCT(iotjs_uart_t);

typedef struct {
  iotjs_string_t path;
  int baudRate;
  uint8_t dataBits;
  char* buf_data;
  unsigned buf_len;
  iotjs_uart_t* uart_instance;

  UartOp op;
  UartError error;
} iotjs_uart_reqdata_t;

typedef struct {
  iotjs_reqwrap_t reqwrap;
  uv_work_t req;
  iotjs_uart_reqdata_t req_data;
} IOTJS_VALIDATED_STRUCT(iotjs_uart_reqwrap_t);

#define THIS iotjs_uart_reqwrap_t* uart_reqwrap
iotjs_uart_reqwrap_t* iotjs_uart_reqwrap_create(const iotjs_jval_t* jcallback,
                                                UartOp op);
void iotjs_uart_reqwrap_dispatched(THIS);
uv_work_t* iotjs_uart_reqwrap_req(THIS);
const iotjs_jval_t* iotjs_uart_reqwrap_jcallback(THIS);
iotjs_uart_reqwrap_t* iotjs_uart_reqwrap_from_request(uv_work_t* req);
iotjs_uart_reqdata_t* iotjs_uart_reqwrap_data(THIS);
#undef THIS

void iotjs_uart_set_device_fd(iotjs_uart_t* uart, int fd);
int iotjs_uart_get_device_fd(iotjs_uart_t* uart);
iotjs_jval_t* iotjs_uart_get_jthis(iotjs_uart_t* uart);
uv_poll_t* iotjs_uart_get_poll_handle(iotjs_uart_t* uart);

iotjs_uart_t* iotjs_uart_create(const iotjs_jval_t* juart);
const iotjs_jval_t* iotjs_uart_get_juart();
iotjs_uart_t* iotjs_uart_get_instance(const iotjs_jval_t* juart);

void iotjs_uart_onread(iotjs_jval_t* jthis, char* buf);

void OpenWorkerUart(uv_work_t* work_req);
void UartClose(iotjs_uart_t* uart);
void WriteWorkerUart(uv_work_t* work_req);

#endif /* IOTJS_MODULE_UART_H */
