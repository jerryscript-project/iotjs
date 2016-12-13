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

#include "iotjs_def.h"
#include "iotjs_module_pwm.h"
#include "iotjs_objectwrap.h"


#define THIS iotjs_pwm_reqwrap_t* pwm_reqwrap

iotjs_pwm_reqwrap_t* iotjs_pwm_reqwrap_create(const iotjs_jval_t* jcallback,
                                              PwmOp op) {
  iotjs_pwm_reqwrap_t* pwm_reqwrap = IOTJS_ALLOC(iotjs_pwm_reqwrap_t);
  IOTJS_VALIDATED_STRUCT_CONSTRUCTOR(iotjs_pwm_reqwrap_t, pwm_reqwrap);

  iotjs_reqwrap_initialize(&_this->reqwrap, jcallback, (uv_req_t*)&_this->req);

  _this->req_data.op = op;

  return pwm_reqwrap;
}


static void iotjs_pwm_reqwrap_destroy(THIS) {
  IOTJS_VALIDATED_STRUCT_DESTRUCTOR(iotjs_pwm_reqwrap_t, pwm_reqwrap);
  iotjs_reqwrap_destroy(&_this->reqwrap);
  IOTJS_RELEASE(pwm_reqwrap);
}


void iotjs_pwm_reqwrap_dispatched(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_pwm_reqwrap_t, pwm_reqwrap);
  iotjs_pwm_reqwrap_destroy(pwm_reqwrap);
}


uv_work_t* iotjs_pwm_reqwrap_req(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_pwm_reqwrap_t, pwm_reqwrap);
  return &_this->req;
}


const iotjs_jval_t* iotjs_pwm_reqwrap_jcallback(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_pwm_reqwrap_t, pwm_reqwrap);
  return iotjs_reqwrap_jcallback(&_this->reqwrap);
}


iotjs_pwm_reqwrap_t* iotjs_pwm_reqwrap_from_request(uv_work_t* req) {
  return (iotjs_pwm_reqwrap_t*)(iotjs_reqwrap_from_request((uv_req_t*)req));
}


iotjs_pwm_reqdata_t* iotjs_pwm_reqwrap_data(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_pwm_reqwrap_t, pwm_reqwrap);
  return &_this->req_data;
}


void iotjs_pwm_set_pwmdata(iotjs_jhandler_t* jhandler, THIS) {
  iotjs_pwm_reqdata_t* req_data = iotjs_pwm_reqwrap_data(pwm_reqwrap);
  req_data->pin = JHANDLER_GET_ARG(0, number);

  const iotjs_jval_t* joptions = JHANDLER_GET_ARG(1, object);
  iotjs_jval_t jperiod = iotjs_jval_get_property(joptions, "period");
  if (iotjs_jval_is_number(&jperiod))
    req_data->period = iotjs_jval_as_number(&jperiod);

  iotjs_jval_t jduty_cycle = iotjs_jval_get_property(joptions, "dutyCycle");
  if (iotjs_jval_is_number(&jduty_cycle))
    req_data->duty_cycle = iotjs_jval_as_number(&jduty_cycle);

  iotjs_jval_destroy(&jperiod);
  iotjs_jval_destroy(&jduty_cycle);
}

#undef THIS


static void iotjs_pwm_destroy(iotjs_pwm_t* pwm);


iotjs_pwm_t* iotjs_pwm_create(const iotjs_jval_t* jpwm) {
  iotjs_pwm_t* pwm = IOTJS_ALLOC(iotjs_pwm_t);
  IOTJS_VALIDATED_STRUCT_CONSTRUCTOR(iotjs_pwm_t, pwm);
  iotjs_jobjectwrap_initialize(&_this->jobjectwrap, jpwm,
                               (JFreeHandlerType)iotjs_pwm_destroy);
  return pwm;
}


static void iotjs_pwm_destroy(iotjs_pwm_t* pwm) {
  IOTJS_VALIDATED_STRUCT_DESTRUCTOR(iotjs_pwm_t, pwm);
  iotjs_jobjectwrap_destroy(&_this->jobjectwrap);
  IOTJS_RELEASE(pwm);
}


const iotjs_jval_t* iotjs_pwm_get_jpwm() {
  return iotjs_module_get(MODULE_PWM);
}


iotjs_pwm_t* iotjs_pwm_get_instance() {
  const iotjs_jval_t* jpwm = iotjs_pwm_get_jpwm();
  iotjs_jobjectwrap_t* jobjectwrap = iotjs_jobjectwrap_from_jobject(jpwm);
  return (iotjs_pwm_t*)jobjectwrap;
}


void AfterPwmWork(uv_work_t* work_req, int status) {
  iotjs_pwm_t* pwm = iotjs_pwm_get_instance();

  iotjs_pwm_reqwrap_t* req_wrap = iotjs_pwm_reqwrap_from_request(work_req);
  iotjs_pwm_reqdata_t* req_data = iotjs_pwm_reqwrap_data(req_wrap);

  if (status) {
    req_data->result = kPwmErrSys;
  }

  iotjs_jargs_t jargs = iotjs_jargs_create(1);
  iotjs_jargs_append_number(&jargs, req_data->result);

  switch (req_data->op) {
    case kPwmOpExport:
    case kPwmOpSetDutyCycle:
    case kPwmOpSetPeriod:
    case kPwmOpSetFrequency:
    case kPwmOpSetEnable:
    case kPwmOpUnexport:
      break;
    default: {
      IOTJS_ASSERT(!"Unreachable");
      break;
    }
  }

  const iotjs_jval_t* jcallback = iotjs_pwm_reqwrap_jcallback(req_wrap);
  const iotjs_jval_t* jpwm = iotjs_pwm_get_jpwm();
  iotjs_make_callback(jcallback, jpwm, &jargs);

  iotjs_jargs_destroy(&jargs);

  iotjs_pwm_reqwrap_dispatched(req_wrap);
}


#define PWM_ASYNC(op)                                                  \
  do {                                                                 \
    uv_loop_t* loop = iotjs_environment_loop(iotjs_environment_get()); \
    uv_work_t* req = iotjs_pwm_reqwrap_req(req_wrap);                  \
    uv_queue_work(loop, req, op##Worker, AfterPwmWork);                \
  } while (0)


JHANDLER_FUNCTION(Export) {
  JHANDLER_CHECK_ARGS(3, number, object, function);

  const iotjs_jval_t* jcallback = JHANDLER_GET_ARG(2, function);

  iotjs_pwm_reqwrap_t* req_wrap =
      iotjs_pwm_reqwrap_create(jcallback, kPwmOpExport);

  iotjs_pwm_set_pwmdata(jhandler, req_wrap);

  PWM_ASYNC(Export);

  iotjs_jhandler_return_null(jhandler);
}


JHANDLER_FUNCTION(SetPeriod) {
  JHANDLER_CHECK_ARGS(3, number, object, function);

  const iotjs_jval_t* jcallback = JHANDLER_GET_ARG(2, function);

  iotjs_pwm_reqwrap_t* req_wrap =
      iotjs_pwm_reqwrap_create(jcallback, kPwmOpSetPeriod);

  iotjs_pwm_set_pwmdata(jhandler, req_wrap);

  PWM_ASYNC(SetPeriod);

  iotjs_jhandler_return_null(jhandler);
}


JHANDLER_FUNCTION(SetFrequency) {
  JHANDLER_CHECK_ARGS(3, number, object, function);

  const iotjs_jval_t* jcallback = JHANDLER_GET_ARG(2, function);

  iotjs_pwm_reqwrap_t* req_wrap =
      iotjs_pwm_reqwrap_create(jcallback, kPwmOpSetFrequency);

  iotjs_pwm_set_pwmdata(jhandler, req_wrap);

  PWM_ASYNC(SetFrequency);

  iotjs_jhandler_return_null(jhandler);
}

JHANDLER_FUNCTION(SetDutyCycle) {
  JHANDLER_CHECK_ARGS(3, number, object, function);

  const iotjs_jval_t* jcallback = JHANDLER_GET_ARG(2, function);

  iotjs_pwm_reqwrap_t* req_wrap =
      iotjs_pwm_reqwrap_create(jcallback, kPwmOpSetDutyCycle);

  iotjs_pwm_set_pwmdata(jhandler, req_wrap);

  PWM_ASYNC(SetDutyCycle);

  iotjs_jhandler_return_null(jhandler);
}


JHANDLER_FUNCTION(SetEnable) {
  JHANDLER_CHECK_ARGS(3, number, boolean, function);

  const iotjs_jval_t* jcallback = JHANDLER_GET_ARG(2, function);

  iotjs_pwm_reqwrap_t* req_wrap =
      iotjs_pwm_reqwrap_create(jcallback, kPwmOpSetEnable);

  iotjs_pwm_reqdata_t* req_data = iotjs_pwm_reqwrap_data(req_wrap);
  req_data->pin = JHANDLER_GET_ARG(0, number);
  req_data->enable = JHANDLER_GET_ARG(1, boolean);

  PWM_ASYNC(SetEnable);

  iotjs_jhandler_return_null(jhandler);
}


JHANDLER_FUNCTION(Unexport) {
  JHANDLER_CHECK_ARGS(2, number, function);

  const iotjs_jval_t* jcallback = JHANDLER_GET_ARG(1, function);

  iotjs_pwm_reqwrap_t* req_wrap =
      iotjs_pwm_reqwrap_create(jcallback, kPwmOpUnexport);

  iotjs_pwm_reqdata_t* req_data = iotjs_pwm_reqwrap_data(req_wrap);
  req_data->pin = JHANDLER_GET_ARG(0, number);

  PWM_ASYNC(Unexport);

  iotjs_jhandler_return_null(jhandler);
}


iotjs_jval_t InitPwm() {
  iotjs_jval_t jpwm = iotjs_jval_create_object();

  iotjs_jval_set_method(&jpwm, "export", Export);
  iotjs_jval_set_method(&jpwm, "setPeriod", SetPeriod);
  iotjs_jval_set_method(&jpwm, "setFrequency", SetFrequency);
  iotjs_jval_set_method(&jpwm, "setDutyCycle", SetDutyCycle);
  iotjs_jval_set_method(&jpwm, "setEnable", SetEnable);
  iotjs_jval_set_method(&jpwm, "unexport", Unexport);


#define SET_PWM_CONSTANT(object, constant)                       \
  do {                                                           \
    iotjs_jval_set_property_number(object, #constant, constant); \
  } while (0)

  SET_PWM_CONSTANT(&jpwm, kPwmErrOk);
  SET_PWM_CONSTANT(&jpwm, kPwmErrExport);
  SET_PWM_CONSTANT(&jpwm, kPwmErrUnexport);
  SET_PWM_CONSTANT(&jpwm, kPwmErrEnable);
  SET_PWM_CONSTANT(&jpwm, kPwmErrWrite);
  SET_PWM_CONSTANT(&jpwm, kPwmErrSys);

#undef SET_PWM_CONSTANT

  iotjs_pwm_initialize();

  iotjs_pwm_t* pwm = iotjs_pwm_create(&jpwm);
  IOTJS_ASSERT(pwm ==
               (iotjs_pwm_t*)(iotjs_jval_get_object_native_handle(&jpwm)));

  return jpwm;
}
