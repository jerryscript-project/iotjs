/* Copyright 2016 Samsung Electronics Co., Ltd.
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

#ifndef IOTJS_MODULE_PWM_LINUX_GENERAL_INL_H
#define IOTJS_MODULE_PWM_LINUX_GENERAL_INL_H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "iotjs_systemio-linux.h"
#include "module/iotjs_module_pwm.h"


// Generic PWM implementation for linux.


#define PWM_INTERFACE "/sys/class/pwm/pwmchip%d/"
#define PWM_PIN_INTERFACE "pwm%d/"
#define PWM_PIN_FORMAT PWM_INTERFACE PWM_PIN_INTERFACE
#define PWM_EXPORT PWM_INTERFACE "export"
#define PWM_UNEXPORT PWM_INTERFACE "unexport"
#define PWM_PIN_DUTYCYCLE "duty_cycle"
#define PWM_PIN_PERIOD "period"
#define PWM_PIN_ENABlE "enable"
#define PWM_DEFAULT_CHIP_NUMBER 0


// Set PWM period.
bool SetPwmPeriod(iotjs_pwmreqdata_t* req_data) {
  IOTJS_ASSERT(!iotjs_string_is_empty(&req_data->device));

  char path_buff[64] = { 0 };
  char value_buff[32] = { 0 };

  strcat(path_buff, iotjs_string_data(&req_data->device));
  strcat(path_buff, PWM_PIN_PERIOD);

  DDDLOG("PWM SetPeriod - path: %s, value: %d", path_buff, req_data->period);

  snprintf(value_buff, 31, "%d", req_data->period);

  if (!DeviceOpenWriteClose(path_buff, value_buff)) {
    return false;
  }

  return true;
}


// Set PWM Duty-Cycle.
bool SetPwmDutyCycle(iotjs_pwmreqdata_t* req_data) {
  IOTJS_ASSERT(!iotjs_string_is_empty(&req_data->device));

  char path_buff[64] = { 0 };
  char value_buff[32] = { 0 };

  strcat(path_buff, iotjs_string_data(&req_data->device));
  strcat(path_buff, PWM_PIN_DUTYCYCLE);

  DDDLOG("PWM SetdutyCycle - path: %s, value: %d", path_buff,
         req_data->duty_cycle);

  snprintf(value_buff, 31, "%d", req_data->duty_cycle);

  if (!DeviceOpenWriteClose(path_buff, value_buff)) {
    return false;
  }

  return true;
}


#define PWM_WORKER_INIT_TEMPLATE                                          \
  iotjs_pwmreqwrap_t* req_wrap = iotjs_pwmreqwrap_from_request(work_req); \
  iotjs_pwmreqdata_t* req_data = iotjs_pwmreqwrap_data(req_wrap);


int PwmInitializePwmPath(iotjs_pwmreqdata_t* req_data) {
  int32_t chip_number, pwm_number;
  const char* path = iotjs_string_data(&req_data->device);
  char buffer[64] = { 0 };

  if (sscanf(path, PWM_PIN_FORMAT, &chip_number, &pwm_number) == 2) {
  } else if (sscanf(path, "%d", &pwm_number) == 1) {
    chip_number = PWM_DEFAULT_CHIP_NUMBER;
  } else {
    return -1;
  }

  // Create Device Path
  iotjs_string_make_empty(&req_data->device);
  snprintf(buffer, 63, PWM_PIN_FORMAT, chip_number, pwm_number);
  iotjs_string_append(&req_data->device, buffer, -1);

  return 0;
}


void ExportWorker(uv_work_t* work_req) {
  PWM_WORKER_INIT_TEMPLATE;

  IOTJS_ASSERT(!iotjs_string_is_empty(&req_data->device));

  const char* path = iotjs_string_data(&req_data->device);
  const char* export_path;
  int32_t chip_number, pwm_number;

  // See if the pwm is already opened.
  if (!DeviceCheckPath(path)) {
    // Get chip_number and pwm_number.
    if (sscanf(path, PWM_PIN_FORMAT, &chip_number, &pwm_number) != 2) {
      req_data->result = kPwmErrExport;
      return;
    }

    // Write export pwm
    char buffer[64] = { 0 };
    snprintf(buffer, 63, PWM_EXPORT, chip_number);

    const char* created_files[] = { PWM_PIN_DUTYCYCLE, PWM_PIN_PERIOD,
                                    PWM_PIN_ENABlE };
    int created_files_length = sizeof(created_files) / sizeof(created_files[0]);
    if (!DeviceExport(buffer, pwm_number, path, created_files,
                      created_files_length)) {
      req_data->result = kPwmErrExport;
      return;
    }
  }

  // Set options.
  if (req_data->period >= 0) {
    if (!SetPwmPeriod(req_data)) {
      req_data->result = kPwmErrWrite;
      return;
    }
  }

  if (req_data->duty_cycle >= 0) {
    if (!SetPwmDutyCycle(req_data)) {
      req_data->result = kPwmErrWrite;
      return;
    }
  }

  DDDLOG("PWM ExportWorker - path: %s", path);

  req_data->result = kPwmErrOk;
}


void SetPeriodWorker(uv_work_t* work_req) {
  PWM_WORKER_INIT_TEMPLATE;

  if (!SetPwmPeriod(req_data)) {
    req_data->result = kPwmErrWrite;
    return;
  }

  DDDLOG("PWM SetPeriodWorker");

  req_data->result = kPwmErrOk;
}


void SetDutyCycleWorker(uv_work_t* work_req) {
  PWM_WORKER_INIT_TEMPLATE;

  if (!SetPwmDutyCycle(req_data)) {
    req_data->result = kPwmErrWrite;
    return;
  }

  DDDLOG("PWM SetDutyCycleWorker");

  req_data->result = kPwmErrOk;
}


void SetEnableWorker(uv_work_t* work_req) {
  PWM_WORKER_INIT_TEMPLATE;

  IOTJS_ASSERT(!iotjs_string_is_empty(&req_data->device));

  iotjs_string_append(&req_data->device, PWM_PIN_ENABlE, -1);
  const char* path = iotjs_string_data(&req_data->device);

  char buff[10] = { 0 };
  snprintf(buff, 9, "%d", req_data->enable);
  if (!DeviceOpenWriteClose(path, buff)) {
    req_data->result = kPwmErrWrite;
    return;
  }

  DDDLOG("PWM SetEnableWorker - path: %s", path);

  req_data->result = kPwmErrOk;
}


void UnexportWorker(uv_work_t* work_req) {
  PWM_WORKER_INIT_TEMPLATE;

  IOTJS_ASSERT(!iotjs_string_is_empty(&req_data->device));

  const char* path = iotjs_string_data(&req_data->device);
  const char* export_path;
  int32_t chip_number, pwm_number;

  if (DeviceCheckPath(path)) {
    sscanf(path, PWM_PIN_FORMAT, &chip_number, &pwm_number);
    // Write export pin
    char buffer[64] = { 0 };
    snprintf(buffer, 63, PWM_UNEXPORT, chip_number);

    DeviceUnexport(buffer, pwm_number);
  }

  DDDLOG("Pwm Unexport - path: %s", path);

  req_data->result = kPwmErrOk;
}


#endif /* IOTJS_MODULE_PWM_LINUX_GENERAL_INL_H */
