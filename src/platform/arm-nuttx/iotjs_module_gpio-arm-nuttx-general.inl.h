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

#ifndef IOTJS_MODULE_GPIO_ARM_NUTTX_GENERAL_INL_H
#define IOTJS_MODULE_GPIO_ARM_NUTTX_GENERAL_INL_H

#include "module/iotjs_module_gpio.h"


void iotjs_gpio_open_worker(uv_work_t* work_req) {
  IOTJS_ASSERT(!"Not implemented");
}

void iotjs_gpio_write_worker(uv_work_t* work_req) {
  IOTJS_ASSERT(!"Not implemented");
}

void iotjs_gpio_read_worker(uv_work_t* work_req) {
  IOTJS_ASSERT(!"Not implemented");
}

void iotjs_gpio_close_worker(uv_work_t* work_req) {
  IOTJS_ASSERT(!"Not implemented");
}

bool iotjs_gpio_write(uv_work_t* work_req) {
  IOTJS_ASSERT(!"Not implemented");
  return false;
}

int iotjs_gpio_read(uv_work_t* work_req) {
  IOTJS_ASSERT(!"Not implemented");
  return -1;
}

#endif /* IOTJS_MODULE_GPIO_ARM_NUTTX_GENERAL_INL_H */
