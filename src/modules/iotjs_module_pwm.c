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


IOTJS_DEFINE_NATIVE_HANDLE_INFO_THIS_MODULE(pwm);

static iotjs_pwm_t* pwm_create(jerry_value_t jpwm) {
  iotjs_pwm_t* pwm = IOTJS_ALLOC(iotjs_pwm_t);
  iotjs_pwm_create_platform_data(pwm);
  pwm->jobject = jpwm;
  pwm->period = -1;
  pwm->duty_cycle = 0;
  jerry_set_object_native_pointer(jpwm, pwm, &this_module_native_info);

  return pwm;
}

static iotjs_pwm_reqwrap_t* iotjs_pwm_reqwrap_create(jerry_value_t jcallback,
                                                     iotjs_pwm_t* pwm,
                                                     PwmOp op) {
  iotjs_pwm_reqwrap_t* pwm_reqwrap = IOTJS_ALLOC(iotjs_pwm_reqwrap_t);

  iotjs_reqwrap_initialize(&pwm_reqwrap->reqwrap, jcallback,
                           (uv_req_t*)&pwm_reqwrap->req);

  pwm_reqwrap->req_data.op = op;
  pwm_reqwrap->pwm_data = pwm;

  return pwm_reqwrap;
}

static void pwm_reqwrap_destroy(iotjs_pwm_reqwrap_t* pwm_reqwrap) {
  iotjs_reqwrap_destroy(&pwm_reqwrap->reqwrap);
  IOTJS_RELEASE(pwm_reqwrap);
}

static void iotjs_pwm_destroy(iotjs_pwm_t* pwm) {
  iotjs_pwm_destroy_platform_data(pwm->platform_data);
  IOTJS_RELEASE(pwm);
}

static void pwm_worker(uv_work_t* work_req) {
  iotjs_pwm_reqwrap_t* req_wrap =
      (iotjs_pwm_reqwrap_t*)(iotjs_reqwrap_from_request((uv_req_t*)work_req));
  iotjs_pwm_reqdata_t* req_data = &req_wrap->req_data;
  iotjs_pwm_t* pwm = req_wrap->pwm_data;

  switch (req_data->op) {
    case kPwmOpClose:
      req_data->result = iotjs_pwm_close(pwm);
      break;
    case kPwmOpOpen:
      req_data->result = iotjs_pwm_open(pwm);
      break;
    case kPwmOpSetDutyCycle:
      req_data->result = iotjs_pwm_set_dutycycle(pwm);
      break;
    case kPwmOpSetEnable:
      req_data->result = iotjs_pwm_set_enable(pwm);
      break;
    case kPwmOpSetFrequency: /* update the period */
    case kPwmOpSetPeriod:
      req_data->result = iotjs_pwm_set_period(pwm);
      break;
    default:
      IOTJS_ASSERT(!"Invalid Operation");
  }
}

static const char* pwm_error_str(int op) {
  switch (op) {
    case kPwmOpClose:
      return "Cannot close PWM device";
    case kPwmOpOpen:
      return "Failed to open PWM device";
    case kPwmOpSetDutyCycle:
      return "Failed to set duty-cycle";
    case kPwmOpSetEnable:
      return "Failed to set enable";
    case kPwmOpSetFrequency:
      return "Failed to set frequency";
    case kPwmOpSetPeriod:
      return "Failed to set period";
    default:
      return "Unknown error";
  }
}

static void pwm_after_worker(uv_work_t* work_req, int status) {
  iotjs_pwm_reqwrap_t* req_wrap =
      (iotjs_pwm_reqwrap_t*)(iotjs_reqwrap_from_request((uv_req_t*)work_req));
  iotjs_pwm_reqdata_t* req_data = &req_wrap->req_data;

  iotjs_jargs_t jargs = iotjs_jargs_create(1);

  if (status) {
    iotjs_jargs_append_error(&jargs, "System error");
  } else {
    switch (req_data->op) {
      case kPwmOpClose:
      case kPwmOpOpen:
      case kPwmOpSetDutyCycle:
      case kPwmOpSetEnable:
      case kPwmOpSetFrequency:
      case kPwmOpSetPeriod: {
        if (!req_data->result) {
          iotjs_jargs_append_error(&jargs, pwm_error_str(req_data->op));
        } else {
          iotjs_jargs_append_null(&jargs);
        }
        break;
      }
      default: {
        IOTJS_ASSERT(!"Unreachable");
        break;
      }
    }
  }

  jerry_value_t jcallback = iotjs_reqwrap_jcallback(&req_wrap->reqwrap);
  if (jerry_value_is_function(jcallback)) {
    iotjs_make_callback(jcallback, jerry_create_undefined(), &jargs);
  }

  iotjs_jargs_destroy(&jargs);
  pwm_reqwrap_destroy(req_wrap);
}

#define PWM_CALL_ASYNC(op, jcallback)                                  \
  do {                                                                 \
    uv_loop_t* loop = iotjs_environment_loop(iotjs_environment_get()); \
    iotjs_pwm_reqwrap_t* req_wrap =                                    \
        iotjs_pwm_reqwrap_create(jcallback, pwm, op);                  \
    uv_work_t* req = &req_wrap->req;                                   \
    uv_queue_work(loop, req, pwm_worker, pwm_after_worker);            \
  } while (0)

JS_FUNCTION(PwmCons) {
  DJS_CHECK_THIS();
  DJS_CHECK_ARGS(1, object);
  DJS_CHECK_ARG_IF_EXIST(1, function);

  // Create PWM object
  jerry_value_t jpwm = JS_GET_THIS();
  iotjs_pwm_t* pwm = pwm_create(jpwm);

  jerry_value_t jconfig;
  JS_GET_REQUIRED_ARG_VALUE(0, jconfig, IOTJS_MAGIC_STRING_CONFIG, object);

  jerry_value_t res = iotjs_pwm_set_platform_config(pwm, jconfig);
  if (jerry_value_has_error_flag(res)) {
    return res;
  }

  DJS_GET_REQUIRED_CONF_VALUE(jconfig, pwm->duty_cycle,
                              IOTJS_MAGIC_STRING_DUTYCYCLE, number);
  DJS_GET_REQUIRED_CONF_VALUE(jconfig, pwm->period, IOTJS_MAGIC_STRING_PERIOD,
                              number);
  DJS_GET_REQUIRED_CONF_VALUE(jconfig, pwm->pin, IOTJS_MAGIC_STRING_PIN,
                              number);

  jerry_value_t jcallback = JS_GET_ARG_IF_EXIST(1, function);

  // If the callback doesn't exist, it is completed synchronously.
  // Otherwise, it will be executed asynchronously.
  if (!jerry_value_is_null(jcallback)) {
    PWM_CALL_ASYNC(kPwmOpOpen, jcallback);
  } else if (!iotjs_pwm_open(pwm)) {
    return JS_CREATE_ERROR(COMMON, pwm_error_str(kPwmOpOpen));
  }

  return jerry_create_undefined();
}

JS_FUNCTION(Close) {
  JS_DECLARE_THIS_PTR(pwm, pwm);
  DJS_CHECK_ARG_IF_EXIST(1, function);

  PWM_CALL_ASYNC(kPwmOpClose, JS_GET_ARG_IF_EXIST(0, function));

  return jerry_create_undefined();
}

JS_FUNCTION(CloseSync) {
  JS_DECLARE_THIS_PTR(pwm, pwm);

  if (!iotjs_pwm_close(pwm)) {
    return JS_CREATE_ERROR(COMMON, pwm_error_str(kPwmOpClose));
  }

  return jerry_create_undefined();
}

JS_FUNCTION(SetDutyCycle) {
  JS_DECLARE_THIS_PTR(pwm, pwm);
  DJS_CHECK_ARGS(1, number);
  DJS_CHECK_ARG_IF_EXIST(1, function);

  jerry_value_t jcallback = JS_GET_ARG_IF_EXIST(1, function);

  pwm->duty_cycle = JS_GET_ARG(0, number);

  PWM_CALL_ASYNC(kPwmOpSetDutyCycle, jcallback);

  return jerry_create_undefined();
}

JS_FUNCTION(SetDutyCycleSync) {
  JS_DECLARE_THIS_PTR(pwm, pwm);
  DJS_CHECK_ARGS(1, number);

  pwm->duty_cycle = JS_GET_ARG(0, number);

  if (!iotjs_pwm_set_dutycycle(pwm)) {
    return JS_CREATE_ERROR(COMMON, pwm_error_str(kPwmOpSetDutyCycle));
  }

  return jerry_create_undefined();
}

JS_FUNCTION(SetEnable) {
  JS_DECLARE_THIS_PTR(pwm, pwm);
  DJS_CHECK_ARGS(1, boolean);
  DJS_CHECK_ARG_IF_EXIST(1, function);

  jerry_value_t jcallback = JS_GET_ARG_IF_EXIST(1, function);

  pwm->enable = JS_GET_ARG(0, boolean);

  PWM_CALL_ASYNC(kPwmOpSetEnable, jcallback);

  return jerry_create_undefined();
}

JS_FUNCTION(SetEnableSync) {
  JS_DECLARE_THIS_PTR(pwm, pwm);
  DJS_CHECK_ARGS(1, boolean);

  pwm->enable = JS_GET_ARG(0, boolean);

  if (!iotjs_pwm_set_enable(pwm)) {
    return JS_CREATE_ERROR(COMMON, pwm_error_str(kPwmOpSetEnable));
  }

  return jerry_create_undefined();
}

static jerry_value_t pwm_set_period_or_frequency(iotjs_pwm_t* pwm,
                                                 const jerry_value_t jargv[],
                                                 const jerry_length_t jargc,
                                                 uint8_t op, bool async) {
  if (op == kPwmOpSetFrequency) {
    pwm->period = 1.0 / JS_GET_ARG(0, number);
  } else {
    pwm->period = JS_GET_ARG(0, number);
  }

  if (async) {
    PWM_CALL_ASYNC(op, JS_GET_ARG_IF_EXIST(1, function));
  } else {
    if (!iotjs_pwm_set_period(pwm)) {
      return JS_CREATE_ERROR(COMMON, pwm_error_str(op));
    }
  }

  return jerry_create_undefined();
}

JS_FUNCTION(SetFrequency) {
  JS_DECLARE_THIS_PTR(pwm, pwm);
  DJS_CHECK_ARGS(1, number);
  DJS_CHECK_ARG_IF_EXIST(1, function);

  return pwm_set_period_or_frequency(pwm, jargv, jargc, kPwmOpSetFrequency,
                                     true);
}

JS_FUNCTION(SetFrequencySync) {
  JS_DECLARE_THIS_PTR(pwm, pwm);
  DJS_CHECK_ARGS(1, number);

  return pwm_set_period_or_frequency(pwm, jargv, jargc, kPwmOpSetFrequency,
                                     false);
}

JS_FUNCTION(SetPeriod) {
  JS_DECLARE_THIS_PTR(pwm, pwm);
  DJS_CHECK_ARGS(1, number);
  DJS_CHECK_ARG_IF_EXIST(1, function);

  return pwm_set_period_or_frequency(pwm, jargv, jargc, kPwmOpSetPeriod, true);
}

JS_FUNCTION(SetPeriodSync) {
  JS_DECLARE_THIS_PTR(pwm, pwm);
  DJS_CHECK_ARGS(1, number);

  return pwm_set_period_or_frequency(pwm, jargv, jargc, kPwmOpSetPeriod, false);
}

jerry_value_t InitPwm() {
  jerry_value_t jpwm_cons = jerry_create_external_function(PwmCons);

  jerry_value_t jprototype = jerry_create_object();

  iotjs_jval_set_method(jprototype, IOTJS_MAGIC_STRING_CLOSE, Close);
  iotjs_jval_set_method(jprototype, IOTJS_MAGIC_STRING_CLOSESYNC, CloseSync);
  iotjs_jval_set_method(jprototype, IOTJS_MAGIC_STRING_SETDUTYCYCLE,
                        SetDutyCycle);
  iotjs_jval_set_method(jprototype, IOTJS_MAGIC_STRING_SETDUTYCYCLESYNC,
                        SetDutyCycleSync);
  iotjs_jval_set_method(jprototype, IOTJS_MAGIC_STRING_SETENABLE, SetEnable);
  iotjs_jval_set_method(jprototype, IOTJS_MAGIC_STRING_SETENABLESYNC,
                        SetEnableSync);
  iotjs_jval_set_method(jprototype, IOTJS_MAGIC_STRING_SETFREQUENCY,
                        SetFrequency);
  iotjs_jval_set_method(jprototype, IOTJS_MAGIC_STRING_SETFREQUENCYSYNC,
                        SetFrequencySync);
  iotjs_jval_set_method(jprototype, IOTJS_MAGIC_STRING_SETPERIOD, SetPeriod);
  iotjs_jval_set_method(jprototype, IOTJS_MAGIC_STRING_SETPERIODSYNC,
                        SetPeriodSync);

  iotjs_jval_set_property_jval(jpwm_cons, IOTJS_MAGIC_STRING_PROTOTYPE,
                               jprototype);

  jerry_release_value(jprototype);

  return jpwm_cons;
}
