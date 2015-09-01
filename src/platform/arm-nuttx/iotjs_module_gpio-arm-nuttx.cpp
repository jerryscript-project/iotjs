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


#include "iotjs_module_gpio.h"


namespace iotjs {


// GPIO implementeation for arm-nuttx target.
class GpioArmNuttx : public Gpio {
 public:
  explicit GpioArmNuttx(JObject& jgpio);

  static GpioArmNuttx* GetInstance();

  virtual int Initialize(GpioReqWrap* gpio_req);
  virtual int Release(GpioReqWrap* gpio_req);
  virtual int SetPin(GpioReqWrap* gpio_req);
  virtual int WritePin(GpioReqWrap* gpio_req);
  virtual int ReadPin(GpioReqWrap* gpio_req);
};



Gpio* Gpio::Create(JObject& jgpio) {
  return new GpioArmNuttx(jgpio);
}


GpioArmNuttx::GpioArmNuttx(JObject& jgpio)
    : Gpio(jgpio) {
}


GpioArmNuttx* GpioArmNuttx::GetInstance()
{
  return static_cast<GpioArmNuttx*>(Gpio::GetInstance());
}



int GpioArmNuttx::Initialize(GpioReqWrap* gpio_req) {
  IOTJS_ASSERT(!"Not implemented");
  return 0;
}


int GpioArmNuttx::Release(GpioReqWrap* gpio_req) {
  IOTJS_ASSERT(!"Not implemented");
  return 0;
}


int GpioArmNuttx::SetPin(GpioReqWrap* gpio_req) {
  IOTJS_ASSERT(!"Not implemented");
  return 0;
}


int GpioArmNuttx::WritePin(GpioReqWrap* gpio_req) {
  IOTJS_ASSERT(!"Not implemented");
  return 0;
}


int GpioArmNuttx::ReadPin(GpioReqWrap* gpio_req) {
  IOTJS_ASSERT(!"Not implemented");
  return 0;
}


} // namespace iotjs

#endif // __NUTTX__
