/* Copyright 2017-present Samsung Electronics Co., Ltd. and other contributors
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


#include <iotbus_gpio.h>
#include <stdlib.h>

#include "modules/iotjs_module_gpio.h"

struct iotjs_gpio_platform_data_s {
  iotbus_gpio_context_h gpio_context;
};


void iotjs_gpio_create_platform_data(iotjs_gpio_t* gpio) {
  gpio->platform_data = IOTJS_ALLOC(iotjs_gpio_platform_data_t);
}


void iotjs_gpio_destroy_platform_data(
    iotjs_gpio_platform_data_t* platform_data) {
  IOTJS_RELEASE(platform_data);
}


bool iotjs_gpio_open(iotjs_gpio_t* gpio) {
  DDDLOG("%s - pin: %d, direction: %d, mode: %d", __func__, gpio->pin,
         gpio->direction, gpio->mode);

  iotbus_gpio_context_h gpio_context = iotbus_gpio_open((int)gpio->pin);
  if (gpio_context == NULL) {
    iotbus_gpio_close(gpio_context);
    return false;
  }

  // Set direction
  iotbus_gpio_direction_e direction;
  if (gpio->direction == kGpioDirectionIn) {
    direction = IOTBUS_GPIO_DIRECTION_IN;
  } else if (gpio->direction == kGpioDirectionOut) {
    direction = IOTBUS_GPIO_DIRECTION_OUT;
  } else {
    direction = IOTBUS_GPIO_DIRECTION_NONE;
  }

  if (iotbus_gpio_set_direction(gpio_context, direction) < 0) {
    iotbus_gpio_close(gpio_context);
    return false;
  }

  gpio->platform_data->gpio_context = gpio_context;

  return true;
}


bool iotjs_gpio_write(iotjs_gpio_t* gpio) {
  if (iotbus_gpio_write(gpio->platform_data->gpio_context, gpio->value) < 0) {
    return false;
  }
  return true;
}


bool iotjs_gpio_read(iotjs_gpio_t* gpio) {
  int ret = iotbus_gpio_read(gpio->platform_data->gpio_context);
  if (ret < 0) {
    DLOG("%s, Cannot read value(%d).", __func__, ret);
    return false;
  }

  gpio->value = (bool)ret;
  return true;
}


bool iotjs_gpio_close(iotjs_gpio_t* gpio) {
  if (gpio->platform_data->gpio_context &&
      iotbus_gpio_close(gpio->platform_data->gpio_context) < 0) {
    return false;
  }
  return true;
}


bool iotjs_gpio_set_direction(iotjs_gpio_t* gpio) {
  iotbus_gpio_direction_e direction;
  if (gpio->direction == kGpioDirectionIn) {
    direction = IOTBUS_GPIO_DIRECTION_IN;
  } else {
    direction = IOTBUS_GPIO_DIRECTION_OUT;
  }

  int ret =
      iotbus_gpio_set_direction(gpio->platform_data->gpio_context, direction);
  if (ret != 0) {
    DLOG("%s, Cannot set direction(%d).", __func__, ret);
    return false;
  }

  return true;
}
