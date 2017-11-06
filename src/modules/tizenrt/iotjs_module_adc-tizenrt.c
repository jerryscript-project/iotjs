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

#if defined(__TIZENRT__)

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <tinyara/analog/adc.h>
#include <tinyara/analog/ioctl.h>

#include "iotjs_def.h"
#include "modules/iotjs_module_adc.h"

#define S5J_ADC_MAX_CHANNELS 4

// There is one file for all ADC inputs so wee need one common file descriptor
static int32_t device_fd;
// This is simple ref counter. Each time ADC is opened, it is increased.
static size_t device_fd_counter = 0;
// Path of ADC device.
#define TIZENRT_ADC_DEVICE "/dev/adc0"

int32_t iotjs_adc_read(iotjs_adc_t* adc) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_adc_t, adc);
  int ret, nbytes;
  size_t readsize, i, nsamples;
  struct adc_msg_s samples[S5J_ADC_MAX_CHANNELS];
  ret = ioctl(_this->device_fd, ANIOC_TRIGGER, 0);
  if (ret < 0) {
    return -1;
  }
  readsize = sizeof(samples);
  while (true) {
    nbytes = read(_this->device_fd, samples, readsize);
    if (nbytes > 0) {
      nsamples = (size_t)nbytes / sizeof(struct adc_msg_s);
      int sample = -1;
      for (i = 0; i < nsamples; ++i) {
        if (_this->pin == samples[i].am_channel) {
          sample = samples[i].am_data;
        }
      }
      if (-1 != sample) {
        return sample;
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
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_adc_t, adc);
  if (_this->device_fd > 0) {
    device_fd_counter--;
  }
  if (0 == device_fd_counter) {
    close(_this->device_fd);
  }
  return true;
}


void iotjs_adc_open_worker(uv_work_t* work_req) {
  ADC_WORKER_INIT;
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_adc_t, adc);
  if (0 == device_fd_counter) {
    device_fd = open(TIZENRT_ADC_DEVICE, O_RDONLY);
  }
  _this->device_fd = device_fd;
  if (_this->device_fd < 0) {
    req_data->result = false;
    return;
  }
  device_fd_counter++;
  req_data->result = true;
}

#endif // __TIZENRT__
