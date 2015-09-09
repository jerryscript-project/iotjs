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


Gpio::Gpio(JObject& jgpio)
    : JObjectWrap(jgpio) {
}


Gpio::~Gpio() {
}


JObject* Gpio::GetJGpio() {
  Module* module = GetBuiltinModule(MODULE_GPIO);
  JObject* jgpio = module->module;
  IOTJS_ASSERT(jgpio != NULL);

  return jgpio;
}


Gpio* Gpio::GetInstance() {
  Gpio* gpio = reinterpret_cast<Gpio*>(Gpio::GetJGpio()->GetNative());
  IOTJS_ASSERT(gpio != NULL);

  return gpio;
}


// initialize(afterInitalize)
JHANDLER_FUNCTION(Initialize) {
  IOTJS_ASSERT(handler.GetArgLength() == 1);
  IOTJS_ASSERT(handler.GetArg(0)->IsFunction());

  GpioReqWrap* req_wrap = new GpioReqWrap(*handler.GetArg(0));
  req_wrap->req()->op = kGpioOpInitize;

  Gpio* gpio = Gpio::GetInstance();
  gpio->Initialize(req_wrap);

  handler.Return(JObject::Null());
  return true;
}


// release(afterInitalize)
JHANDLER_FUNCTION(Release) {
  IOTJS_ASSERT(handler.GetArgLength() == 1);
  IOTJS_ASSERT(handler.GetArg(0)->IsFunction());

  GpioReqWrap* req_wrap = new GpioReqWrap(*handler.GetArg(0));
  req_wrap->req()->op = kGpioOpRelease;

  Gpio* gpio = Gpio::GetInstance();
  gpio->Release(req_wrap);

  handler.Return(JObject::Null());
  return true;
}


// setPin(pinNumber, direction, mode, afterSetPin)
JHANDLER_FUNCTION(SetPin) {
  IOTJS_ASSERT(handler.GetArgLength() == 4);
  IOTJS_ASSERT(handler.GetArg(0)->IsNumber());
  IOTJS_ASSERT(handler.GetArg(1)->IsNumber());
  IOTJS_ASSERT(handler.GetArg(2)->IsNumber());
  IOTJS_ASSERT(handler.GetArg(3)->IsFunction());

  GpioReqWrap* req_wrap = new GpioReqWrap(*handler.GetArg(3));
  GpioReqData* req_data = req_wrap->req();

  req_data->pin = handler.GetArg(0)->GetInt32();
  req_data->dir = (GpioDirection)handler.GetArg(1)->GetInt32();
  req_data->mode = (GpioMode)handler.GetArg(2)->GetInt32();
  req_data->op = kGpioOpSetPin;

  IOTJS_ASSERT(req_data->dir >= kGpioDirectionNone &&
               req_data->dir <= kGpioDirectionOut);

  IOTJS_ASSERT(req_data->mode >= kGpioModeNone &&
               req_data->mode <= kGpioModeOpendrain);

  Gpio* gpio = Gpio::GetInstance();
  gpio->SetPin(req_wrap);

  handler.Return(JObject::Null());
  return true;
}


// writePin(pinNumber, value, afterWritePin)
JHANDLER_FUNCTION(WritePin) {
  IOTJS_ASSERT(handler.GetArgLength() == 3);
  IOTJS_ASSERT(handler.GetArg(0)->IsNumber());
  IOTJS_ASSERT(handler.GetArg(1)->IsBoolean());
  IOTJS_ASSERT(handler.GetArg(2)->IsFunction());

  GpioReqWrap* req_wrap = new GpioReqWrap(*handler.GetArg(2));
  GpioReqData* req_data = req_wrap->req();

  req_data->pin = handler.GetArg(0)->GetInt32();
  req_data->value = (handler.GetArg(1)->GetBoolean()) ? 1 : 0;
  req_data->op = kGpioOpWritePin;

  Gpio* gpio = Gpio::GetInstance();
  gpio->WritePin(req_wrap);

  handler.Return(JObject::Null());
  return true;
}


// readPin(pinNumber, afterReadPin)
JHANDLER_FUNCTION(ReadPin) {
  IOTJS_ASSERT(handler.GetArgLength() == 2);
  IOTJS_ASSERT(handler.GetArg(0)->IsNumber());
  IOTJS_ASSERT(handler.GetArg(1)->IsFunction());

  GpioReqWrap* req_wrap = new GpioReqWrap(*handler.GetArg(1));
  GpioReqData* req_data = req_wrap->req();

  req_data->pin = handler.GetArg(0)->GetInt32();
  req_data->op = kGpioOpReadPin;

  Gpio* gpio = Gpio::GetInstance();
  gpio->ReadPin(req_wrap);

  handler.Return(JObject::Null());
  return true;
}


JHANDLER_FUNCTION(SetPort) {
  IOTJS_ASSERT(!"Not implemented");

  handler.Return(JObject::Null());
  return true;
}


JHANDLER_FUNCTION(WritePort) {
  IOTJS_ASSERT(!"Not implemented");

  handler.Return(JObject::Null());
  return true;
}


JHANDLER_FUNCTION(ReadPort) {
  IOTJS_ASSERT(!"Not implemented");

  handler.Return(JObject::Null());
  return true;
}


JHANDLER_FUNCTION(Query) {
  IOTJS_ASSERT(!"Not implemented");

  handler.Return(JObject::Null());
  return true;
}


JObject* InitGpio() {
  Module* module = GetBuiltinModule(MODULE_GPIO);
  JObject* jgpio = module->module;

  if (jgpio == NULL) {
    jgpio = new JObject();

    jgpio->SetMethod("initialize", Initialize);
    jgpio->SetMethod("release", Release);
    jgpio->SetMethod("setPin", SetPin);
    jgpio->SetMethod("writePin", WritePin);
    jgpio->SetMethod("readPin", ReadPin);
    jgpio->SetMethod("setPort", SetPort);
    jgpio->SetMethod("writePort", WritePort);
    jgpio->SetMethod("readPort", ReadPort);
    jgpio->SetMethod("query", Query);

#define SET_CONSTANT(object, name, constant) \
  do { \
    JObject value(constant); \
    object->SetProperty(name, value); \
  } while (0)

    SET_CONSTANT(jgpio, "kGpioDirectionNone", kGpioDirectionNone);
    SET_CONSTANT(jgpio, "kGpioDirectionIn", kGpioDirectionIn);
    SET_CONSTANT(jgpio, "kGpioDirectionOut", kGpioDirectionOut);

    SET_CONSTANT(jgpio, "kGpioModeNone", kGpioModeNone);
    SET_CONSTANT(jgpio, "kGpioModePullup", kGpioModePullup);
    SET_CONSTANT(jgpio, "kGpioModePulldown", kGpioModePulldown);
    SET_CONSTANT(jgpio, "kGpioModeFloat", kGpioModeFloat);
    SET_CONSTANT(jgpio, "kGpioModePushpull", kGpioModePushpull);
    SET_CONSTANT(jgpio, "kGpioModeOpendrain", kGpioModeOpendrain);

    SET_CONSTANT(jgpio, "kGpioErrOk", kGpioErrOk);
    SET_CONSTANT(jgpio, "kGpioErrInitialize", kGpioErrInitialize);
    SET_CONSTANT(jgpio, "kGpioErrNotInitialized", kGpioErrNotInitialized);
    SET_CONSTANT(jgpio, "kGpioErrWrongUse", kGpioErrWrongUse);
    SET_CONSTANT(jgpio, "kGpioErrSysErr", kGpioErrSys);

#undef SET_CONSTANT

    Gpio* gpio = Gpio::Create(*jgpio);
    IOTJS_ASSERT(gpio == reinterpret_cast<Gpio*>(jgpio->GetNative()));

    module->module = jgpio;
  }

  return jgpio;
}


} // namespace iotjs
