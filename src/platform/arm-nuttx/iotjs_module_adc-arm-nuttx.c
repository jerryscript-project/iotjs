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

#if defined(__NUTTX__)


#include <uv.h>
#include <nuttx/analog/adc.h>
#include <stdlib.h>

#include "iotjs_def.h"
#include "iotjs_systemio-arm-nuttx.h"
#include "module/iotjs_module_adc.h"
#include "module/iotjs_module_stm32f4dis.h"


#define ADC_DEVICE_PATH_FORMAT "/dev/adc%d"
#define ADC_DEVICE_PATH_BUFFER_SIZE 12


static void iotjs_adc_get_path(char* buffer, int32_t number) {
  // Create ADC device path
  snprintf(buffer, ADC_DEVICE_PATH_BUFFER_SIZE - 1, ADC_DEVICE_PATH_FORMAT,
           number);
}


#define ADC_WORKER_INIT_TEMPLATE                                            \
  iotjs_adc_reqwrap_t* req_wrap = iotjs_adc_reqwrap_from_request(work_req); \
  iotjs_adc_reqdata_t* req_data = iotjs_adc_reqwrap_data(req_wrap);


static bool iotjs_adc_read_data(int32_t pin, struct adc_msg_s* msg) {
  int32_t adc_number = ADC_GET_NUMBER(pin);
  char path[ADC_DEVICE_PATH_BUFFER_SIZE] = { 0 };
  iotjs_adc_get_path(path, adc_number);

  const iotjs_environment_t* env = iotjs_environment_get();
  uv_loop_t* loop = iotjs_environment_loop(env);
  int result, close_result;

  // Open file
  uv_fs_t open_req;
  result = uv_fs_open(loop, &open_req, path, O_RDONLY, 0666, NULL);
  uv_fs_req_cleanup(&open_req);
  if (result < 0) {
    return false;
  }

  // Read value
  uv_fs_t read_req;
  uv_buf_t uvbuf = uv_buf_init((char*)msg, sizeof(*msg));
  result = uv_fs_read(loop, &read_req, open_req.result, &uvbuf, 1, 0, NULL);
  uv_fs_req_cleanup(&read_req);

  // Close file
  uv_fs_t close_req;
  close_result = uv_fs_close(loop, &close_req, open_req.result, NULL);
  uv_fs_req_cleanup(&close_req);
  if (result < 0 || close_result < 0) {
    return false;
  }

  DDLOG("ADC Read - path: %s, value: %d", path, msg->am_data);

  return true;
}


void iotjs_adc_export_worker(uv_work_t* work_req) {
  ADC_WORKER_INIT_TEMPLATE;

  int32_t pin = req_data->pin;
  int32_t adc_number = ADC_GET_NUMBER(pin);
  int32_t timer = SYSIO_GET_TIMER(pin);
  struct adc_dev_s* adc = iotjs_adc_config_nuttx(adc_number, timer, pin);

  char path[ADC_DEVICE_PATH_BUFFER_SIZE] = { 0 };
  iotjs_adc_get_path(path, adc_number);

  if (adc_register(path, adc) != 0) {
    req_data->result = kAdcErrExport;
    return;
  }

  DDLOG("ADC Export - path: %s, number: %d, timer: %d", path, adc_number,
        timer);

  req_data->result = kAdcErrOk;
}


void iotjs_adc_read_worker(uv_work_t* work_req) {
  ADC_WORKER_INIT_TEMPLATE;

  struct adc_msg_s msg;

  if (!iotjs_adc_read_data(req_data->pin, &msg)) {
    req_data->result = kAdcErrRead;
    return;
  }

  req_data->value = msg.am_data;
  req_data->result = kAdcErrOk;
}


int32_t iotjs_adc_read_sync(int32_t pin) {
  struct adc_msg_s msg;

  if (!iotjs_adc_read_data(pin, &msg)) {
    return -1;
  }

  return msg.am_data;
}


void iotjs_adc_unexport_worker(uv_work_t* work_req) {
  ADC_WORKER_INIT_TEMPLATE;

  int32_t pin = req_data->pin;
  int32_t adc_number = ADC_GET_NUMBER(pin);

  char path[ADC_DEVICE_PATH_BUFFER_SIZE] = { 0 };
  iotjs_adc_get_path(path, adc_number);

  // Release driver
  if (unregister_driver(path) < 0) {
    req_data->result = kAdcErrUnexport;
    return;
  }

  iotjs_gpio_unconfig_nuttx(pin);

  req_data->result = kAdcErrOk;
}


#endif // __NUTTX__
