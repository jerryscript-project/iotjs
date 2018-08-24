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

#ifndef IOTJS_MODULE_PERIPH_COMMON_H
#define IOTJS_MODULE_PERIPH_COMMON_H

#include "iotjs_def.h"

typedef enum {
  kAdcOpOpen,
  kAdcOpRead,
  kAdcOpClose,
  kGpioOpOpen,
  kGpioOpWrite,
  kGpioOpRead,
  kGpioOpClose,
  kI2cOpOpen,
  kI2cOpClose,
  kI2cOpWrite,
  kI2cOpRead,
  kPwmOpClose,
  kPwmOpOpen,
  kPwmOpSetDutyCycle,
  kPwmOpSetEnable,
  kPwmOpSetFrequency,
  kPwmOpSetPeriod,
  kSpiOpClose,
  kSpiOpOpen,
  kSpiOpTransferArray,
  kSpiOpTransferBuffer,
  kUartOpClose,
  kUartOpOpen,
  kUartOpWrite
} iotjs_periph_op_t;

typedef struct {
  uint8_t op;
  bool result;
  void* data;
} iotjs_periph_data_t;

const char* iotjs_periph_error_str(uint8_t op);
void iotjs_periph_call_async(void* type_p, jerry_value_t jcallback, uint8_t op,
                             uv_work_cb worker);

#define IOTJS_DEFINE_PERIPH_CREATE_FUNCTION(name)                             \
  static iotjs_##name##_t* name##_create(const jerry_value_t jobject) {       \
    iotjs_##name##_t* data = IOTJS_ALLOC(iotjs_##name##_t);                   \
    iotjs_##name##_create_platform_data(data);                                \
    data->jobject = jobject;                                                  \
    jerry_set_object_native_pointer(jobject, data, &this_module_native_info); \
    return data;                                                              \
  }

#endif /* IOTJS_MODULE_PERIPH_COMMON_H */
