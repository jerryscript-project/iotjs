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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "iotjs_systemio-linux.h"
#include "modules/iotjs_module_adc.h"


#define ADC_PIN_FORMAT ADC_INTERFACE ADC_PIN_INTERFACE

#define ADC_PATH_BUFFER_SIZE DEVICE_IO_PATH_BUFFER_SIZE
#define ADC_PIN_BUFFER_SIZE DEVICE_IO_PIN_BUFFER_SIZE
#define ADC_VALUE_BUFFER_SIZE 64


// Implementation used here are based on:
//  https://www.kernel.org/doc/Documentation/adc/sysfs.txt

int32_t iotjs_adc_read(iotjs_adc_t* adc) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_adc_t, adc);

  const char* device_path = iotjs_string_data(&_this->device);
  char buffer[ADC_VALUE_BUFFER_SIZE];

  if (!iotjs_systemio_open_read_close(device_path, buffer, sizeof(buffer))) {
    return -1;
  }

  return atoi(buffer);
}


bool iotjs_adc_close(iotjs_adc_t* adc) {
  return true;
}

void iotjs_adc_open_worker(uv_work_t* work_req) {
  ADC_WORKER_INIT;
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_adc_t, adc);

  DDDLOG("%s()", __func__);
  const char* device_path = iotjs_string_data(&_this->device);

  // Check if ADC interface exists.
  req_data->result = iotjs_systemio_check_path(device_path);
}


#endif /* IOTJS_MODULE_ADC_LINUX_GENERAL_INL_H */
