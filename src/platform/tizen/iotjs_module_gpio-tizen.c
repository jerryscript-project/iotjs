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

struct _iotjs_gpio_module_platform_t {
  peripheral_gpio_h peripheral_gpio;
};

void iotjs_gpio_platform_create(iotjs_gpio_t_impl_t* _this) {
  size_t private_mem = sizeof(struct _iotjs_gpio_module_platform_t);
  _this->platform = (iotjs_gpio_module_platform_t)malloc(private_mem);
}

void iotjs_gpio_platform_destroy(iotjs_gpio_t_impl_t* _this) {
  iotjs_buffer_release((char*)_this->platform);
}

bool iotjs_gpio_write(iotjs_gpio_t* gpio, bool value) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_gpio_t, gpio);
  int retVal = peripheral_gpio_write(_this->platform->peripheral_gpio, value);
  return PERIPHERAL_ERROR_NONE == retVal;
}


int iotjs_gpio_read(iotjs_gpio_t* gpio) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_gpio_t, gpio);
  int value;
  int retVal = peripheral_gpio_read(_this->platform->peripheral_gpio, &value);
  if (PERIPHERAL_ERROR_NONE == retVal) {
    return value;
  } else {
    return -1;
  }
}


bool iotjs_gpio_close(iotjs_gpio_t* gpio) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_gpio_t, gpio);
  peripheral_gpio_close(_this->platform->peripheral_gpio);
  return true;
}


void iotjs_gpio_open_worker(uv_work_t* work_req) {
  GPIO_WORKER_INIT;
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_gpio_t, gpio);

  peripheral_gpio_h _gpio;
  int retVal = peripheral_gpio_open((int)_this->pin, &_gpio);
  if (retVal != PERIPHERAL_ERROR_NONE) {
    req_data->result = false;
    return;
  }
  _this->platform->peripheral_gpio = _gpio;
  peripheral_gpio_direction_e _direction;
  if (_this->direction == kGpioDirectionIn) {
    _direction = PERIPHERAL_GPIO_DIRECTION_IN;
  } else {
    _direction = PERIPHERAL_GPIO_DIRECTION_OUT;
  }
  retVal = peripheral_gpio_set_direction(_gpio, _direction);
  if (retVal != PERIPHERAL_ERROR_NONE) {
    req_data->result = false;
    return;
  }
  // Mode is not supported by Peripheral API for Tizen
  peripheral_gpio_edge_e _edge;
  switch (_this->edge) {
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
    req_data->result = false;
    return;
  }
  req_data->result = true;
}
