/* Copyright 2018-present Samsung Electronics Co., Ltd. and other contributors
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

#include <stdlib.h>

#include "modules/iotjs_module_gpio.h"

struct iotjs_gpio_platform_data_s {
  bool is_open;
  bool level;
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
  if (gpio->platform_data->is_open) {
    return false; // pin is open already
  }

  gpio->platform_data->is_open = true;
  return true;
}


bool iotjs_gpio_write(iotjs_gpio_t* gpio) {
  gpio->platform_data->level = gpio->value;
  return true;
}


bool iotjs_gpio_read(iotjs_gpio_t* gpio) {
  gpio->value = gpio->platform_data->level;
  return true;
}


bool iotjs_gpio_close(iotjs_gpio_t* gpio) {
  if (!gpio->platform_data->is_open) {
    return false; // pin is not open
  }

  gpio->platform_data->is_open = false;
  return true;
}


bool iotjs_gpio_set_direction(iotjs_gpio_t* gpio) {
  return true;
}
