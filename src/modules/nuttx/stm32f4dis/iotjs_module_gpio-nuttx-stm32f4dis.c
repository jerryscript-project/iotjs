/* Copyright 2016-present Samsung Electronics Co., Ltd. and other contributors
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


#include "modules/iotjs_module_gpio.h"
#include "stm32_gpio.h"

#define GPIO_FREQUENCY_DEFAULT GPIO_SPEED_25MHz
#define GPIO_PINCNT_IN_NUTTXPORT 16

#define GPIO_CONFIG_OK 0


uint32_t gpioDirection[] = {
  GPIO_INPUT, GPIO_OUTPUT | GPIO_OUTPUT_SET | GPIO_FREQUENCY_DEFAULT,
};


uint32_t gpioMode[] = {
  0, // none
  GPIO_PULLUP, GPIO_PULLDOWN, GPIO_FLOAT, GPIO_PUSHPULL, GPIO_OPENDRAIN,
};


void iotjs_gpio_platform_create(iotjs_gpio_t_impl_t* _this) {
}


void iotjs_gpio_platform_destroy(iotjs_gpio_t_impl_t* _this) {
}


bool iotjs_gpio_write(iotjs_gpio_t* gpio, bool value) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_gpio_t, gpio);

  DDDLOG("%s - pin: %d, value: %d", __func__, _this->pin, value);
  stm32_gpiowrite(_this->pin, value);

  return true;
}


int iotjs_gpio_read(iotjs_gpio_t* gpio) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_gpio_t, gpio);

  DDDLOG("%s - pin: %d", __func__, _this->pin);
  return stm32_gpioread(_this->pin);
}


bool iotjs_gpio_close(iotjs_gpio_t* gpio) {
  iotjs_gpio_write(gpio, 0);

  return true;
}


void iotjs_gpio_open_worker(uv_work_t* work_req) {
  GPIO_WORKER_INIT;
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_gpio_t, gpio);

  DDDLOG("%s - pin: %d, dir: %d, mode: %d", __func__, _this->pin,
         _this->direction, _this->mode);

  uint32_t cfgset = 0;

  // Set pin direction and mode
  cfgset = gpioDirection[_this->direction] | gpioMode[_this->mode] | _this->pin;

  if (stm32_configgpio(cfgset) != GPIO_CONFIG_OK) {
    req_data->result = false;
    return;
  }

  req_data->result = true;
}
