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

class SetpinWrap : public GpioCbWrap {
 public:
  explicit SetpinWrap(JObject& jcallback, JObject& jgpioctl)
      : GpioCbWrap(jcallback, reinterpret_cast<GpioCbData*>(&_data))
      , _jgpioctl(jgpioctl) {
    memset(&_data, 0, sizeof(_data));
  }

  GpioCbDataSetpin* data() {
    return &_data;
  }

  JObject& jgpioctl() {
    return _jgpioctl;
  }

 protected:
   GpioCbDataSetpin _data;
   JObject _jgpioctl;
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


//-----------------------------------------------------------------------------

JHANDLER_FUNCTION(Initialize, handler) {
  JObject* jgpioctl = handler.GetThis();
  GpioControl* gpioctrl = GpioControl::FromJGpioCtl(*jgpioctl);

  int err = gpioctrl->Initialize();

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


static void AfterSetPin(GpioCbDataSetpin* cdbata, int err) {
  SetpinWrap* wrap = reinterpret_cast<SetpinWrap*>(cdbata->data);
  IOTJS_ASSERT(wrap != NULL);

  JObject jcallback(wrap->jcallback());
  IOTJS_ASSERT(jcallback.IsFunction());
  JObject jgpioctl(wrap->jgpioctl());

  JArgList args(2);
  args.Add(jcallback);
  if (err < 0) args.Add(JVal::Number(err));
  else args.Add(JVal::Null());

  JObject callback(jgpioctl.GetProperty("_docallback"));
  IOTJS_ASSERT(callback.IsFunction());
  MakeCallback(callback, JObject::Null(), args);
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
    SetpinWrap* wrap = new SetpinWrap(jcallback, *jgpioctl);
    GpioCbDataSetpin* setpindata = wrap->data();
    setpindata->pin = pin;
    setpindata->dir = dir;
    setpindata->mode = mode;

    err = gpioctrl->SetPin(setpindata, AfterSetPin);

    wrap->Dispatched();

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


JHANDLER_FUNCTION(ErrMessage, handler) {
  IOTJS_ASSERT(handler.GetArgLength() == 1);
  IOTJS_ASSERT(handler.GetArg(0)->IsNumber());
  int32_t err = handler.GetArg(0)->GetInt32();
  const char* str = "Unknown error";
  switch(err) {
  case GPIO_ERR_INITALIZE : str = "Failed to initialize"; break;
  case GPIO_ERR_INVALIDPARAM : str = "Invalid parameter"; break;
  case GPIO_ERR_SYSERR : str = "System error"; break;
  }
  JObject ret(str);
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
    jgpioctl->SetMethod("errMessage", ErrMessage);

    SET_CONSTANT(jgpioctl, "NONE", GPIO_DIR_NONE);
    SET_CONSTANT(jgpioctl, "IN", GPIO_DIR_IN);
    SET_CONSTANT(jgpioctl, "OUT", GPIO_DIR_OUT);

    SET_CONSTANT(jgpioctl, "PULLUP", GPIO_MODE_PULLUP);
    SET_CONSTANT(jgpioctl, "PULLDN", GPIO_MODE_PULLDN);
    SET_CONSTANT(jgpioctl, "FLOAT", GPIO_MODE_FLOAT);
    SET_CONSTANT(jgpioctl, "PUSHPULL", GPIO_MODE_PUSHPULL);
    SET_CONSTANT(jgpioctl, "OPENDRAIN", GPIO_MODE_OPENDRAIN);

    SET_CONSTANT(jgpioctl, "ERR_INITALIZE", GPIO_ERR_INITALIZE);
    SET_CONSTANT(jgpioctl, "ERR_INVALIDPARAM", GPIO_ERR_INVALIDPARAM);
    SET_CONSTANT(jgpioctl, "ERR_SYSERR", GPIO_ERR_SYSERR);

    GpioControl* gpioctrl = GpioControl::Create(*jgpioctl);
    IOTJS_ASSERT(gpioctrl ==
                 reinterpret_cast<GpioControl*>(jgpioctl->GetNative()));

    module->module = jgpioctl;
  }

  return jgpioctl;
}


} // namespace iotjs
