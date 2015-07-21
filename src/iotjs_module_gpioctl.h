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
#include "iotjs_gpiocbwrap.h"


namespace iotjs {

struct gpio_setpin_s {
  void* data;
  int32_t pin;
  int32_t dir;
  int32_t mode;
};

typedef struct gpio_setpin_s gpio_setpin_t;


typedef void (*gpio_setpin_cb)(gpio_cb_t* gpiocb, int err);


class GpioControl : public JObjectWrap {
public:
  explicit GpioControl(JObject& jgpioctl);
  virtual ~GpioControl();

  static GpioControl* Create(JObject& jgpioctl);
  static GpioControl* FromJGpioCtl(JObject& jgpioctl);

  virtual int Initialize(void);
  virtual void Release(void);
  virtual int SetPin(gpio_setpin_t* setpin, gpio_setpin_cb cb);
  virtual int SetPin(int32_t pin, int32_t dir, int32_t mode);

protected:
  int _fd;
};


JObject* InitGpioCtl();



enum {
  GPIO_ERR_INTIALIZED = -1,
  GPIO_ERR_INVALIDPARAM = -2,
};

enum {
  GPIO_DIR_NONE = 0,
  GPIO_DIR_IN,
  GPIO_DIR_OUT,
};

enum {
  GPIO_MODE_NONE = 0,
  GPIO_MODE_PULLUP,
  GPIO_MODE_PULLDN,
  GPIO_MODE_FLOAT,
  GPIO_MODE_PUSHPULL,
  GPIO_MODE_OPENDRAIN,
};


} // namespace iotjs

#endif /* IOTJS_MODULE_GPIOCTL_H */
