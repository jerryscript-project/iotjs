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

struct _iotjs_gpio_module_platform_t {
  iotbus_gpio_context_h gpio_context;
};


void iotjs_gpio_platform_create(iotjs_gpio_t_impl_t* _this) {
  size_t private_mem = sizeof(struct _iotjs_gpio_module_platform_t);
  _this->platform = (iotjs_gpio_module_platform_t)malloc(private_mem);
}


void iotjs_gpio_platform_destroy(iotjs_gpio_t_impl_t* _this) {
  iotjs_buffer_release((char*)_this->platform);
}


void iotjs_gpio_open_worker(uv_work_t* work_req) {
  GPIO_WORKER_INIT;
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_gpio_t, gpio);

  DDDLOG("%s - pin: %d, direction: %d, mode: %d", __func__, _this->pin,
         _this->direction, _this->mode);

  iotbus_gpio_context_h gpio_context = iotbus_gpio_open((int)_this->pin);
  if (gpio_context == NULL) {
    req_data->result = false;
    return;
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
    req_data->result = false;
    return;
  }

  _this->platform->gpio_context = gpio_context;

  req_data->result = true;
}


bool iotjs_gpio_write(iotjs_gpio_t* gpio, bool value) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_gpio_t, gpio);
  if (iotbus_gpio_write(_this->platform->gpio_context, value) < 0) {
    return false;
  }
  return true;
}


int iotjs_gpio_read(iotjs_gpio_t* gpio) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_gpio_t, gpio);
  return iotbus_gpio_read(_this->platform->gpio_context);
}


bool iotjs_gpio_close(iotjs_gpio_t* gpio) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_gpio_t, gpio);
  if (iotbus_gpio_close(_this->platform->gpio_context) < 0) {
    return false;
  }
  return true;
}
