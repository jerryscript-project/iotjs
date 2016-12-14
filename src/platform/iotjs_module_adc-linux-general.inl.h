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

#ifndef IOTJS_MODULE_ADC_LINUX_GENERAL_INL_H
#define IOTJS_MODULE_ADC_LINUX_GENERAL_INL_H


#include "module/iotjs_module_adc.h"


void iotjs_adc_export_worker(uv_work_t* work_req) {
  IOTJS_ASSERT(!"Not implemented");
}


void iotjs_adc_read_worker(uv_work_t* work_req) {
  IOTJS_ASSERT(!"Not implemented");
}


int32_t iotjs_adc_read_sync(int32_t pin) {
  IOTJS_ASSERT(!"Not implemented");
  return -1;
}


void iotjs_adc_unexport_worker(uv_work_t* work_req) {
  IOTJS_ASSERT(!"Not implemented");
}


#endif /* IOTJS_MODULE_ADC_LINUX_GENERAL_INL_H */
