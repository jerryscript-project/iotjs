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


#ifndef IOTJS_MODULE_PWM_H
#define IOTJS_MODULE_PWM_H

#include "iotjs_def.h"
#include "iotjs_objectwrap.h"
#include "iotjs_reqwrap.h"


typedef enum {
  kPwmOpExport,
  kPwmOpSetDutyCycle,
  kPwmOpSetPeriod,
  kPwmOpSetFrequency,
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
  int32_t pin;
  int32_t duty_cycle;
  int32_t period;
  bool enable;

  PwmError result;
  PwmOp op;
} iotjs_pwm_reqdata_t;


typedef struct {
  iotjs_reqwrap_t reqwrap;
  uv_work_t req;
  iotjs_pwm_reqdata_t req_data;
} IOTJS_VALIDATED_STRUCT(iotjs_pwm_reqwrap_t);


#define THIS iotjs_pwm_reqwrap_t* pwm_reqwrap
iotjs_pwm_reqwrap_t* iotjs_pwm_reqwrap_create(const iotjs_jval_t* jcallback,
                                              PwmOp op);
void iotjs_pwm_reqwrap_dispatched(THIS);
uv_work_t* iotjs_pwm_reqwrap_req(THIS);
const iotjs_jval_t* iotjs_pwm_reqwrap_jcallback(THIS);
iotjs_pwm_reqwrap_t* iotjs_pwm_reqwrap_from_request(uv_work_t* req);
iotjs_pwm_reqdata_t* iotjs_pwm_reqwrap_data(THIS);
#undef THIS


// This Pwm class provides interfaces for PWM operation.
typedef struct {
  iotjs_jobjectwrap_t jobjectwrap;
} IOTJS_VALIDATED_STRUCT(iotjs_pwm_t);

iotjs_pwm_t* iotjs_pwm_create(const iotjs_jval_t* jpwm);
const iotjs_jval_t* iotjs_pwm_get_jpwm();
iotjs_pwm_t* iotjs_pwm_get_instance();
void iotjs_pwm_initialize();

void ExportWorker(uv_work_t* work_req);
void SetPeriodWorker(uv_work_t* work_req);
void SetFrequencyWorker(uv_work_t* work_req);
void SetDutyCycleWorker(uv_work_t* work_req);
void SetEnableWorker(uv_work_t* work_req);
void UnexportWorker(uv_work_t* work_req);


#endif /* IOTJS_MODULE_PWM_H */
