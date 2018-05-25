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


void iotjs_gpio_create_platform_data(iotjs_gpio_t* gpio) {
}


void iotjs_gpio_destroy_platform_data(
    iotjs_gpio_platform_data_t* platform_data) {
}


bool iotjs_gpio_write(iotjs_gpio_t* gpio) {
  DDDLOG("%s - pin: %d, value: %d", __func__, gpio->pin, gpio->value);
  stm32_gpiowrite(gpio->pin, gpio->value);

  return true;
}


bool iotjs_gpio_read(iotjs_gpio_t* gpio) {
  DDDLOG("%s - pin: %d", __func__, gpio->pin);

  gpio->value = stm32_gpioread(gpio->pin);
  return true;
}


bool iotjs_gpio_close(iotjs_gpio_t* gpio) {
  iotjs_gpio_write(gpio);

  return true;
}


static bool gpio_set_config(iotjs_gpio_t* gpio) {
  uint32_t cfgset = 0;

  // Set pin direction and mode
  cfgset = gpioDirection[gpio->direction] | gpioMode[gpio->mode] | gpio->pin;

  if (stm32_configgpio(cfgset) != GPIO_CONFIG_OK) {
    return false;
  }

  return true;
}


bool iotjs_gpio_open(iotjs_gpio_t* gpio) {
  DDDLOG("%s - pin: %d, dir: %d, mode: %d", __func__, gpio->pin,
         gpio->direction, gpio->mode);
  return gpio_set_config(gpio);
}


bool iotjs_gpio_set_direction(iotjs_gpio_t* gpio) {
  return gpio_set_config(gpio);
}
