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

#ifndef IOTJS_MODULE_GPIOCTL_H
#define IOTJS_MODULE_GPIOCTL_H

#include "iotjs_binding.h"
#include "iotjs_objectwrap.h"


namespace iotjs {

enum {
  IOTJS_GPIO_NOTINITED = -1,
  IOTJS_GPIO_INUSE  = -2,
};


class GpioControl : public JObjectWrap {
public:
  explicit GpioControl(JObject& jgpioctl);
  virtual ~GpioControl();

  static GpioControl* Create(JObject& jgpioctl);
  static GpioControl* FromJGpioCtl(JObject& jgpioctl);

  virtual int Initialize(void);
  virtual void Release(void);
  virtual int PinMode(uint32_t portpin);
  virtual int WritePin(uint32_t portpin, uint8_t data);
  virtual int ReadPin(uint32_t portpin, uint8_t* pdata);

protected:
  int _fd;
};


JObject* InitGpioCtl();

} // namespace iotjs

#endif /* IOTJS_MODULE_GPIOCTL_H */
