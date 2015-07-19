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
#include "iotjs_module_gpioctl.h"


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

  _fd = 1;
  return _fd;
}


void GpioControlInst::Release(void) {
  _fd = 0;
}


int GpioControlInst::PinMode(uint32_t portpin) {
  if (_fd <= 0)
    return IOTJS_GPIO_NOTINITED;
  return 0;
}


int GpioControlInst::WritePin(uint32_t portpin, uint8_t data) {
  if (_fd <= 0)
    return IOTJS_GPIO_NOTINITED;
  return 0;
}


int GpioControlInst::ReadPin(uint32_t portpin, uint8_t* pdata) {
  if (_fd <= 0)
    return IOTJS_GPIO_NOTINITED;
  return 0;
}


} // namespace iotjs
