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


namespace iotjs {

enum PwmOp {
  kPwmOpExport,
  kPwmOpSetDutyCycle,
  kPwmOpSetPeriod,
  kPwmOpSetEnable,
  kPwmOpUnexport,
};

enum PwmError {
  kPwmErrOk = 0,
  kPwmErrExport = -1,
  kPwmErrUnexport = -2,
  kPwmErrEnable = -3,
  kPwmErrWrite = -4,
  kPwmErrSys = -5,
};

struct PwmReqData {
  iotjs_string_t device;
  int32_t duty_cycle;
  int32_t period;
  bool enable;

  PwmError result;
  PwmOp op;

  void* data; // pointer to PwmReqWrap
};

typedef ReqWrap<PwmReqData> PwmReqWrap;


// This Pwm class provides interfaces for PWM operation.
class Pwm {
 public:
  explicit Pwm(const iotjs_jval_t* jpwm);
  virtual ~Pwm();

  static Pwm* Create(const iotjs_jval_t* jpwm);
  static Pwm* GetInstance();
  static const iotjs_jval_t* GetJPwm();

  virtual int InitializePwmPath(PwmReqWrap* pwm_req) = 0;
  virtual int Export(PwmReqWrap* pwm_req) = 0;
  virtual int SetPeriod(PwmReqWrap* pwm_req) = 0;
  virtual int SetDutyCycle(PwmReqWrap* pwm_req) = 0;
  virtual int SetEnable(PwmReqWrap* pwm_req) = 0;
  virtual int Unexport(PwmReqWrap* pwm_req) = 0;

  static void Delete(const uintptr_t data) {
    delete ((Pwm*)data);
  }

 protected:
  iotjs_jobjectwrap_t _jobjectwrap;
};


} // namespace iotjs


#endif /* IOTJS_MODULE_PWM_H */
