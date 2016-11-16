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


#ifndef IOTJS_MODULE_PWM_H
#define IOTJS_MODULE_PWM_H

#include "iotjs_def.h"
#include "iotjs_objectwrap.h"
#include "iotjs_reqwrap.h"


typedef enum {
  kPwmOpExport,
  kPwmOpSetDutyCycle,
  kPwmOpSetPeriod,
  kPwmOpSetEnable,
  kPwmOpUnexport,
} PwmOp;

typedef enum {
  kPwmErrOk = 0,
  kPwmErrExport = -1,
  kPwmErrUnexport = -2,
  kPwmErrEnable = -3,
  kPwmErrWrite = -4,
  kPwmErrSys = -5,
} PwmError;


typedef struct {
  iotjs_string_t device;
  int32_t duty_cycle;
  int32_t period;
  bool enable;

  PwmError result;
  PwmOp op;
} iotjs_pwmreqdata_t;


typedef struct {
  iotjs_reqwrap_t reqwrap;
  uv_work_t req;
  iotjs_pwmreqdata_t req_data;
} IOTJS_VALIDATED_STRUCT(iotjs_pwmreqwrap_t);


#define THIS iotjs_pwmreqwrap_t* pwmreqwrap
void iotjs_pwmreqwrap_initialize(THIS, const iotjs_jval_t* jcallback, PwmOp op);
void iotjs_pwmreqwrap_destroy(THIS);
uv_work_t* iotjs_pwmreqwrap_req(THIS);
const iotjs_jval_t* iotjs_pwmreqwrap_jcallback(THIS);
iotjs_pwmreqwrap_t* iotjs_pwmreqwrap_from_request(uv_work_t* req);
iotjs_pwmreqdata_t* iotjs_pwmreqwrap_data(THIS);
#undef THIS


// This Pwm class provides interfaces for PWM operation.
typedef struct {
  iotjs_jobjectwrap_t jobjectwrap;
} IOTJS_VALIDATED_STRUCT(iotjs_pwm_t);

iotjs_pwm_t* iotjs_pwm_create(const iotjs_jval_t* jpwm);
void iotjs_pwm_destroy(iotjs_pwm_t* pwm);
const iotjs_jval_t* iotjs_pwm_get_jpwm();
iotjs_pwm_t* iotjs_pwm_get_instance();
bool iotjs_pwm_initialized();
void iotjs_pwm_set_initialized(iotjs_pwm_t* pwm, bool initialized);


int PwmInitializePwmPath(iotjs_pwmreqdata_t* req_data);
void ExportWorker(uv_work_t* work_req);
void SetPeriodWorker(uv_work_t* work_req);
void SetDutyCycleWorker(uv_work_t* work_req);
void SetEnableWorker(uv_work_t* work_req);
void UnexportWorker(uv_work_t* work_req);


#endif /* IOTJS_MODULE_PWM_H */
