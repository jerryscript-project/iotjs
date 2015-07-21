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
#include "iotjs_module_gpioctl.h"


namespace iotjs {


//-----------------------------------------------------------------------------

GpioCbWrap::GpioCbWrap(JObject& jcallback, gpio_cb_t* cb)
    : __cb(cb)
    , _jcallback(NULL) {
  if (!jcallback.IsNull()) {
    _jcallback = new JObject(jcallback);
  }
}


GpioCbWrap::~GpioCbWrap() {
  if (_jcallback != NULL) {
    delete _jcallback;
  }
}


JObject& GpioCbWrap::jcallback() {
  IOTJS_ASSERT(_jcallback != NULL);
  return *_jcallback;
}


gpio_cb_t* GpioCbWrap::cb() {
  return __cb;
}


void GpioCbWrap::Dispatched() {
  cb()->data = this;
}

//-----------------------------------------------------------------------------

class SetPinWrap : public GpioCbWrap {
 public:
  explicit SetPinWrap(JObject& jcallback)
      : GpioCbWrap(jcallback, reinterpret_cast<gpio_cb_t*>(&_data)) {
    memset(&_data, 0, sizeof(_data));
  }

  gpio_setpin_t* data() {
    return &_data;
  }

 protected:
   gpio_setpin_t _data;
};


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
    return GPIO_ERR_INTIALIZED;

  DDDLOG("gpio initalize dummy");
  _fd = 1;
  return _fd;
}


void GpioControl::Release(void) {
  DDDLOG("gpio release dummy");
  _fd = 0;
}


int GpioControl::SetPin(gpio_setpin_t* setpin_data, gpio_setpin_cb cb) {
  DDDLOG("gpio setpin dummy pin(%d), dir(%d), mode(%d)",
         setpin_data->pin, setpin_data->dir, setpin_data->mode);
  // temporary code to invoke cb in this thread and it's not async
  cb(reinterpret_cast<gpio_cb_t*>(setpin_data), 0);
  return 0;
}


int GpioControl::SetPin(int32_t pin, int32_t dir, int32_t mode) {
  DDDLOG("gpio setpin sync dummy pin(%d), dir(%d), mode(%d)",
         pin, dir, mode);
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

  return true;
}


static void AfterSetPin(gpio_cb_t* gpiocb, int err) {
  SetPinWrap* wrap = reinterpret_cast<SetPinWrap*>(gpiocb->data);
  IOTJS_ASSERT(wrap != NULL);

  JObject jcallback(wrap->jcallback());
  IOTJS_ASSERT(jcallback.IsFunction());

  JArgList args(1);
  args.Add(JVal::Number(err));

  MakeCallback(jcallback, JObject::Null(), args);
  delete wrap;
}


JHANDLER_FUNCTION(SetPin, handler) {
  IOTJS_ASSERT(handler.GetArgLength() == 4);
  IOTJS_ASSERT(handler.GetArg(0)->IsNumber());
  IOTJS_ASSERT(handler.GetArg(1)->IsNumber());
  IOTJS_ASSERT(handler.GetArg(2)->IsNumber());
  JObject* jgpioctl = handler.GetThis();
  GpioControl* gpioctrl = GpioControl::FromJGpioCtl(*jgpioctl);
  int32_t pin = handler.GetArg(0)->GetInt32();
  int32_t dir = handler.GetArg(1)->GetInt32();
  int32_t mode = handler.GetArg(2)->GetInt32();
  JObject jcallback = *handler.GetArg(3);
  int err = 0;

  if (jcallback.IsFunction()) {
    JObject jcallback = *handler.GetArg(3);
    SetPinWrap* wrap = new SetPinWrap(jcallback);
    gpio_setpin_t* setpin = wrap->data();
    setpin->pin = pin;
    setpin->dir = dir;
    setpin->mode = mode;

    // as current implementation of gpio is in sync mode,
    // Dispatched should be called prior to SetPin
    wrap->Dispatched();
    err = gpioctrl->SetPin(setpin, AfterSetPin);

    if (err) {
      delete wrap;
    }
  }
  else {
    err = gpioctrl->SetPin(pin, dir, mode);
  }

  JObject ret(err);
  handler.Return(ret);
  return true;
}


JHANDLER_FUNCTION(WritePin, handler) {
  int err = 0;
  JObject ret(err);
  handler.Return(ret);
  return true;
}


JHANDLER_FUNCTION(ReadPin, handler) {
  int err = 0;
  JObject ret(err);
  handler.Return(ret);
  return true;
}


JHANDLER_FUNCTION(SetPort, handler) {
  int err = 0;
  JObject ret(err);
  handler.Return(ret);
  return true;
}


JHANDLER_FUNCTION(WritePort, handler) {
  int err = 0;
  JObject ret(err);
  handler.Return(ret);
  return true;
}


JHANDLER_FUNCTION(ReadPort, handler) {
  int err = 0;
  JObject ret(err);
  handler.Return(ret);
  return true;
}


JHANDLER_FUNCTION(Query, handler) {
  int err = 0;
  JObject ret(err);
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
    jgpioctl->SetMethod("setPin", SetPin);
    jgpioctl->SetMethod("writePin", WritePin);
    jgpioctl->SetMethod("readPin", ReadPin);
    jgpioctl->SetMethod("setPort", SetPort);
    jgpioctl->SetMethod("writePort", WritePort);
    jgpioctl->SetMethod("readPort", ReadPort);
    jgpioctl->SetMethod("query", Query);

    SET_CONSTANT(jgpioctl, "NONE", GPIO_DIR_NONE);
    SET_CONSTANT(jgpioctl, "IN", GPIO_DIR_IN);
    SET_CONSTANT(jgpioctl, "OUT", GPIO_DIR_OUT);

    SET_CONSTANT(jgpioctl, "PULLUP", GPIO_MODE_PULLUP);
    SET_CONSTANT(jgpioctl, "PULLDN", GPIO_MODE_PULLDN);
    SET_CONSTANT(jgpioctl, "FLOAT", GPIO_MODE_FLOAT);
    SET_CONSTANT(jgpioctl, "PUSHPULL", GPIO_MODE_PUSHPULL);
    SET_CONSTANT(jgpioctl, "OPENDRAIN", GPIO_MODE_OPENDRAIN);

    SET_CONSTANT(jgpioctl, "ERR_INTIALIZED", GPIO_ERR_INTIALIZED);
    SET_CONSTANT(jgpioctl, "ERR_INVALIDPARAM", GPIO_ERR_INVALIDPARAM);

    GpioControl* gpioctrl = GpioControl::Create(*jgpioctl);
    IOTJS_ASSERT(gpioctrl ==
                 reinterpret_cast<GpioControl*>(jgpioctl->GetNative()));

    module->module = jgpioctl;
  }

  return jgpioctl;
}


} // namespace iotjs
