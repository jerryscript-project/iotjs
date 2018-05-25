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

#include <peripheral_io.h>
#include <stdlib.h>

#include "modules/iotjs_module_gpio.h"

struct iotjs_gpio_platform_data_s {
  peripheral_gpio_h peripheral_gpio;
};

void iotjs_gpio_create_platform_data(iotjs_gpio_t* gpio) {
  gpio->platform_data = IOTJS_ALLOC(iotjs_gpio_platform_data_t);
}

void iotjs_gpio_destroy_platform_data(
    iotjs_gpio_platform_data_t* platform_data) {
  IOTJS_RELEASE(platform_data);
}

bool iotjs_gpio_write(iotjs_gpio_t* gpio) {
  int retVal =
      peripheral_gpio_write(gpio->platform_data->peripheral_gpio, gpio->value);
  return PERIPHERAL_ERROR_NONE == retVal;
}


bool iotjs_gpio_read(iotjs_gpio_t* gpio) {
  uint32_t value;
  int retVal =
      peripheral_gpio_read(gpio->platform_data->peripheral_gpio, &value);
  if (retVal != PERIPHERAL_ERROR_NONE) {
    return false;
  }

  gpio->value = (bool)value;
  return true;
}


bool iotjs_gpio_close(iotjs_gpio_t* gpio) {
  peripheral_gpio_close(gpio->platform_data->peripheral_gpio);
  return true;
}


bool iotjs_gpio_open(iotjs_gpio_t* gpio) {
  peripheral_gpio_h _gpio;
  int retVal = peripheral_gpio_open((int)gpio->pin, &_gpio);
  if (retVal != PERIPHERAL_ERROR_NONE) {
    return false;
  }

  gpio->platform_data->peripheral_gpio = _gpio;
  peripheral_gpio_direction_e _direction;

  if (gpio->direction == kGpioDirectionIn) {
    _direction = PERIPHERAL_GPIO_DIRECTION_IN;
  } else {
    _direction = PERIPHERAL_GPIO_DIRECTION_OUT_INITIALLY_LOW;
  }

  retVal = peripheral_gpio_set_direction(_gpio, _direction);
  if (retVal != PERIPHERAL_ERROR_NONE) {
    return false;
  }

  // Mode is not supported by Peripheral API for Tizen
  peripheral_gpio_edge_e _edge;
  switch (gpio->edge) {
    case kGpioEdgeNone:
      _edge = PERIPHERAL_GPIO_EDGE_NONE;
      break;
    case kGpioEdgeRising:
      _edge = PERIPHERAL_GPIO_EDGE_RISING;
      break;
    case kGpioEdgeFalling:
      _edge = PERIPHERAL_GPIO_EDGE_FALLING;
      break;
    case kGpioEdgeBoth:
      _edge = PERIPHERAL_GPIO_EDGE_BOTH;
      break;
    default:
      _edge = PERIPHERAL_GPIO_EDGE_NONE;
  }

  retVal = peripheral_gpio_set_edge_mode(_gpio, _edge);
  if (retVal != PERIPHERAL_ERROR_NONE) {
    return false;
  }

  return true;
}


bool iotjs_gpio_set_direction(iotjs_gpio_t* gpio) {
  peripheral_gpio_direction_e direction;
  if (gpio->direction == kGpioDirectionIn) {
    direction = PERIPHERAL_GPIO_DIRECTION_IN;
  } else {
    direction = PERIPHERAL_GPIO_DIRECTION_OUT_INITIALLY_LOW;
  }

  int ret = peripheral_gpio_set_direction(gpio->platform_data->peripheral_gpio,
                                          direction);
  if (ret != PERIPHERAL_ERROR_NONE) {
    DLOG("%s, Cannot set direction(%d).", __func__, ret);
    return false;
  }

  return true;
}
