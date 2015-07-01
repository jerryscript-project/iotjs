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

#include <unistd.h>
#include <fcntl.h>

#if defined(__NUTTX__)
#include <nuttx/gpio.h>
#include <sys/ioctl.h>
#endif


namespace iotjs {


enum {
  IOTJS_GPIO_NOTINITED = -1,
  IOTJS_GPIO_INUSE  = -2,
};


class GpioControl : public JObjectWrap {
public:
  explicit GpioControl(JObject& jgpioctl);
  virtual ~GpioControl();

  static GpioControl* FromJGpioCtl(JObject& jgpioctl);

  int Initialize(const char* devpath);
  void Release(void);
  int PinMode(uint32_t portpin);
  int WritePin(uint32_t portpin, uint8_t data);
  int ReadPin(uint32_t portpin, uint8_t* pdata);

protected:
  int _fd;
};

//-----------------------------------------------------------------------------

GpioControl::GpioControl(JObject& jgpioctl)
    : JObjectWrap(jgpioctl)
    , _fd(0) {
  DDDLOG("gpio CTOR");
}


GpioControl::~GpioControl() {
  DDDLOG("gpio DTOR");
  IOTJS_ASSERT(_fd <= 0);
}


GpioControl* GpioControl::FromJGpioCtl(JObject& jgpioctl) {
  GpioControl* gpioctrl = reinterpret_cast<GpioControl*>(jgpioctl.GetNative());
  IOTJS_ASSERT(gpioctrl != NULL);
  return gpioctrl;
}


int GpioControl::Initialize(const char* devpath) {
  if (_fd > 0 )
    return IOTJS_GPIO_INUSE;
#if defined(__NUTTX__)
  _fd = open(devpath, O_RDWR);
  DDDLOG("gpio> %s : fd(%d)", devpath, _fd);
#else
  DDDLOG("gpio opendevice %s", devpath);
  _fd = 1;
#endif
  return _fd;
}

void GpioControl::Release(void) {
#if defined(__NUTTX__)
  if (_fd > 0) {
    close(_fd);
  }
#endif
  _fd = 0;
}


int GpioControl::PinMode(uint32_t portpin) {
#if defined(__NUTTX__)
  if (_fd > 0) {
    struct gpioioctl_config_s cdata;
    cdata.port = portpin;
    return ioctl(_fd, GPIOIOCTL_CONFIG, (long unsigned int)&cdata);
  }
#else
  DDDLOG("gpio pin mode 0x%08x", portpin);
  return 0;
#endif
  return IOTJS_GPIO_NOTINITED;
}


int GpioControl::WritePin(uint32_t portpin, uint8_t data) {
#if defined(__NUTTX__)
  if (_fd > 0) {
    struct gpioioctl_write_s wdata;
    wdata.port = portpin;
    wdata.data = data;
    return ioctl(_fd, GPIOIOCTL_WRITE, (long unsigned int)&wdata);
  }
#else
  DDDLOG("gpio write 0x%08x: 0x%02x", portpin, data);
  return 0;
#endif
  return IOTJS_GPIO_NOTINITED;
}

int GpioControl::ReadPin(uint32_t portpin, uint8_t* pdata) {
#if defined(__NUTTX__)
  if (_fd > 0) {
    struct gpioioctl_write_s wdata;
    int ret;
    wdata.port = portpin;
    wdata.port = *pdata = 0;
    ret = ioctl(_fd, GPIOIOCTL_READ, (long unsigned int)&wdata);
    if (ret >= 0) {
      *pdata = wdata.data;
    }
    return ret;
  }
#else
  DDDLOG("gpio read 0x%08x", portpin);
  return 0;
#endif
  return IOTJS_GPIO_NOTINITED;
}


//-----------------------------------------------------------------------------

JHANDLER_FUNCTION(Initialize, handler) {
  IOTJS_ASSERT(handler.GetArgLength() == 1);
  IOTJS_ASSERT(handler.GetArg(0)->IsString());
  JObject* jgpioctl = handler.GetThis();
  GpioControl* gpioctrl = GpioControl::FromJGpioCtl(*jgpioctl);

  LocalString src(handler.GetArg(0)->GetCString());
  int err = gpioctrl->Initialize(src);
  DDDLOG("gpio initialize %s %d", (const char*)src, err);

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

    GpioControl* gpioctrl = new GpioControl(*jgpioctl);
    IOTJS_ASSERT(gpioctrl ==
                 reinterpret_cast<GpioControl*>(jgpioctl->GetNative()));

    module->module = jgpioctl;
  }

  return jgpioctl;
}


} // namespace iotjs
