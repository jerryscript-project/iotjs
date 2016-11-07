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


Gpio::Gpio(const iotjs_jval_t* jgpio)
    : JObjectWrap(jgpio) {
}


Gpio::~Gpio() {
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


#define SET_GPIO(setType) \
  JHANDLER_CHECK_ARGS(4, number, number, number, function); \
  \
  int32_t pin = JHANDLER_GET_ARG(0, number); \
  GpioDirection dir = (GpioDirection)JHANDLER_GET_ARG(1, number); \
  GpioMode mode = (GpioMode)JHANDLER_GET_ARG(2, number); \
  const iotjs_jval_t* arg3 = JHANDLER_GET_ARG(3, function); \
  \
  if (dir < kGpioDirectionNone || \
      dir > kGpioDirectionOut) { \
    JHANDLER_THROW(TYPE, "Invalid GPIO direction"); \
    return; \
  } \
  if (mode < kGpioModeNone || \
      mode > kGpioModeOpendrain) { \
    JHANDLER_THROW(TYPE, "Invalid GPIO mode"); \
    return; \
  } \
  \
  GpioReqWrap* req_wrap = new GpioReqWrap(arg3); \
  GpioReqData* req_data = req_wrap->req(); \
  \
  req_data->pin = pin; \
  req_data->dir = dir; \
  req_data->mode = mode; \
  req_data->op = kGpioOpSet ## setType; \
  \
  Gpio* gpio = Gpio::GetInstance(); \
  gpio->Set ## setType(req_wrap);


#define WRITE_GPIO(writeType, writeTypeEnum) \
  if (writeTypeEnum == GpioSettingType::kGpioPin) { \
    JHANDLER_CHECK_ARGS(3, number, boolean, function); \
  } else { \
    JHANDLER_CHECK_ARGS(3, number, number, function); \
  } \
  \
  const iotjs_jval_t* arg2 = JHANDLER_GET_ARG(2, object); \
  GpioReqWrap* req_wrap = new GpioReqWrap(arg2); \
  GpioReqData* req_data = req_wrap->req(); \
  \
  req_data->pin = JHANDLER_GET_ARG(0, number); \
  if (writeTypeEnum == GpioSettingType::kGpioPin) { \
    req_data->value = JHANDLER_GET_ARG(1, boolean) ? 1 : 0; \
  } else { \
    req_data->value = JHANDLER_GET_ARG(1, number); \
  } \
  req_data->op = kGpioOpWrite ## writeType; \
  \
  Gpio* gpio = Gpio::GetInstance(); \
  gpio->Write ## writeType(req_wrap);


#define READ_GPIO(readType) \
  JHANDLER_CHECK_ARGS(2, number, function); \
  \
  const iotjs_jval_t* arg1 = JHANDLER_GET_ARG(1, function); \
  GpioReqWrap* req_wrap = new GpioReqWrap(arg1); \
  GpioReqData* req_data = req_wrap->req(); \
  \
  req_data->pin = JHANDLER_GET_ARG(0, number); \
  req_data->op = kGpioOpRead ## readType; \
  \
  Gpio* gpio = Gpio::GetInstance(); \
  gpio->Read ## readType(req_wrap);


// initialize(afterInitialize)
JHANDLER_FUNCTION(Initialize) {
  JHANDLER_CHECK_ARGS(1, function);
  const iotjs_jval_t* arg0 = JHANDLER_GET_ARG(0, object);

  GpioReqWrap* req_wrap = new GpioReqWrap(arg0);
  req_wrap->req()->op = kGpioOpInitize;

  Gpio* gpio = Gpio::GetInstance();
  gpio->Initialize(req_wrap);

  iotjs_jhandler_return_null(jhandler);
}


// release(afterInitialize)
JHANDLER_FUNCTION(Release) {
  JHANDLER_CHECK_ARGS(1, function);

  GpioReqWrap* req_wrap = new GpioReqWrap(iotjs_jhandler_get_arg(jhandler, 0));
  req_wrap->req()->op = kGpioOpRelease;

  Gpio* gpio = Gpio::GetInstance();
  gpio->Release(req_wrap);

  iotjs_jhandler_return_null(jhandler);
}


// setPin(pinNumber, direction, mode, afterSetPin)
JHANDLER_FUNCTION(SetPin) {
  SET_GPIO(Pin);

  iotjs_jhandler_return_null(jhandler);
}


// writePin(pinNumber, value, afterWritePin)
JHANDLER_FUNCTION(WritePin) {
  WRITE_GPIO(Pin, GpioSettingType::kGpioPin);

  iotjs_jhandler_return_null(jhandler);
}


// readPin(pinNumber, afterReadPin)
JHANDLER_FUNCTION(ReadPin) {
  READ_GPIO(Pin);

  iotjs_jhandler_return_null(jhandler);
}


// setPort(portNumber, direction, mode, afterSetPort)
JHANDLER_FUNCTION(SetPort) {
  SET_GPIO(Port);

  iotjs_jhandler_return_null(jhandler);
}


// writePort(portNumber, value, afterWritePort)
JHANDLER_FUNCTION(WritePort) {
  WRITE_GPIO(Port, GpioSettingType::kGpioPort);

  iotjs_jhandler_return_null(jhandler);
}


// readPort(portNumber, afterReadPort)
JHANDLER_FUNCTION(ReadPort) {
  READ_GPIO(Port);

  iotjs_jhandler_return_null(jhandler);
}


JHANDLER_FUNCTION(Query) {
  IOTJS_ASSERT(!"Not implemented");

  iotjs_jhandler_return_null(jhandler);
}


iotjs_jval_t InitGpio() {
  iotjs_jval_t jgpio = iotjs_jval_create_object();

  iotjs_jval_set_method(&jgpio, "initialize", Initialize);

  iotjs_jval_set_method(&jgpio, "initialize", Initialize);
  iotjs_jval_set_method(&jgpio, "release", Release);
  iotjs_jval_set_method(&jgpio, "setPin", SetPin);
  iotjs_jval_set_method(&jgpio, "writePin", WritePin);
  iotjs_jval_set_method(&jgpio, "readPin", ReadPin);
  iotjs_jval_set_method(&jgpio, "setPort", SetPort);
  iotjs_jval_set_method(&jgpio, "writePort", WritePort);
  iotjs_jval_set_method(&jgpio, "readPort", ReadPort);
  iotjs_jval_set_method(&jgpio, "query", Query);

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
