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

#include "iotjs_def.h"
#include "iotjs_objectwrap.h"
#include "iotjs_module_pwm.h"


namespace iotjs {


Pwm::Pwm(const iotjs_jval_t* jpwm) {
  iotjs_jobjectwrap_initialize(&_jobjectwrap, jpwm, (uintptr_t)this, Delete);
}


Pwm::~Pwm() {
  iotjs_jobjectwrap_destroy(&_jobjectwrap);
}


const iotjs_jval_t* Pwm::GetJPwm() {
  return iotjs_module_get(MODULE_PWM);
}


Pwm* Pwm::GetInstance() {
  Pwm* pwm = reinterpret_cast<Pwm*>(
          iotjs_jval_get_object_native_handle(Pwm::GetJPwm()));
  IOTJS_ASSERT(pwm != NULL);

  return pwm;
}


JHANDLER_FUNCTION(Export) {
  JHANDLER_CHECK_ARGS(3, string, object, function);

  // PwmReqWrap is freed by AfterPwmWork.
  PwmReqWrap* req_wrap = new PwmReqWrap(JHANDLER_GET_ARG(2, function));

  const iotjs_jval_t* joption = JHANDLER_GET_ARG(1, object);
  iotjs_jval_t jperiod = iotjs_jval_get_property(joption, "period");
  iotjs_jval_t jduty_cycle = iotjs_jval_get_property(joption, "dutyCycle");

  req_wrap->op = kPwmOpExport;
  req_wrap->device = JHANDLER_GET_ARG(0, string);

  // Set options.
  if (iotjs_jval_is_number(&jperiod)) {
    req_wrap->period = iotjs_jval_as_number(&jperiod);
  } else {
    req_wrap->period = -1;
  }

  if (iotjs_jval_is_number(&jduty_cycle)) {
    req_wrap->duty_cycle = iotjs_jval_as_number(&jduty_cycle);
  } else {
    req_wrap->duty_cycle = -1;
  }

  Pwm* pwm = Pwm::GetInstance();
  if (pwm->InitializePwmPath(req_wrap) < 0) {
    iotjs_string_destroy(&req_wrap->device);
    delete req_wrap;

    JHANDLER_THROW(TYPE, "Invalid Pwm Path");
  }
  pwm->Export(req_wrap);


  iotjs_jval_destroy(&jperiod);
  iotjs_jval_destroy(&jduty_cycle);

  // Return exported pwm device path.
  iotjs_jhandler_return_string(jhandler, &req_wrap->device);
}


JHANDLER_FUNCTION(SetPeriod) {
  JHANDLER_CHECK_ARGS(3, string, number, function);

  PwmReqWrap* req_wrap = new PwmReqWrap(JHANDLER_GET_ARG(2, function));

  req_wrap->device = JHANDLER_GET_ARG(0, string);
  req_wrap->period = JHANDLER_GET_ARG(1, number);
  req_wrap->op = kPwmOpSetPeriod;

  Pwm* pwm = Pwm::GetInstance();
  pwm->SetPeriod(req_wrap);

  iotjs_jhandler_return_null(jhandler);
}


JHANDLER_FUNCTION(SetDutyCycle) {
  JHANDLER_CHECK_ARGS(3, string, number, function);

  PwmReqWrap* req_wrap = new PwmReqWrap(JHANDLER_GET_ARG(2, function));

  req_wrap->device = JHANDLER_GET_ARG(0, string);
  req_wrap->duty_cycle = JHANDLER_GET_ARG(1, number);
  req_wrap->op = kPwmOpSetDutyCycle;

  Pwm* pwm = Pwm::GetInstance();
  pwm->SetDutyCycle(req_wrap);

  iotjs_jhandler_return_null(jhandler);
}


JHANDLER_FUNCTION(SetEnable) {
  JHANDLER_CHECK_ARGS(3, string, boolean, function);

  PwmReqWrap* req_wrap = new PwmReqWrap(JHANDLER_GET_ARG(2, function));

  req_wrap->device = JHANDLER_GET_ARG(0, string);
  req_wrap->enable = JHANDLER_GET_ARG(1, boolean);
  req_wrap->op = kPwmOpSetEnable;

  Pwm* pwm = Pwm::GetInstance();
  pwm->SetEnable(req_wrap);

  iotjs_jhandler_return_null(jhandler);
}


JHANDLER_FUNCTION(Unexport) {
  JHANDLER_CHECK_ARGS(2, string, function);

  PwmReqWrap* req_wrap = new PwmReqWrap(JHANDLER_GET_ARG(1, function));

  req_wrap->device = JHANDLER_GET_ARG(0, string);
  req_wrap->op = kPwmOpUnexport;

  Pwm* pwm = Pwm::GetInstance();
  pwm->Unexport(req_wrap);

  iotjs_jhandler_return_null(jhandler);
}


iotjs_jval_t InitPwm() {
  iotjs_jval_t jpwm = iotjs_jval_create_object();

  iotjs_jval_set_method(&jpwm, "export", Export);
  iotjs_jval_set_method(&jpwm, "setPeriod", SetPeriod);
  iotjs_jval_set_method(&jpwm, "setDutyCycle", SetDutyCycle);
  iotjs_jval_set_method(&jpwm, "setEnable", SetEnable);
  iotjs_jval_set_method(&jpwm, "unexport", Unexport);


#define SET_PWM_CONSTANT(object, constant) \
  do { \
    iotjs_jval_set_property_number(object, #constant, constant); \
  } while (0)

    SET_PWM_CONSTANT(&jpwm, kPwmErrOk);
    SET_PWM_CONSTANT(&jpwm, kPwmErrExport);
    SET_PWM_CONSTANT(&jpwm, kPwmErrUnexport);
    SET_PWM_CONSTANT(&jpwm, kPwmErrEnable);
    SET_PWM_CONSTANT(&jpwm, kPwmErrWrite);
    SET_PWM_CONSTANT(&jpwm, kPwmErrSys);

#undef SET_PWM_CONSTANT

    Pwm* pwm = Pwm::Create(&jpwm);
    IOTJS_ASSERT(pwm == reinterpret_cast<Pwm*>(
                 iotjs_jval_get_object_native_handle(&jpwm)));

  return jpwm;
}


} // namespace iotjs


extern "C" {

iotjs_jval_t InitPwm() {
  return iotjs::InitPwm();
}

} // extern "C"
