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

#if defined(__TIZENRT__)
#include <iotbus_pwm.h>
#include <stdint.h>
#endif


typedef enum {
  kPwmOpOpen,
  kPwmOpSetDutyCycle,
  kPwmOpSetPeriod,
  kPwmOpSetFrequency,
  kPwmOpSetEnable,
  kPwmOpClose,
} PwmOp;


typedef struct {
  iotjs_jobjectwrap_t jobjectwrap;

#if defined(__linux__)
  int chip;
  iotjs_string_t device;
#elif defined(__NUTTX__)
  int device_fd;
#elif defined(__TIZENRT__)
  iotbus_pwm_context_h ctx;
#endif
  uint32_t pin;
  double duty_cycle;
  double period;
  bool enable;
} IOTJS_VALIDATED_STRUCT(iotjs_pwm_t);


typedef bool (*pwm_func_ptr)(iotjs_pwm_t*);


typedef struct {
  pwm_func_ptr caller;
  bool result;
  PwmOp op;
} iotjs_pwm_reqdata_t;


typedef struct {
  iotjs_reqwrap_t reqwrap;
  uv_work_t req;
  iotjs_pwm_reqdata_t req_data;
  iotjs_pwm_t* pwm_instance;
} IOTJS_VALIDATED_STRUCT(iotjs_pwm_reqwrap_t);


#define THIS iotjs_pwm_reqwrap_t* pwm_reqwrap

iotjs_pwm_reqwrap_t* iotjs_pwm_reqwrap_from_request(uv_work_t* req);
iotjs_pwm_reqdata_t* iotjs_pwm_reqwrap_data(THIS);

iotjs_pwm_t* iotjs_pwm_instance_from_reqwrap(THIS);

#undef THIS


#define PWM_WORKER_INIT                                                     \
  iotjs_pwm_reqwrap_t* req_wrap = iotjs_pwm_reqwrap_from_request(work_req); \
  iotjs_pwm_reqdata_t* req_data = iotjs_pwm_reqwrap_data(req_wrap);         \
  iotjs_pwm_t* pwm = iotjs_pwm_instance_from_reqwrap(req_wrap);


void iotjs_pwm_open_worker(uv_work_t* work_req);

bool iotjs_pwm_set_period(iotjs_pwm_t* pwm);
bool iotjs_pwm_set_dutycycle(iotjs_pwm_t* pwm);
bool iotjs_pwm_set_enable(iotjs_pwm_t* pwm);
bool iotjs_pwm_close(iotjs_pwm_t* pwm);


#endif /* IOTJS_MODULE_PWM_H */
