/* Copyright 2015 Samsung Electronics Co., Ltd.
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
#include "iotjs_objectwrap.h"
#include "iotjs_module_gpio.h"

namespace iotjs {


Gpio::Gpio(const iotjs_jval_t* jgpio) {
  iotjs_jobjectwrap_initialize(&_jobjectwrap, jgpio, Delete);
}


Gpio::~Gpio() {
  iotjs_jobjectwrap_destroy(&_jobjectwrap);
}


const iotjs_jval_t* Gpio::GetJGpio() {
  return iotjs_module_get(MODULE_GPIO);
}


Gpio* Gpio::GetInstance() {
  Gpio* gpio = reinterpret_cast<Gpio*>(
          iotjs_jval_get_object_native_handle(Gpio::GetJGpio()));
  IOTJS_ASSERT(gpio != NULL);

  return gpio;
}


// initialize(afterInitialize)
JHANDLER_FUNCTION(Initialize) {
  JHANDLER_CHECK_ARGS(1, function);

  GpioReqWrap* req_wrap = new GpioReqWrap(JHANDLER_GET_ARG(0, function));
  req_wrap->op = kGpioOpInitize;

  Gpio* gpio = Gpio::GetInstance();
  gpio->Initialize(req_wrap);

  iotjs_jhandler_return_null(jhandler);
}


// release(afterInitialize)
JHANDLER_FUNCTION(Release) {
  JHANDLER_CHECK_ARGS(1, function);

  GpioReqWrap* req_wrap = new GpioReqWrap(JHANDLER_GET_ARG(0, function));
  req_wrap->op = kGpioOpRelease;

  Gpio* gpio = Gpio::GetInstance();
  gpio->Release(req_wrap);

  iotjs_jhandler_return_null(jhandler);
}


// open(pinNumber, direction, mode, afterOpen)
JHANDLER_FUNCTION(Open) {
  JHANDLER_CHECK_ARGS(4, number, number, number, function);

  int32_t pin = JHANDLER_GET_ARG(0, number);
  GpioDirection dir = (GpioDirection)JHANDLER_GET_ARG(1, number);
  GpioMode mode = (GpioMode)JHANDLER_GET_ARG(2, number);

  if (dir < kGpioDirectionNone ||
      dir > kGpioDirectionOut) {
    JHANDLER_THROW(TYPE, "Invalid GPIO direction");
    return;
  }
  if (mode < kGpioModeNone ||
      mode > kGpioModeOpendrain) {
    JHANDLER_THROW(TYPE, "Invalid GPIO mode");
    return;
  }

  GpioReqWrap* req_wrap = new GpioReqWrap(JHANDLER_GET_ARG(3, function));

  req_wrap->pin = pin;
  req_wrap->dir = dir;
  req_wrap->mode = mode;
  req_wrap->op = kGpioOpOpen;

  Gpio* gpio = Gpio::GetInstance();
  gpio->Open(req_wrap);

  iotjs_jhandler_return_null(jhandler);
}


// write(pinNumber, value, afterWrite)
JHANDLER_FUNCTION(Write) {
  JHANDLER_CHECK_ARGS(3, number, boolean, function);

  GpioReqWrap* req_wrap = new GpioReqWrap(JHANDLER_GET_ARG(2, function));

  req_wrap->pin = JHANDLER_GET_ARG(0, number);
  req_wrap->value = JHANDLER_GET_ARG(1, boolean) ? 1 : 0;
  req_wrap->op = kGpioOpWrite;

  Gpio* gpio = Gpio::GetInstance();
  gpio->Write(req_wrap);

  iotjs_jhandler_return_null(jhandler);
}


// read(pinNumber, afterRead)
JHANDLER_FUNCTION(Read) {
  JHANDLER_CHECK_ARGS(2, number, function);

  GpioReqWrap* req_wrap = new GpioReqWrap(JHANDLER_GET_ARG(1, function));

  req_wrap->pin = JHANDLER_GET_ARG(0, number);
  req_wrap->op = kGpioOpRead;

  Gpio* gpio = Gpio::GetInstance();
  gpio->Read(req_wrap);

  iotjs_jhandler_return_null(jhandler);
}


iotjs_jval_t InitGpio() {
  iotjs_jval_t jgpio = iotjs_jval_create_object();

  iotjs_jval_set_method(&jgpio, "initialize", Initialize);

  iotjs_jval_set_method(&jgpio, "initialize", Initialize);
  iotjs_jval_set_method(&jgpio, "release", Release);
  iotjs_jval_set_method(&jgpio, "open", Open);
  iotjs_jval_set_method(&jgpio, "write", Write);
  iotjs_jval_set_method(&jgpio, "read", Read);

#define SET_CONSTANT(object, constant) \
  do { \
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

  Gpio* gpio = Gpio::Create(&jgpio);
  IOTJS_ASSERT(gpio == reinterpret_cast<Gpio*>(
              iotjs_jval_get_object_native_handle(&jgpio)));

  return jgpio;
}


} // namespace iotjs


extern "C" {

iotjs_jval_t InitGpio() {
  return iotjs::InitGpio();
}

} // extern "C"
