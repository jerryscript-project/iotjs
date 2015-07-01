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

#if defined(__NUTTX__)

#include "iotjs_def.h"
#include "iotjs_objectwrap.h"
#include "iotjs_module_gpioctl.h"

#include <unistd.h>
#include <fcntl.h>
#include <nuttx/gpio.h>
#include <sys/ioctl.h>


namespace iotjs {


class GpioControlInst : public GpioControl {
public:
  explicit GpioControlInst(JObject& jgpioctl);
  virtual int Initialize(void);
  virtual void Release(void);
  virtual int PinMode(uint32_t portpin);
  virtual int WritePin(uint32_t portpin, uint8_t data);
  virtual int ReadPin(uint32_t portpin, uint8_t* pdata);
};

//-----------------------------------------------------------------------------

GpioControl* GpioControl::Create(JObject& jgpioctl)
{
  return new GpioControlInst(jgpioctl);
}


GpioControlInst::GpioControlInst(JObject& jgpioctl)
    : GpioControl(jgpioctl) {
}


int GpioControlInst::Initialize(void) {
  if (_fd > 0 )
    return IOTJS_GPIO_INUSE;

  const char* devfilepath = "/dev/gpio";
  _fd = open(devfilepath, O_RDWR);
  DDDLOG("gpio> %s : fd(%d)", devfilepath, _fd);
  return _fd;
}

void GpioControlInst::Release(void) {
  if (_fd > 0) {
    close(_fd);
  }
  _fd = 0;
}


int GpioControlInst::PinMode(uint32_t portpin) {
  if (_fd > 0) {
    struct gpioioctl_config_s cdata;
    cdata.port = portpin;
    return ioctl(_fd, GPIOIOCTL_CONFIG, (long unsigned int)&cdata);
  }
  return IOTJS_GPIO_NOTINITED;
}


int GpioControlInst::WritePin(uint32_t portpin, uint8_t data) {
  if (_fd > 0) {
    struct gpioioctl_write_s wdata;
    wdata.port = portpin;
    wdata.data = data;
    return ioctl(_fd, GPIOIOCTL_WRITE, (long unsigned int)&wdata);
  }
  return IOTJS_GPIO_NOTINITED;
}


int GpioControlInst::ReadPin(uint32_t portpin, uint8_t* pdata) {
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
  return IOTJS_GPIO_NOTINITED;
}


} // namespace iotjs

#endif // __NUTTX__
