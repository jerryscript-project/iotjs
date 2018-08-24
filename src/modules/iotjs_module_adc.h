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


#ifndef IOTJS_MODULE_ADC_H
#define IOTJS_MODULE_ADC_H

#include "iotjs_def.h"
#include "iotjs_module_periph_common.h"

// Forward declaration of platform data. These are only used by platform code.
// Generic ADC module never dereferences platform data pointer.
typedef struct iotjs_adc_platform_data_s iotjs_adc_platform_data_t;

typedef struct {
  jerry_value_t jobject;
  iotjs_adc_platform_data_t* platform_data;
  int32_t value;
} iotjs_adc_t;

bool iotjs_adc_read(iotjs_adc_t* adc);
bool iotjs_adc_close(iotjs_adc_t* adc);
bool iotjs_adc_open(iotjs_adc_t* adc);

// Platform-related functions; they are implemented
// by platform code (i.e.: linux, nuttx, tizen).
void iotjs_adc_create_platform_data(iotjs_adc_t* adc);
void iotjs_adc_destroy_platform_data(iotjs_adc_platform_data_t* platform_data);
jerry_value_t iotjs_adc_set_platform_config(iotjs_adc_t* adc,
                                            const jerry_value_t jconfig);

#endif /* IOTJS_MODULE_ADC_H */
