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

#if defined(__NUTTX__)


#include "module/iotjs_module_pwm.h"


namespace iotjs {


// PWM implementation for arm-nuttx target.
class PwmArmNuttx : public Pwm {
 public:
  explicit PwmArmNuttx(const iotjs_jval_t* jpwm);

  static PwmArmNuttx* GetInstance();
  virtual int InitializePwmPath(PwmReqWrap* pwm_req);
  virtual int Export(PwmReqWrap* pwm_req);
  virtual int SetPeriod(PwmReqWrap* pwm_req);
  virtual int SetDutyCycle(PwmReqWrap* pwm_req);
  virtual int SetEnable(PwmReqWrap* pwm_req);
  virtual int Unexport(PwmReqWrap* pwm_req);
};


Pwm* Pwm::Create(const iotjs_jval_t* jpwm) {
  return new PwmArmNuttx(jpwm);
}


PwmArmNuttx::PwmArmNuttx(const iotjs_jval_t* jpwm)
    : Pwm(jpwm) {
}


PwmArmNuttx* PwmArmNuttx::GetInstance() {
  return static_cast<PwmArmNuttx*>(Pwm::GetInstance());
}


int PwmArmNuttx::InitializePwmPath(PwmReqWrap* pwm_req) {
  IOTJS_ASSERT(!"Not implemented");
  return 0;
}


int PwmArmNuttx::Export(PwmReqWrap* pwm_req) {
  IOTJS_ASSERT(!"Not implemented");
  return 0;
}


int PwmArmNuttx::SetPeriod(PwmReqWrap* pwm_req) {
  IOTJS_ASSERT(!"Not implemented");
  return 0;
}


int PwmArmNuttx::SetDutyCycle(PwmReqWrap* pwm_req) {
  IOTJS_ASSERT(!"Not implemented");
  return 0;
}


int PwmArmNuttx::SetEnable(PwmReqWrap* pwm_req) {
  IOTJS_ASSERT(!"Not implemented");
  return 0;
}


int PwmArmNuttx::Unexport(PwmReqWrap* pwm_req) {
  IOTJS_ASSERT(!"Not implemented");
  return 0;
}


} // namespace iotjs

#endif // __NUTTX__
