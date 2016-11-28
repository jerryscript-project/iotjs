/* Copyright 2015-2016 Samsung Electronics Co., Ltd.
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

#include "iotjs_def.h"
#include "iotjs_module_uart.h"
#include "iotjs_objectwrap.h"


#define THIS iotjs_uart_reqwrap_t* uart_reqwrap


iotjs_uart_reqwrap_t* iotjs_uart_reqwrap_create(const iotjs_jval_t* jcallback,
                                                UartOp op) {
  iotjs_uart_reqwrap_t* uart_reqwrap = IOTJS_ALLOC(iotjs_uart_reqwrap_t);
  IOTJS_VALIDATED_STRUCT_CONSTRUCTOR(iotjs_uart_reqwrap_t, uart_reqwrap);

  iotjs_reqwrap_initialize(&_this->reqwrap, jcallback, (uv_req_t*)&_this->req);

  _this->req_data.path = iotjs_string_create("");
  _this->req_data.op = op;

  return uart_reqwrap;
}


static void iotjs_uart_reqwrap_destroy(THIS) {
  IOTJS_VALIDATED_STRUCT_DESTRUCTOR(iotjs_uart_reqwrap_t, uart_reqwrap);
  iotjs_reqwrap_destroy(&_this->reqwrap);
  iotjs_string_destroy(&_this->req_data.path);
  IOTJS_RELEASE(uart_reqwrap);
}


void iotjs_uart_reqwrap_dispatched(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_uart_reqwrap_t, uart_reqwrap);
  iotjs_uart_reqwrap_destroy(uart_reqwrap);
}


uv_work_t* iotjs_uart_reqwrap_req(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_uart_reqwrap_t, uart_reqwrap);
  return &_this->req;
}


const iotjs_jval_t* iotjs_uart_reqwrap_jcallback(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_uart_reqwrap_t, uart_reqwrap);
  return iotjs_reqwrap_jcallback(&_this->reqwrap);
}


iotjs_uart_reqwrap_t* iotjs_uart_reqwrap_from_request(uv_work_t* req) {
  return (iotjs_uart_reqwrap_t*)(iotjs_reqwrap_from_request((uv_req_t*)req));
}


iotjs_uart_reqdata_t* iotjs_uart_reqwrap_data(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_uart_reqwrap_t, uart_reqwrap);
  return &_this->req_data;
}


#undef THIS


static void iotjs_uart_destroy(iotjs_uart_t* uart);


iotjs_uart_t* iotjs_uart_create(const iotjs_jval_t* juart) {
  iotjs_uart_t* uart = IOTJS_ALLOC(iotjs_uart_t);
  IOTJS_VALIDATED_STRUCT_CONSTRUCTOR(iotjs_uart_t, uart);
  iotjs_jobjectwrap_initialize(&_this->jobjectwrap, juart,
                               (JFreeHandlerType)iotjs_uart_destroy);
  return uart;
}


static void iotjs_uart_destroy(iotjs_uart_t* uart) {
  IOTJS_VALIDATED_STRUCT_DESTRUCTOR(iotjs_uart_t, uart);
  iotjs_jobjectwrap_destroy(&_this->jobjectwrap);
  IOTJS_RELEASE(uart);
}


const iotjs_jval_t* iotjs_uart_get_juart() {
  return iotjs_module_get(MODULE_UART);
}


iotjs_uart_t* iotjs_uart_get_instance() {
  const iotjs_jval_t* juart = iotjs_uart_get_juart();
  iotjs_jobjectwrap_t* jobjectwrap = iotjs_jobjectwrap_from_jobject(juart);
  return (iotjs_uart_t*)jobjectwrap;
}

void iotjs_uart_onread(iotjs_jval_t* jthis, char* buf) {
  iotjs_jval_t jemit = iotjs_jval_get_property(jthis, "emit");
  IOTJS_ASSERT(iotjs_jval_is_function(&jemit));

  iotjs_jargs_t jargs = iotjs_jargs_create(2);
  iotjs_jval_t str = iotjs_jval_create_string_raw("data");
  iotjs_jval_t data = iotjs_jval_create_string_raw(buf);
  iotjs_jargs_append_jval(&jargs, &str);
  iotjs_jargs_append_jval(&jargs, &data);
  iotjs_jhelper_call_ok(&jemit, jthis, &jargs);

  iotjs_jval_destroy(&str);
  iotjs_jval_destroy(&data);
  iotjs_jargs_destroy(&jargs);
}


void AfterUARTWork(uv_work_t* work_req, int status) {
  iotjs_uart_t* uart = iotjs_uart_get_instance();

  iotjs_uart_reqwrap_t* req_wrap = iotjs_uart_reqwrap_from_request(work_req);
  iotjs_uart_reqdata_t* req_data = iotjs_uart_reqwrap_data(req_wrap);

  iotjs_jargs_t jargs = iotjs_jargs_create(2);

  // TODO
  if (status) {
    iotjs_jval_t error = iotjs_jval_create_error("System error");
    iotjs_jargs_append_jval(&jargs, &error);
    iotjs_jval_destroy(&error);
  } else {
    switch (req_data->op) {
      case kUartOpOpen: {
        if (req_data->error == kUartErrOpen) {
          iotjs_jval_t error =
              iotjs_jval_create_error("Failed to open UART device");
          iotjs_jargs_append_jval(&jargs, &error);
          iotjs_jval_destroy(&error);
        } else {
          iotjs_jargs_append_null(&jargs);
        }
        break;
      }
      case kUartOpWrite: {
        if (req_data->error == kUartErrWrite) {
          iotjs_jval_t error =
              iotjs_jval_create_error("Cannot write to device");
          iotjs_jargs_append_jval(&jargs, &error);
          iotjs_jval_destroy(&error);
        } else {
          iotjs_jargs_append_null(&jargs);
        }
        break;
      }
      default: {
        IOTJS_ASSERT(!"Unreachable");
        break;
      }
    }
  }

  const iotjs_jval_t* jcallback = iotjs_uart_reqwrap_jcallback(req_wrap);
  const iotjs_jval_t* juart = iotjs_uart_get_juart();
  iotjs_make_callback(jcallback, juart, &jargs);

  iotjs_jargs_destroy(&jargs);
  iotjs_uart_reqwrap_dispatched(req_wrap);
}


#define UART_ASYNC(op)                                                 \
  do {                                                                 \
    uv_loop_t* loop = iotjs_environment_loop(iotjs_environment_get()); \
    uv_work_t* req = iotjs_uart_reqwrap_req(req_wrap);                 \
    uv_queue_work(loop, req, op##WorkerUart, AfterUARTWork);           \
  } while (0)


// open(value, afterOpen)
JHANDLER_FUNCTION(Open) {
  JHANDLER_CHECK_ARGS(4, object, string, object, function);

  const iotjs_jval_t* jthis = JHANDLER_GET_ARG(0, object);
  const iotjs_string_t path = JHANDLER_GET_ARG(1, string);
  const iotjs_jval_t* options = JHANDLER_GET_ARG(2, object);
  const iotjs_jval_t* jcallback = JHANDLER_GET_ARG(3, function);

  iotjs_uart_reqwrap_t* req_wrap =
      iotjs_uart_reqwrap_create(jcallback, kUartOpOpen);
  iotjs_uart_reqdata_t* req_data = iotjs_uart_reqwrap_data(req_wrap);

  req_data->jthis = iotjs_jval_create_copied(jthis);
  req_data->path = path;
  iotjs_jval_t jbaudRate = iotjs_jval_get_property(options, "baudRate");
  iotjs_jval_t jdataBits = iotjs_jval_get_property(options, "dataBits");
  req_data->baudRate = iotjs_jval_as_number(&jbaudRate);
  req_data->dataBits = iotjs_jval_as_number(&jdataBits);

  UART_ASYNC(Open);

  iotjs_jval_destroy(&jbaudRate);
  iotjs_jval_destroy(&jdataBits);
  iotjs_jhandler_return_null(jhandler);
}


// write(value, afterWrite)
JHANDLER_FUNCTION(Write) {
  JHANDLER_CHECK_ARGS(2, string, function);

  const iotjs_string_t value = JHANDLER_GET_ARG(0, string);
  const iotjs_jval_t* jcallback = JHANDLER_GET_ARG(1, function);

  iotjs_uart_reqwrap_t* req_wrap =
      iotjs_uart_reqwrap_create(jcallback, kUartOpWrite);
  iotjs_uart_reqdata_t* req_data = iotjs_uart_reqwrap_data(req_wrap);
  req_data->buf_data = (char*)iotjs_string_data(&value);
  req_data->buf_len = iotjs_string_size(&value);

  UART_ASYNC(Write);

  iotjs_jhandler_return_null(jhandler);
}


iotjs_jval_t InitUart() {
  iotjs_jval_t juart = iotjs_jval_create_object();

  iotjs_jval_set_method(&juart, "open", Open);
  iotjs_jval_set_method(&juart, "write", Write);

  iotjs_uart_t* uart = iotjs_uart_create(&juart);
  IOTJS_ASSERT(uart ==
               (iotjs_uart_t*)(iotjs_jval_get_object_native_handle(&juart)));

  return juart;
}
