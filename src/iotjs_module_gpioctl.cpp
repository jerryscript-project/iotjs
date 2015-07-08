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

#include "iotjs_def.h"
#include "iotjs_objectwrap.h"
#include "iotjs_module_gpioctl.h"


namespace iotjs {


//-----------------------------------------------------------------------------

GpioControl::GpioControl(JObject& jgpioctl)
    : JObjectWrap(jgpioctl)
    , _fd(0) {
}


GpioControl::~GpioControl() {
  IOTJS_ASSERT(_fd <= 0);
}


GpioControl* GpioControl::FromJGpioCtl(JObject& jgpioctl) {
  GpioControl* gpioctrl = reinterpret_cast<GpioControl*>(jgpioctl.GetNative());
  IOTJS_ASSERT(gpioctrl != NULL);
  return gpioctrl;
}


int GpioControl::Initialize(void) {
  if (_fd > 0 )
    return IOTJS_GPIO_INUSE;

  DDDLOG("gpio initalize dummy");
  _fd = 1;
  return _fd;
}


void GpioControl::Release(void) {
  DDDLOG("gpio release dummy");
  _fd = 0;
}


int GpioControl::PinMode(uint32_t portpin) {
  DDDLOG("gpio pin mode 0x%08x", portpin);
  return 0;
}


int GpioControl::WritePin(uint32_t portpin, uint8_t data) {
  DDDLOG("gpio write 0x%08x: 0x%02x", portpin, data);
  return 0;
}


int GpioControl::ReadPin(uint32_t portpin, uint8_t* pdata) {
  DDDLOG("gpio read 0x%08x", portpin);
  return 0;
}


//-----------------------------------------------------------------------------

JHANDLER_FUNCTION(Initialize, handler) {
  JObject* jgpioctl = handler.GetThis();
  GpioControl* gpioctrl = GpioControl::FromJGpioCtl(*jgpioctl);

  int err = gpioctrl->Initialize();
  DDDLOG("gpio initialize %d", err);

  JObject ret(err);
  handler.Return(ret);
  return true;
}


JHANDLER_FUNCTION(Release, handler) {
  JObject* jgpioctl = handler.GetThis();
  GpioControl* gpioctrl = GpioControl::FromJGpioCtl(*jgpioctl);

  gpioctrl->Release();

  JObject ret(0);
  handler.Return(ret);
  return true;
}


JHANDLER_FUNCTION(PinMode, handler) {
  IOTJS_ASSERT(handler.GetArgLength() == 1);
  IOTJS_ASSERT(handler.GetArg(0)->IsNumber());
  JObject* jgpioctl = handler.GetThis();
  GpioControl* gpioctrl = GpioControl::FromJGpioCtl(*jgpioctl);

  uint32_t portpin = handler.GetArg(0)->GetInt32();
  int err = gpioctrl->PinMode(portpin);

  JObject ret(err);
  handler.Return(ret);
  return true;
}


JHANDLER_FUNCTION(WritePin, handler) {
  IOTJS_ASSERT(handler.GetArgLength() == 2);
  IOTJS_ASSERT(handler.GetArg(0)->IsNumber());
  IOTJS_ASSERT(handler.GetArg(1)->IsNumber());
  JObject* jgpioctl = handler.GetThis();
  GpioControl* gpioctrl = GpioControl::FromJGpioCtl(*jgpioctl);

  uint32_t portpin = handler.GetArg(0)->GetInt64();
  uint8_t data = (uint8_t)handler.GetArg(1)->GetInt32();
  int err = gpioctrl->WritePin(portpin, data);

  JObject ret(err);
  handler.Return(ret);
  return true;
}


JHANDLER_FUNCTION(ReadPin, handler) {
  IOTJS_ASSERT(handler.GetArgLength() == 1);
  IOTJS_ASSERT(handler.GetArg(0)->IsNumber());
  JObject* jgpioctl = handler.GetThis();
  GpioControl* gpioctrl = GpioControl::FromJGpioCtl(*jgpioctl);

  uint32_t portpin = handler.GetArg(0)->GetInt64();
  uint8_t data = 0;
  int err = gpioctrl->ReadPin(portpin, &data);

  JObject ret(err < 0 ? err : data);
  handler.Return(ret);
  return true;
}


#define SET_CONSTANT(object, name, constant) \
  do { \
    JObject value(constant); \
    object->SetProperty(name, value); \
  } while (0)


JObject* InitGpioCtl() {
  Module* module = GetBuiltinModule(MODULE_GPIOCTL);
  JObject* jgpioctl = module->module;

  if (jgpioctl == NULL) {
    jgpioctl = new JObject();

    jgpioctl->SetMethod("initialize", Initialize);
    jgpioctl->SetMethod("release", Release);
    jgpioctl->SetMethod("pinmode", PinMode);
    jgpioctl->SetMethod("writepin", WritePin);
    jgpioctl->SetMethod("readpin", ReadPin);

    SET_CONSTANT(jgpioctl, "NOTINITIALIZED", IOTJS_GPIO_NOTINITED);
    SET_CONSTANT(jgpioctl, "INUSE", IOTJS_GPIO_INUSE);

    GpioControl* gpioctrl = GpioControl::Create(*jgpioctl);
    IOTJS_ASSERT(gpioctrl ==
                 reinterpret_cast<GpioControl*>(jgpioctl->GetNative()));

    module->module = jgpioctl;
  }

  return jgpioctl;
}


} // namespace iotjs
