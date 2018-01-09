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
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_gpio_t, gpio);
  _this->platform_data = IOTJS_ALLOC(iotjs_gpio_platform_data_t);
}


void iotjs_gpio_destroy_platform_data(
    iotjs_gpio_platform_data_t* platform_data) {
  IOTJS_RELEASE(platform_data);
}


bool iotjs_gpio_open(iotjs_gpio_t* gpio) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_gpio_t, gpio);

  DDDLOG("%s - pin: %d, direction: %d, mode: %d", __func__, _this->pin,
         _this->direction, _this->mode);

  iotbus_gpio_context_h gpio_context = iotbus_gpio_open((int)_this->pin);
  if (gpio_context == NULL) {
    iotbus_gpio_close(gpio_context);
    return false;
  }

  // Set direction
  iotbus_gpio_direction_e direction;
  if (_this->direction == kGpioDirectionIn) {
    direction = IOTBUS_GPIO_DIRECTION_IN;
  } else if (_this->direction == kGpioDirectionOut) {
    direction = IOTBUS_GPIO_DIRECTION_OUT;
  } else {
    direction = IOTBUS_GPIO_DIRECTION_NONE;
  }

  if (iotbus_gpio_set_direction(gpio_context, direction) < 0) {
    iotbus_gpio_close(gpio_context);
    return false;
  }

  _this->platform_data->gpio_context = gpio_context;

  return true;
}


bool iotjs_gpio_write(iotjs_gpio_t* gpio) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_gpio_t, gpio);
  if (iotbus_gpio_write(_this->platform_data->gpio_context, _this->value) < 0) {
    return false;
  }
  return true;
}


bool iotjs_gpio_read(iotjs_gpio_t* gpio) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_gpio_t, gpio);
  if (iotbus_gpio_read(_this->platform_data->gpio_context) < 0) {
    return false;
  }
  return true;
}


bool iotjs_gpio_close(iotjs_gpio_t* gpio) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_gpio_t, gpio);

  if (_this->platform_data->gpio_context &&
      iotbus_gpio_close(_this->platform_data->gpio_context) < 0) {
    return false;
  }
  return true;
}
