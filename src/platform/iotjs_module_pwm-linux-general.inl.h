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

#ifndef IOTJS_MODULE_PWM_LINUX_GENERAL_INL_H
#define IOTJS_MODULE_PWM_LINUX_GENERAL_INL_H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
#define PWM_PATH_BUFFER_SIZE 64
#define PWM_VALUE_BUFFER_SIZE 32


void iotjs_pwm_initialize() {
  DDLOG("PWM initialize");
}


// Set PWM period.
bool SetPwmPeriod(iotjs_pwm_reqdata_t* req_data) {
  char path[PWM_PATH_BUFFER_SIZE] = { 0 };
  if (snprintf(path, PWM_PATH_BUFFER_SIZE - 1, PWM_PIN_FORMAT,
               PWM_DEFAULT_CHIP_NUMBER, req_data->pin) < 0) {
    return false;
  }

  strcat(path, PWM_PIN_PERIOD);

  DDLOG("PWM SetPeriod - path: %s, value: %d", path, req_data->period);

  char value_buff[PWM_VALUE_BUFFER_SIZE] = { 0 };
  snprintf(value_buff, PWM_VALUE_BUFFER_SIZE - 1, "%d", req_data->period);

  if (!DeviceOpenWriteClose(path, value_buff)) {
    return false;
  }

  return true;
}


// Set PWM Duty-Cycle.
bool SetPwmDutyCycle(iotjs_pwm_reqdata_t* req_data) {
  char path[PWM_PATH_BUFFER_SIZE] = { 0 };
  if (snprintf(path, PWM_PATH_BUFFER_SIZE - 1, PWM_PIN_FORMAT,
               PWM_DEFAULT_CHIP_NUMBER, req_data->pin) < 0) {
    return false;
  }

  strcat(path, PWM_PIN_DUTYCYCLE);
  uint32_t duty_cycle = req_data->duty_cycle * req_data->period * 0.01;

  DDLOG("PWM SetdutyCycle - path: %s, value: %d\n", path, duty_cycle);

  char value_buff[PWM_VALUE_BUFFER_SIZE] = { 0 };
  snprintf(value_buff, PWM_VALUE_BUFFER_SIZE - 1, "%d", duty_cycle);

  if (!DeviceOpenWriteClose(path, value_buff)) {
    return false;
  }

  return true;
}


#define PWM_WORKER_INIT_TEMPLATE                                            \
  iotjs_pwm_reqwrap_t* req_wrap = iotjs_pwm_reqwrap_from_request(work_req); \
  iotjs_pwm_reqdata_t* req_data = iotjs_pwm_reqwrap_data(req_wrap);


void ExportWorker(uv_work_t* work_req) {
  PWM_WORKER_INIT_TEMPLATE;

  char path[PWM_PATH_BUFFER_SIZE] = { 0 };
  if (snprintf(path, PWM_PATH_BUFFER_SIZE - 1, PWM_PIN_FORMAT,
               PWM_DEFAULT_CHIP_NUMBER, req_data->pin) < 0) {
    req_data->result = kPwmErrSys;
    return;
  }


  // See if the PWM is already opened.
  if (!DeviceCheckPath(path)) {
    // Write exporting PWM path
    char export_path[PWM_PATH_BUFFER_SIZE] = { 0 };
    snprintf(export_path, PWM_PATH_BUFFER_SIZE - 1, PWM_EXPORT,
             PWM_DEFAULT_CHIP_NUMBER);

    const char* created_files[] = { PWM_PIN_DUTYCYCLE, PWM_PIN_PERIOD,
                                    PWM_PIN_ENABlE };
    int created_files_length = sizeof(created_files) / sizeof(created_files[0]);
    if (!DeviceExport(export_path, req_data->pin, path, created_files,
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


void SetFrequencyWorker(uv_work_t* work_req) {
  IOTJS_ASSERT(!"Not implemented");
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

  char path[PWM_PATH_BUFFER_SIZE] = { 0 };
  if (snprintf(path, PWM_PATH_BUFFER_SIZE - 1, PWM_PIN_FORMAT,
               PWM_DEFAULT_CHIP_NUMBER, req_data->pin) < 0) {
    return;
  }

  strcat(path, PWM_PIN_ENABlE);

  char value[2] = { 0 };
  snprintf(value, 1, "%d", req_data->enable);
  if (!DeviceOpenWriteClose(path, value)) {
    req_data->result = kPwmErrWrite;
    return;
  }

  DDDLOG("PWM SetEnableWorker - path: %s", path);

  req_data->result = kPwmErrOk;
}


void UnexportWorker(uv_work_t* work_req) {
  PWM_WORKER_INIT_TEMPLATE;

  char path[PWM_PATH_BUFFER_SIZE] = { 0 };
  if (snprintf(path, PWM_PATH_BUFFER_SIZE - 1, PWM_PIN_FORMAT,
               PWM_DEFAULT_CHIP_NUMBER, req_data->pin) < 0) {
    return;
  }

  const char* export_path;
  int32_t chip_number = PWM_DEFAULT_CHIP_NUMBER;

  if (DeviceCheckPath(path)) {
    // Write exporting pin path
    char unexport_path[PWM_PATH_BUFFER_SIZE] = { 0 };
    snprintf(unexport_path, PWM_PATH_BUFFER_SIZE - 1, PWM_UNEXPORT,
             chip_number);

    DeviceUnexport(unexport_path, req_data->pin);
  }

  DDDLOG("Pwm Unexport - path: %s", path);

  req_data->result = kPwmErrOk;
}


#endif /* IOTJS_MODULE_PWM_LINUX_GENERAL_INL_H */
