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


class GpioControlImpl : public GpioControl {
public:
  explicit GpioControlImpl(JObject& jgpioctl);

  virtual int Initialize(void);
  virtual void Release(void);
  virtual int SetPin(GpioCbDataSetpin* setpin_data, GpioSetpinCb cb);
  virtual int SetPin(int32_t pin, int32_t dir, int32_t mode);
};


//-----------------------------------------------------------------------------

GpioControl* GpioControl::Create(JObject& jgpioctl)
{
  return new GpioControlImpl(jgpioctl);
}


GpioControlImpl::GpioControlImpl(JObject& jgpioctl)
    : GpioControl(jgpioctl) {
}


int GpioControlImpl::Initialize(void) {
  if (_fd > 0 )
    return GPIO_ERR_INITALIZE;

  _fd = 1;
  return _fd;
}


void GpioControlImpl::Release(void) {
  _fd = 0;
}


int GpioControlImpl::SetPin(GpioCbDataSetpin* setpin_data, GpioSetpinCb cb) {
  return 0;
}


int GpioControlImpl::SetPin(int32_t pin, int32_t dir, int32_t mode) {
  return 0;
}


} // namespace iotjs
