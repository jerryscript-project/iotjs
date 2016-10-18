/* Copyright 2016 Samsung Electronics Co., Ltd.
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

#ifndef IOTJS_MODULE_GPIO_ARM_NUTTX_GENERAL_INL_H
#define IOTJS_MODULE_GPIO_ARM_NUTTX_GENERAL_INL_H

#include "module/iotjs_module_gpio.h"


namespace iotjs {


// GPIO implementation for arm-nuttx target.
class GpioArmNuttxGeneral : public Gpio {
 public:
  explicit GpioArmNuttxGeneral(JObject& jgpio);

  static GpioArmNuttxGeneral* GetInstance();

  virtual int Initialize(GpioReqWrap* gpio_req);
  virtual int Release(GpioReqWrap* gpio_req);
  virtual int SetPin(GpioReqWrap* gpio_req);
  virtual int WritePin(GpioReqWrap* gpio_req);
  virtual int ReadPin(GpioReqWrap* gpio_req);
  virtual int SetPort(GpioReqWrap* gpio_req);
  virtual int WritePort(GpioReqWrap* gpio_req);
  virtual int ReadPort(GpioReqWrap* gpio_req);
};


Gpio* Gpio::Create(JObject& jgpio) {
  return new GpioArmNuttxGeneral(jgpio);
}


GpioArmNuttxGeneral::GpioArmNuttxGeneral(JObject& jgpio)
    : Gpio(jgpio) {
}


GpioArmNuttxGeneral* GpioArmNuttxGeneral::GetInstance() {
  return static_cast<GpioArmNuttxGeneral*>(Gpio::GetInstance());
}


int GpioArmNuttxGeneral::Initialize(GpioReqWrap* gpio_req) {
  IOTJS_ASSERT(!"Not implemented");
  return 0;
}


int GpioArmNuttxGeneral::Release(GpioReqWrap* gpio_req) {
  IOTJS_ASSERT(!"Not implemented");
  return 0;
}


int GpioArmNuttxGeneral::SetPin(GpioReqWrap* gpio_req) {
  IOTJS_ASSERT(!"Not implemented");
  return 0;
}


int GpioArmNuttxGeneral::WritePin(GpioReqWrap* gpio_req) {
  IOTJS_ASSERT(!"Not implemented");
  return 0;
}


int GpioArmNuttxGeneral::ReadPin(GpioReqWrap* gpio_req) {
  IOTJS_ASSERT(!"Not implemented");
  return 0;
}


int GpioArmNuttxGeneral::SetPort(GpioReqWrap* gpio_req) {
  IOTJS_ASSERT(!"Not implemented");
  return 0;
}


int GpioArmNuttxGeneral::WritePort(GpioReqWrap* gpio_req) {
  IOTJS_ASSERT(!"Not implemented");
  return 0;
}


int GpioArmNuttxGeneral::ReadPort(GpioReqWrap* gpio_req) {
  IOTJS_ASSERT(!"Not implemented");
  return 0;
}


} // namespace iotjs

#endif /* IOTJS_MODULE_GPIO_ARM_NUTTX_GENERAL_INL_H */
