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
#include "module/iotjs_module_adc.h"


#define ADC_INTERFACE "/sys/devices/12d10000.adc/iio:device0/"
#define ADC_PIN_INTERFACE "in_voltage%d_raw"
#define ADC_PIN_FORMAT ADC_INTERFACE ADC_PIN_INTERFACE

#define ADC_PATH_BUFFER_SIZE DEVICE_IO_PATH_BUFFER_SIZE
#define ADC_PIN_BUFFER_SIZE DEVICE_IO_PIN_BUFFER_SIZE
#define ADC_VALUE_BUFFER_SIZE 64


// Implementation used here are based on:
//  https://www.kernel.org/doc/Documentation/adc/sysfs.txt


void iotjs_adc_export_worker(uv_work_t* work_req) {
  iotjs_adc_reqwrap_t* req_wrap = iotjs_adc_reqwrap_from_request(work_req);
  iotjs_adc_reqdata_t* req_data = iotjs_adc_reqwrap_data(req_wrap);
  DDDLOG("ADC %s()", __func__);

  // Check if ADC interface exits.
  if (iotjs_systemio_check_path(ADC_INTERFACE)) {
    req_data->result = kAdcErrOk;
  } else {
    req_data->result = kAdcErrExport;
  }
}

void iotjs_adc_read_worker(uv_work_t* work_req) {
  iotjs_adc_reqwrap_t* req_wrap = iotjs_adc_reqwrap_from_request(work_req);
  iotjs_adc_reqdata_t* req_data = iotjs_adc_reqwrap_data(req_wrap);
  uint32_t pin = req_data->pin;
  DDDLOG("adc iotjs_adc_read_worker() - pin: %d", pin);

  char value_path[ADC_PATH_BUFFER_SIZE] = { 0 };
  snprintf(value_path, sizeof(value_path), ADC_PIN_FORMAT, pin);

  char buffer[ADC_VALUE_BUFFER_SIZE];
  if (iotjs_systemio_open_read_close(value_path, buffer, sizeof(buffer))) {
    req_data->result = kAdcErrOk;
    req_data->value = atoi(buffer);
  } else {
    req_data->result = kAdcErrRead;
  }
}

int32_t iotjs_adc_read_sync(int32_t pin) {
  DDDLOG("adc iotjs_adc_read_sync() - pin: %d", pin);
  char path[ADC_PATH_BUFFER_SIZE];
  snprintf(path, sizeof(path), ADC_PIN_FORMAT, pin);
  FILE* file = fopen(path, "r");
  if (NULL == file) {
    return -1;
  }
  int32_t value;
  int count = fscanf(file, "%d", &value);
  fclose(file);

  if (1 == count) {
    return value;
  }
  return -1;
}


void iotjs_adc_unexport_worker(uv_work_t* work_req) {
  iotjs_adc_reqwrap_t* req_wrap = iotjs_adc_reqwrap_from_request(work_req);
  iotjs_adc_reqdata_t* req_data = iotjs_adc_reqwrap_data(req_wrap);
  DDDLOG("ADC %s()", __func__);
  req_data->result = kAdcErrOk;
}


#endif /* IOTJS_MODULE_ADC_LINUX_GENERAL_INL_H */
