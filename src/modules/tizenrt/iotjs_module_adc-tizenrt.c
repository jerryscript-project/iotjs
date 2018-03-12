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

#if !defined(__TIZENRT__)
#error "Module __FILE__ is for TizenRT only"
#endif

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <tinyara/analog/adc.h>
#include <tinyara/analog/ioctl.h>

#include "iotjs_def.h"
#include "modules/iotjs_module_adc.h"

#define S5J_ADC_MAX_CHANNELS 4

struct iotjs_adc_platform_data_s {
  int32_t device_fd;
  uint32_t pin;
};

// There is one file for all ADC inputs so wee need one common file descriptor
static int32_t device_fd;
// This is simple ref counter. Each time ADC is opened, it is increased.
static size_t device_fd_counter = 0;
// Path of ADC device.
#define TIZENRT_ADC_DEVICE "/dev/adc0"

void iotjs_adc_create_platform_data(iotjs_adc_t* adc) {
  adc->platform_data = IOTJS_ALLOC(iotjs_adc_platform_data_t);
  adc->platform_data->device_fd = -1;
  adc->platform_data->pin = 0;
}


void iotjs_adc_destroy_platform_data(iotjs_adc_platform_data_t* platform_data) {
  IOTJS_RELEASE(platform_data);
}


jerry_value_t iotjs_adc_set_platform_config(iotjs_adc_t* adc,
                                            const jerry_value_t jconfig) {
  iotjs_adc_platform_data_t* platform_data = adc->platform_data;

  JS_GET_REQUIRED_CONF_VALUE(jconfig, platform_data->pin,
                             IOTJS_MAGIC_STRING_PIN, number);

  return jerry_create_undefined();
}


bool iotjs_adc_read(iotjs_adc_t* adc) {
  iotjs_adc_platform_data_t* platform_data = adc->platform_data;

  int ret, nbytes;
  size_t readsize, i, nsamples;
  struct adc_msg_s samples[S5J_ADC_MAX_CHANNELS];
  ret = ioctl(platform_data->device_fd, ANIOC_TRIGGER, 0);

  if (ret < 0) {
    adc->value = -1;
    return false;
  }

  readsize = sizeof(samples);
  while (true) {
    nbytes = read(platform_data->device_fd, samples, readsize);
    if (nbytes > 0) {
      nsamples = (size_t)nbytes / sizeof(struct adc_msg_s);
      int sample = -1;
      for (i = 0; i < nsamples; ++i) {
        if (platform_data->pin == samples[i].am_channel) {
          sample = samples[i].am_data;
        }
      }
      if (sample != -1) {
        adc->value = sample;
        return true;
      }
    } /* else {
      // The read function is blocking but there are events,
      // which can interrupt it. The function will return
      // non-positive number of obtained samples. We can ignore it and
      // wait for next samples returned from ADC.
    } */
  }
}


bool iotjs_adc_close(iotjs_adc_t* adc) {
  iotjs_adc_platform_data_t* platform_data = adc->platform_data;

  if (platform_data->device_fd > 0) {
    device_fd_counter--;
  }

  if (device_fd_counter == 0) {
    close(platform_data->device_fd);
  }

  return true;
}


bool iotjs_adc_open(iotjs_adc_t* adc) {
  iotjs_adc_platform_data_t* platform_data = adc->platform_data;

  if (device_fd_counter == 0) {
    device_fd = open(TIZENRT_ADC_DEVICE, O_RDONLY);
  }

  platform_data->device_fd = device_fd;
  if (platform_data->device_fd < 0) {
    return false;
  }

  device_fd_counter++;

  return true;
}
