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


#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "iotjs_systemio-linux.h"
#include "module/iotjs_module_pwm.h"


#define PWM_INTERFACE "/sys/class/pwm/pwmchip%d/"
#define PWM_PIN_INTERFACE "pwm%d/"
#define PWM_PIN_FORMAT PWM_INTERFACE PWM_PIN_INTERFACE
#define PWM_EXPORT PWM_INTERFACE "export"
#define PWM_UNEXPORT PWM_INTERFACE "unexport"
#define PWM_PIN_DUTYCYCLE "duty_cycle"
#define PWM_PIN_PERIOD "period"
#define PWM_PIN_ENABlE "enable"
#define PWM_DEFAULT_CHIP_NUMBER 0


namespace iotjs {


// Generic PWM implementation for linux.
class PwmLinuxGeneral : public Pwm {
 public:
  explicit PwmLinuxGeneral(const iotjs_jval_t* jpwm);

  static PwmLinuxGeneral* GetInstance();

  virtual int InitializePwmPath(PwmReqWrap* req_wrap);
  virtual int Export(PwmReqWrap* req_wrap);
  virtual int SetPeriod(PwmReqWrap* req_wrap);
  virtual int SetDutyCycle(PwmReqWrap* req_wrap);
  virtual int SetEnable(PwmReqWrap* req_wrap);
  virtual int Unexport(PwmReqWrap* req_wrap);

 public:
};


PwmLinuxGeneral::PwmLinuxGeneral(const iotjs_jval_t* jpwm)
    : Pwm(jpwm)
{
}


PwmLinuxGeneral* PwmLinuxGeneral::GetInstance() {
  return static_cast<PwmLinuxGeneral*>(Pwm::GetInstance());
}


// Set PWM period.
bool SetPwmPeriod(PwmReqWrap* req_wrap) {
  IOTJS_ASSERT(!iotjs_string_is_empty(&req_wrap->device));

  char path_buff[64] = {0};
  char value_buff[32] = {0};

  strcat(path_buff, iotjs_string_data(&req_wrap->device));
  strcat(path_buff, PWM_PIN_PERIOD);

  DDDLOG("PWM SetPeriod - path: %s, value: %d", path_buff, req_wrap->period);

  snprintf(value_buff, 31, "%d", req_wrap->period);

  if (!DeviceOpenWriteClose(path_buff, value_buff)) {
    return false;
  }

  return true;
}


// Set PWM Duty-Cycle.
bool SetPwmDutyCycle(PwmReqWrap* req_wrap) {
  IOTJS_ASSERT(!iotjs_string_is_empty(&req_wrap->device));

  char path_buff[64] = {0};
  char value_buff[32] = {0};

  strcat(path_buff, iotjs_string_data(&req_wrap->device));
  strcat(path_buff, PWM_PIN_DUTYCYCLE);

  DDDLOG("PWM SetdutyCycle - path: %s, value: %d",
         path_buff, req_wrap->duty_cycle);

  snprintf(value_buff, 31, "%d", req_wrap->duty_cycle);

  if (!DeviceOpenWriteClose(path_buff, value_buff)) {
    return false;
  }

  return true;
}


void AfterPwmWork(uv_work_t* work_req, int status) {
  PwmLinuxGeneral* pwm = PwmLinuxGeneral::GetInstance();
  PwmReqWrap* req_wrap = reinterpret_cast<PwmReqWrap*>(work_req->data);

  if (status) {
    req_wrap->result = kPwmErrSys;
  }

  iotjs_jargs_t jargs = iotjs_jargs_create(1);
  iotjs_jargs_append_number(&jargs, req_wrap->result);

  switch (req_wrap->op) {
    case kPwmOpExport:
    case kPwmOpSetDutyCycle:
    case kPwmOpSetPeriod:
    case kPwmOpSetEnable:
    case kPwmOpUnexport:
      break;
    default:
    {
      IOTJS_ASSERT(!"Unreachable");
      break;
    }
  }

  iotjs_make_callback(req_wrap->jcallback(), Pwm::GetJPwm(), &jargs);

  iotjs_jargs_destroy(&jargs);

  delete req_wrap;
}


void ExportWorker(uv_work_t* work_req) {
  PwmReqWrap* req_wrap = reinterpret_cast<PwmReqWrap*>(work_req->data);

  IOTJS_ASSERT(!iotjs_string_is_empty(&req_wrap->device));

  const char* path = iotjs_string_data(&req_wrap->device);
  const char* export_path;
  int32_t chip_number, pwm_number;

  // See if the pwm is already opened.
  if (!DeviceCheckPath(path)) {
    // Get chip_number and pwm_number.
    if (sscanf(path, PWM_PIN_FORMAT, &chip_number, &pwm_number) != 2) {
      req_wrap->result = kPwmErrExport;
      return;
    }

    // Write export pwm
    char buffer[64] = {0};
    snprintf(buffer, 63, PWM_EXPORT, chip_number);

    const char* created_files[] = {PWM_PIN_DUTYCYCLE, PWM_PIN_PERIOD,
                                   PWM_PIN_ENABlE};
    int created_files_length = sizeof(created_files) / sizeof(created_files[0]);
    if (!DeviceExport(buffer, pwm_number, path, created_files,
                      created_files_length)) {
      req_wrap->result = kPwmErrExport;
      return;
    }
  }

  // Set options.
  if (req_wrap->period >= 0) {
    if (!SetPwmPeriod(req_wrap)) {
      req_wrap->result = kPwmErrWrite;
      return;
    }
  }

  if (req_wrap->duty_cycle >= 0) {
    if (!SetPwmDutyCycle(req_wrap)) {
      req_wrap->result = kPwmErrWrite;
      return;
    }
  }

  DDDLOG("PWM ExportWorker - path: %s", path);

  req_wrap->result = kPwmErrOk;
}


void SetPeriodWorker(uv_work_t* work_req) {
  PwmReqWrap* req_wrap = reinterpret_cast<PwmReqWrap*>(work_req->data);

  if (!SetPwmPeriod(req_wrap)) {
    req_wrap->result = kPwmErrWrite;
    return;
  }

  DDDLOG("PWM SetPeriodWorker");

  req_wrap->result = kPwmErrOk;
}


void SetDutyCycleWorker(uv_work_t* work_req) {
  PwmReqWrap* req_wrap = reinterpret_cast<PwmReqWrap*>(work_req->data);

  if (!SetPwmDutyCycle(req_wrap)) {
    req_wrap->result = kPwmErrWrite;
    return;
  }

  DDDLOG("PWM SetDutyCycleWorker");

  req_wrap->result = kPwmErrOk;
}


void SetEnableWorker(uv_work_t* work_req) {
  PwmReqWrap* req_wrap = reinterpret_cast<PwmReqWrap*>(work_req->data);

  IOTJS_ASSERT(!iotjs_string_is_empty(&req_wrap->device));

  iotjs_string_append(&req_wrap->device, PWM_PIN_ENABlE, -1);
  const char* path = iotjs_string_data(&req_wrap->device);

  char buff[10] = {0};
  snprintf(buff, 9, "%d", req_wrap->enable);
  if (!DeviceOpenWriteClose(path, buff)) {
    req_wrap->result = kPwmErrWrite;
    return;
  }

  DDDLOG("PWM SetEnableWorker - path: %s", path);

  req_wrap->result = kPwmErrOk;
}


void UnexportWorker(uv_work_t* work_req) {
  PwmReqWrap* req_wrap = reinterpret_cast<PwmReqWrap*>(work_req->data);

  IOTJS_ASSERT(!iotjs_string_is_empty(&req_wrap->device));

  const char* path = iotjs_string_data(&req_wrap->device);
  const char* export_path;
  int32_t chip_number, pwm_number;

  if (DeviceCheckPath(path)) {
    sscanf(path, PWM_PIN_FORMAT, &chip_number, &pwm_number);
    // Write export pin
    char buffer[64] = {0};
    snprintf(buffer, 63, PWM_UNEXPORT, chip_number);

    DeviceUnexport(buffer, pwm_number);
  }

  DDDLOG("Pwm Unexport - path: %s", path);

  req_wrap->result = kPwmErrOk;
}


#define PWM_LINUX_GENERAL_IMPL_TEMPLATE(op) \
  do { \
    PwmLinuxGeneral* pwm = PwmLinuxGeneral::GetInstance(); \
    const iotjs_environment_t* env = iotjs_environment_get(); \
    uv_loop_t* loop = iotjs_environment_loop(env); \
    uv_work_t* req = req_wrap->req(); \
    uv_queue_work(loop, req, op ## Worker, AfterPwmWork); \
  } while (0)


int PwmLinuxGeneral::InitializePwmPath(PwmReqWrap* req_wrap) {
  int32_t chip_number, pwm_number;
  const char* path = iotjs_string_data(&req_wrap->device);
  char buffer[64] = {0};

  if (sscanf(path, PWM_PIN_FORMAT, &chip_number, &pwm_number) == 2) {

  } else if (sscanf(path, "%d", &pwm_number) == 1) {
    chip_number = PWM_DEFAULT_CHIP_NUMBER;
  } else {
    return -1;
  }

  // Create Device Path
  iotjs_string_make_empty(&req_wrap->device);
  snprintf(buffer, 63, PWM_PIN_FORMAT, chip_number, pwm_number);
  iotjs_string_append(&req_wrap->device, buffer, -1);

  return 0;
}


int PwmLinuxGeneral::Export(PwmReqWrap* req_wrap) {
  PWM_LINUX_GENERAL_IMPL_TEMPLATE(Export);
  return 0;
}


int PwmLinuxGeneral::SetPeriod(PwmReqWrap* req_wrap) {
  PWM_LINUX_GENERAL_IMPL_TEMPLATE(SetPeriod);
  return 0;
}


int PwmLinuxGeneral::SetDutyCycle(PwmReqWrap* req_wrap) {
  PWM_LINUX_GENERAL_IMPL_TEMPLATE(SetDutyCycle);
  return 0;
}


int PwmLinuxGeneral::SetEnable(PwmReqWrap* req_wrap) {
  PWM_LINUX_GENERAL_IMPL_TEMPLATE(SetEnable);
  return 0;
}


int PwmLinuxGeneral::Unexport(PwmReqWrap* req_wrap) {
  PWM_LINUX_GENERAL_IMPL_TEMPLATE(Unexport);
  return 0;
}


} // namespace iotjs


#endif /* IOTJS_MODULE_PWM_LINUX_GENERAL_INL_H */
