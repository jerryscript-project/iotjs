/* Copyright 2015-present Samsung Electronics Co., Ltd. and other contributors
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

#include <string.h>

#include "iotjs_def.h"
#include "iotjs_module_gpio.h"
#include "iotjs_objectwrap.h"


#define THIS iotjs_gpio_reqwrap_t* gpio_reqwrap

iotjs_gpio_reqwrap_t* iotjs_gpio_reqwrap_create(const iotjs_jval_t* jcallback,
                                                GpioOp op) {
  iotjs_gpio_reqwrap_t* gpio_reqwrap = IOTJS_ALLOC(iotjs_gpio_reqwrap_t);
  IOTJS_VALIDATED_STRUCT_CONSTRUCTOR(iotjs_gpio_reqwrap_t, gpio_reqwrap);

  iotjs_reqwrap_initialize(&_this->reqwrap, jcallback, (uv_req_t*)&_this->req);

  _this->req_data.op = op;

  return gpio_reqwrap;
}


static void iotjs_gpio_reqwrap_destroy(THIS) {
  IOTJS_VALIDATED_STRUCT_DESTRUCTOR(iotjs_gpio_reqwrap_t, gpio_reqwrap);
  iotjs_reqwrap_destroy(&_this->reqwrap);
  IOTJS_RELEASE(gpio_reqwrap);
}


void iotjs_gpio_reqwrap_dispatched(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_gpio_reqwrap_t, gpio_reqwrap);
  iotjs_gpio_reqwrap_destroy(gpio_reqwrap);
}


uv_work_t* iotjs_gpio_reqwrap_req(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_gpio_reqwrap_t, gpio_reqwrap);
  return &_this->req;
}


const iotjs_jval_t* iotjs_gpio_reqwrap_jcallback(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_gpio_reqwrap_t, gpio_reqwrap);
  return iotjs_reqwrap_jcallback(&_this->reqwrap);
}


iotjs_gpio_reqwrap_t* iotjs_gpio_reqwrap_from_request(uv_work_t* req) {
  return (iotjs_gpio_reqwrap_t*)(iotjs_reqwrap_from_request((uv_req_t*)req));
}


iotjs_gpio_reqdata_t* iotjs_gpio_reqwrap_data(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_gpio_reqwrap_t, gpio_reqwrap);
  return &_this->req_data;
}

#undef THIS


static void iotjs_gpio_destroy(iotjs_gpio_t* gpio);


iotjs_gpio_t* iotjs_gpio_create(const iotjs_jval_t* jgpio) {
  iotjs_gpio_t* gpio = IOTJS_ALLOC(iotjs_gpio_t);
  IOTJS_VALIDATED_STRUCT_CONSTRUCTOR(iotjs_gpio_t, gpio);
  iotjs_jobjectwrap_initialize(&_this->jobjectwrap, jgpio,
                               (JFreeHandlerType)iotjs_gpio_destroy);
  _this->initialized = false;
  return gpio;
}


static void iotjs_gpio_destroy(iotjs_gpio_t* gpio) {
  IOTJS_VALIDATED_STRUCT_DESTRUCTOR(iotjs_gpio_t, gpio);
  iotjs_jobjectwrap_destroy(&_this->jobjectwrap);
  IOTJS_RELEASE(gpio);
}


const iotjs_jval_t* iotjs_gpio_get_jgpio() {
  return iotjs_module_get(MODULE_GPIO);
}


iotjs_gpio_t* iotjs_gpio_get_instance() {
  const iotjs_jval_t* jgpio = iotjs_gpio_get_jgpio();
  iotjs_jobjectwrap_t* jobjectwrap = iotjs_jobjectwrap_from_jobject(jgpio);
  return (iotjs_gpio_t*)jobjectwrap;
}


bool iotjs_gpio_initialized() {
  iotjs_gpio_t* gpio = iotjs_gpio_get_instance();
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_gpio_t, gpio);
  return _this->initialized;
}


void iotjs_gpio_set_initialized(iotjs_gpio_t* gpio, bool initialized) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_gpio_t, gpio);
  _this->initialized = initialized;
}


void AfterGPIOWork(uv_work_t* work_req, int status) {
  iotjs_gpio_t* gpio = iotjs_gpio_get_instance();

  iotjs_gpio_reqwrap_t* req_wrap = iotjs_gpio_reqwrap_from_request(work_req);
  iotjs_gpio_reqdata_t* req_data = iotjs_gpio_reqwrap_data(req_wrap);

  GpioError result = req_data->result;
  bool value = req_data->value;

  if (status) {
    result = kGpioErrSys;
  }

  iotjs_jargs_t jargs = iotjs_jargs_create(2);
  iotjs_jargs_append_number(&jargs, result);

  switch (req_data->op) {
    case kGpioOpInitize: {
      if (result == kGpioErrOk) {
        iotjs_gpio_set_initialized(gpio, true);
      }
      break;
    }
    case kGpioOpRelease: {
      if (result == kGpioErrOk) {
        iotjs_gpio_set_initialized(gpio, false);
      }
      break;
    }
    case kGpioOpOpen:
    case kGpioOpWrite: {
      break;
    }
    case kGpioOpRead: {
      if (result == kGpioErrOk) {
        iotjs_jargs_append_bool(&jargs, value);
      }
      break;
    }
    default: {
      IOTJS_ASSERT(!"Unreachable");
      break;
    }
  }

  const iotjs_jval_t* jcallback = iotjs_gpio_reqwrap_jcallback(req_wrap);
  const iotjs_jval_t* jgpio = iotjs_gpio_get_jgpio();
  iotjs_make_callback(jcallback, jgpio, &jargs);

  iotjs_jargs_destroy(&jargs);

  iotjs_gpio_reqwrap_dispatched(req_wrap);
}


#define GPIO_ASYNC(op, initialized)                                    \
  do {                                                                 \
    IOTJS_ASSERT(iotjs_gpio_initialized() == initialized);             \
    uv_loop_t* loop = iotjs_environment_loop(iotjs_environment_get()); \
    uv_work_t* req = iotjs_gpio_reqwrap_req(req_wrap);                 \
    uv_queue_work(loop, req, op##GpioWorker, AfterGPIOWork);           \
  } while (0)


// initialize(afterInitialize)
JHANDLER_FUNCTION(Initialize) {
  JHANDLER_CHECK_ARGS(1, function);
  const iotjs_jval_t* jcallback = JHANDLER_GET_ARG(0, function);

  iotjs_gpio_reqwrap_t* req_wrap =
      iotjs_gpio_reqwrap_create(jcallback, kGpioOpInitize);

  GPIO_ASYNC(Initialize, false);

  iotjs_jhandler_return_null(jhandler);
}


// release(afterInitialize)
JHANDLER_FUNCTION(Release) {
  JHANDLER_CHECK_ARGS(1, function);
  const iotjs_jval_t* jcallback = JHANDLER_GET_ARG(0, function);

  iotjs_gpio_reqwrap_t* req_wrap =
      iotjs_gpio_reqwrap_create(jcallback, kGpioOpRelease);

  GPIO_ASYNC(Release, true);

  iotjs_jhandler_return_null(jhandler);
}


// open(pinNumber, direction, mode, afterOpen)
JHANDLER_FUNCTION(Open) {
  JHANDLER_CHECK_ARGS(4, number, number, number, function);

  int32_t pin = JHANDLER_GET_ARG(0, number);
  GpioDirection dir = (GpioDirection)JHANDLER_GET_ARG(1, number);
  GpioMode mode = (GpioMode)JHANDLER_GET_ARG(2, number);
  const iotjs_jval_t* jcallback = JHANDLER_GET_ARG(3, function);

  if (dir < kGpioDirectionNone || dir > kGpioDirectionOut) {
    JHANDLER_THROW(TYPE, "Invalid GPIO direction");
    return;
  }
  if (mode < kGpioModeNone || mode > kGpioModeOpendrain) {
    JHANDLER_THROW(TYPE, "Invalid GPIO mode");
    return;
  }

  iotjs_gpio_reqwrap_t* req_wrap =
      iotjs_gpio_reqwrap_create(jcallback, kGpioOpOpen);

  iotjs_gpio_reqdata_t* req_data = iotjs_gpio_reqwrap_data(req_wrap);
  req_data->pin = pin;
  req_data->dir = dir;
  req_data->mode = mode;

  GPIO_ASYNC(Open, true);

  iotjs_jhandler_return_null(jhandler);
}


// write(pinNumber, value, afterWrite)
JHANDLER_FUNCTION(Write) {
  JHANDLER_CHECK_ARGS(3, number, boolean, function);

  uint32_t pin = JHANDLER_GET_ARG(0, number);
  bool value = JHANDLER_GET_ARG(1, boolean);
  const iotjs_jval_t* jcallback = JHANDLER_GET_ARG(2, function);

  iotjs_gpio_reqwrap_t* req_wrap =
      iotjs_gpio_reqwrap_create(jcallback, kGpioOpWrite);

  iotjs_gpio_reqdata_t* req_data = iotjs_gpio_reqwrap_data(req_wrap);
  req_data->pin = pin;
  req_data->value = value;

  GPIO_ASYNC(Write, true);

  iotjs_jhandler_return_null(jhandler);
}


// read(pinNumber, afterRead)
JHANDLER_FUNCTION(Read) {
  JHANDLER_CHECK_ARGS(2, number, function);

  uint32_t pin = JHANDLER_GET_ARG(0, number);
  const iotjs_jval_t* jcallback = JHANDLER_GET_ARG(1, function);

  iotjs_gpio_reqwrap_t* req_wrap =
      iotjs_gpio_reqwrap_create(jcallback, kGpioOpRead);

  iotjs_gpio_reqdata_t* req_data = iotjs_gpio_reqwrap_data(req_wrap);
  req_data->pin = pin;

  GPIO_ASYNC(Read, true);

  iotjs_jhandler_return_null(jhandler);
}


iotjs_jval_t InitGpio() {
  iotjs_jval_t jgpio = iotjs_jval_create_object();

  iotjs_jval_set_method(&jgpio, "initialize", Initialize);
  iotjs_jval_set_method(&jgpio, "release", Release);
  iotjs_jval_set_method(&jgpio, "open", Open);
  iotjs_jval_set_method(&jgpio, "write", Write);
  iotjs_jval_set_method(&jgpio, "read", Read);

#define SET_CONSTANT(object, constant)                           \
  do {                                                           \
    iotjs_jval_set_property_number(object, #constant, constant); \
  } while (0)

  SET_CONSTANT(&jgpio, kGpioDirectionNone);
  SET_CONSTANT(&jgpio, kGpioDirectionIn);
  SET_CONSTANT(&jgpio, kGpioDirectionOut);

  SET_CONSTANT(&jgpio, kGpioModeNone);
  SET_CONSTANT(&jgpio, kGpioModePullup);
  SET_CONSTANT(&jgpio, kGpioModePulldown);
  SET_CONSTANT(&jgpio, kGpioModeFloat);
  SET_CONSTANT(&jgpio, kGpioModePushpull);
  SET_CONSTANT(&jgpio, kGpioModeOpendrain);

  SET_CONSTANT(&jgpio, kGpioErrOk);
  SET_CONSTANT(&jgpio, kGpioErrInitialize);
  SET_CONSTANT(&jgpio, kGpioErrNotInitialized);
  SET_CONSTANT(&jgpio, kGpioErrWrongUse);
  SET_CONSTANT(&jgpio, kGpioErrSys);

#undef SET_CONSTANT

  iotjs_gpio_t* gpio = iotjs_gpio_create(&jgpio);
  IOTJS_ASSERT(gpio ==
               (iotjs_gpio_t*)(iotjs_jval_get_object_native_handle(&jgpio)));

  return jgpio;
}
