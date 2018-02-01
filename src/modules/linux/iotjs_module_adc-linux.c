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

#if !defined(__linux__)
#error "Module __FILE__ is for Linux only"
#endif

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

struct iotjs_adc_platform_data_s {
  iotjs_string_t device;
};


void iotjs_adc_create_platform_data(iotjs_adc_t* adc) {
  adc->platform_data = IOTJS_ALLOC(iotjs_adc_platform_data_t);
}


void iotjs_adc_destroy_platform_data(iotjs_adc_platform_data_t* platform_data) {
  iotjs_string_destroy(&platform_data->device);
  IOTJS_RELEASE(platform_data);
}


jerry_value_t iotjs_adc_set_platform_config(iotjs_adc_t* adc,
                                            const jerry_value_t jconfig) {
  iotjs_adc_platform_data_t* platform_data = adc->platform_data;

  JS_GET_REQUIRED_CONF_VALUE(jconfig, platform_data->device,
                             IOTJS_MAGIC_STRING_DEVICE, string);

  return jerry_create_undefined();
}


// Implementation used here are based on:
//  https://www.kernel.org/doc/Documentation/adc/sysfs.txt

bool iotjs_adc_read(iotjs_adc_t* adc) {
  iotjs_adc_platform_data_t* platform_data = adc->platform_data;

  const char* device_path = iotjs_string_data(&platform_data->device);
  char buffer[ADC_VALUE_BUFFER_SIZE];

  if (!iotjs_systemio_open_read_close(device_path, buffer, sizeof(buffer))) {
    return false;
  }

  adc->value = atoi(buffer) != 0;

  return true;
}


bool iotjs_adc_close(iotjs_adc_t* adc) {
  return true;
}

bool iotjs_adc_open(iotjs_adc_t* adc) {
  iotjs_adc_platform_data_t* platform_data = adc->platform_data;

  DDDLOG("%s()", __func__);
  const char* device_path = iotjs_string_data(&platform_data->device);

  // Check if ADC interface exists.
  if (!iotjs_systemio_check_path(device_path)) {
    return false;
  }

  return true;
}
