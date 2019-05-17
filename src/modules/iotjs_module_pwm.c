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
#include "iotjs_uv_request.h"


IOTJS_DEFINE_NATIVE_HANDLE_INFO_THIS_MODULE(pwm);

IOTJS_DEFINE_PERIPH_CREATE_FUNCTION(pwm);

static void iotjs_pwm_destroy(iotjs_pwm_t* pwm) {
  iotjs_pwm_destroy_platform_data(pwm->platform_data);
  IOTJS_RELEASE(pwm);
}

static void pwm_worker(uv_work_t* work_req) {
  iotjs_periph_data_t* worker_data =
      (iotjs_periph_data_t*)IOTJS_UV_REQUEST_EXTRA_DATA(work_req);
  iotjs_pwm_t* pwm = (iotjs_pwm_t*)worker_data->data;

  switch (worker_data->op) {
    case kPwmOpClose:
      worker_data->result = iotjs_pwm_close(pwm);
      break;
    case kPwmOpOpen:
      worker_data->result = iotjs_pwm_open(pwm);
      break;
    case kPwmOpSetDutyCycle:
      worker_data->result = iotjs_pwm_set_dutycycle(pwm);
      break;
    case kPwmOpSetEnable:
      worker_data->result = iotjs_pwm_set_enable(pwm);
      break;
    case kPwmOpSetFrequency: /* update the period */
    case kPwmOpSetPeriod:
      worker_data->result = iotjs_pwm_set_period(pwm);
      break;
    default:
      IOTJS_ASSERT(!"Invalid Operation");
  }
}

static jerry_value_t pwm_set_configuration(iotjs_pwm_t* pwm,
                                           jerry_value_t jconfig) {
  JS_GET_REQUIRED_CONF_VALUE(jconfig, pwm->duty_cycle,
                             IOTJS_MAGIC_STRING_DUTYCYCLE, number);
  if (pwm->duty_cycle < 0.0 || pwm->duty_cycle > 1.0) {
    return JS_CREATE_ERROR(RANGE, "pwm.dutyCycle must be within 0.0 and 1.0");
  }

  JS_GET_REQUIRED_CONF_VALUE(jconfig, pwm->period, IOTJS_MAGIC_STRING_PERIOD,
                             number);
  if (pwm->period < 0) {
    return JS_CREATE_ERROR(RANGE, "pwm.period must be a positive value");
  }

  JS_GET_REQUIRED_CONF_VALUE(jconfig, pwm->pin, IOTJS_MAGIC_STRING_PIN, number);

  return jerry_create_undefined();
}

JS_FUNCTION(pwm_constructor) {
  DJS_CHECK_THIS();
  DJS_CHECK_ARGS(1, object);
  DJS_CHECK_ARG_IF_EXIST(1, function);

  // Create PWM object
  jerry_value_t jpwm = JS_GET_THIS();
  iotjs_pwm_t* pwm = pwm_create(jpwm);

  jerry_value_t jconfig;
  JS_GET_REQUIRED_ARG_VALUE(0, jconfig, IOTJS_MAGIC_STRING_CONFIG, object);

  jerry_value_t res = iotjs_pwm_set_platform_config(pwm, jconfig);
  if (jerry_value_is_error(res)) {
    return res;
  }
  IOTJS_ASSERT(jerry_value_is_undefined(res));

  res = pwm_set_configuration(pwm, jconfig);
  if (jerry_value_is_error(res)) {
    return res;
  }
  IOTJS_ASSERT(jerry_value_is_undefined(res));

  jerry_value_t jcallback = JS_GET_ARG_IF_EXIST(1, function);

  // If the callback doesn't exist, it is completed synchronously.
  // Otherwise, it will be executed asynchronously.
  if (!jerry_value_is_null(jcallback)) {
    iotjs_periph_call_async(pwm, jcallback, kPwmOpOpen, pwm_worker);
  } else if (!iotjs_pwm_open(pwm)) {
    return JS_CREATE_ERROR(COMMON, iotjs_periph_error_str(kPwmOpOpen));
  }

  return jerry_create_undefined();
}

JS_FUNCTION(pwm_close) {
  JS_DECLARE_THIS_PTR(pwm, pwm);
  DJS_CHECK_ARG_IF_EXIST(1, function);

  iotjs_periph_call_async(pwm, JS_GET_ARG_IF_EXIST(0, function), kPwmOpClose,
                          pwm_worker);

  return jerry_create_undefined();
}

JS_FUNCTION(pwm_close_sync) {
  JS_DECLARE_THIS_PTR(pwm, pwm);

  if (!iotjs_pwm_close(pwm)) {
    return JS_CREATE_ERROR(COMMON, iotjs_periph_error_str(kPwmOpClose));
  }

  return jerry_create_undefined();
}

JS_FUNCTION(pwm_set_duty_cycle) {
  JS_DECLARE_THIS_PTR(pwm, pwm);
  DJS_CHECK_ARGS(1, number);
  DJS_CHECK_ARG_IF_EXIST(1, function);

  jerry_value_t jcallback = JS_GET_ARG_IF_EXIST(1, function);

  pwm->duty_cycle = JS_GET_ARG(0, number);
  if (pwm->duty_cycle < 0.0 || pwm->duty_cycle > 1.0) {
    return JS_CREATE_ERROR(RANGE, "pwm.dutyCycle must be within 0.0 and 1.0");
  }

  iotjs_periph_call_async(pwm, jcallback, kPwmOpSetDutyCycle, pwm_worker);

  return jerry_create_undefined();
}

JS_FUNCTION(pwm_set_duty_cycle_sync) {
  JS_DECLARE_THIS_PTR(pwm, pwm);
  DJS_CHECK_ARGS(1, number);

  pwm->duty_cycle = JS_GET_ARG(0, number);
  if (pwm->duty_cycle < 0.0 || pwm->duty_cycle > 1.0) {
    return JS_CREATE_ERROR(RANGE, "pwm.dutyCycle must be within 0.0 and 1.0");
  }

  if (!iotjs_pwm_set_dutycycle(pwm)) {
    return JS_CREATE_ERROR(COMMON, iotjs_periph_error_str(kPwmOpSetDutyCycle));
  }

  return jerry_create_undefined();
}

JS_FUNCTION(pwm_set_enable) {
  JS_DECLARE_THIS_PTR(pwm, pwm);
  DJS_CHECK_ARGS(1, boolean);
  DJS_CHECK_ARG_IF_EXIST(1, function);

  jerry_value_t jcallback = JS_GET_ARG_IF_EXIST(1, function);

  pwm->enable = JS_GET_ARG(0, boolean);

  iotjs_periph_call_async(pwm, jcallback, kPwmOpSetEnable, pwm_worker);

  return jerry_create_undefined();
}

JS_FUNCTION(pwm_set_enable_sync) {
  JS_DECLARE_THIS_PTR(pwm, pwm);
  DJS_CHECK_ARGS(1, boolean);

  pwm->enable = JS_GET_ARG(0, boolean);

  if (!iotjs_pwm_set_enable(pwm)) {
    return JS_CREATE_ERROR(COMMON, iotjs_periph_error_str(kPwmOpSetEnable));
  }

  return jerry_create_undefined();
}

static jerry_value_t pwm_set_period_or_frequency(iotjs_pwm_t* pwm,
                                                 const jerry_value_t jargv[],
                                                 const jerry_length_t jargc,
                                                 uint8_t op, bool async) {
  const double num_value = JS_GET_ARG(0, number);

  if (op == kPwmOpSetFrequency) {
    if (num_value <= 0) {
      return JS_CREATE_ERROR(RANGE, "frequency must be greater than 0");
    }
    pwm->period = 1.0 / num_value;

  } else {
    if (num_value < 0) {
      return JS_CREATE_ERROR(RANGE, "period must be a positive value");
    }
    pwm->period = num_value;
  }

  if (async) {
    iotjs_periph_call_async(pwm, JS_GET_ARG_IF_EXIST(1, function), op,
                            pwm_worker);
  } else {
    if (!iotjs_pwm_set_period(pwm)) {
      return JS_CREATE_ERROR(COMMON, iotjs_periph_error_str(op));
    }
  }

  return jerry_create_undefined();
}

JS_FUNCTION(pwm_set_frequency) {
  JS_DECLARE_THIS_PTR(pwm, pwm);
  DJS_CHECK_ARGS(1, number);
  DJS_CHECK_ARG_IF_EXIST(1, function);

  return pwm_set_period_or_frequency(pwm, jargv, jargc, kPwmOpSetFrequency,
                                     true);
}

JS_FUNCTION(pwm_set_frequency_sync) {
  JS_DECLARE_THIS_PTR(pwm, pwm);
  DJS_CHECK_ARGS(1, number);

  return pwm_set_period_or_frequency(pwm, jargv, jargc, kPwmOpSetFrequency,
                                     false);
}

JS_FUNCTION(pwm_set_period) {
  JS_DECLARE_THIS_PTR(pwm, pwm);
  DJS_CHECK_ARGS(1, number);
  DJS_CHECK_ARG_IF_EXIST(1, function);

  return pwm_set_period_or_frequency(pwm, jargv, jargc, kPwmOpSetPeriod, true);
}

JS_FUNCTION(pwm_set_period_sync) {
  JS_DECLARE_THIS_PTR(pwm, pwm);
  DJS_CHECK_ARGS(1, number);

  return pwm_set_period_or_frequency(pwm, jargv, jargc, kPwmOpSetPeriod, false);
}

jerry_value_t iotjs_init_pwm(void) {
  jerry_value_t jpwm_cons = jerry_create_external_function(pwm_constructor);

  jerry_value_t jprototype = jerry_create_object();

  iotjs_jval_set_method(jprototype, IOTJS_MAGIC_STRING_CLOSE, pwm_close);
  iotjs_jval_set_method(jprototype, IOTJS_MAGIC_STRING_CLOSESYNC,
                        pwm_close_sync);
  iotjs_jval_set_method(jprototype, IOTJS_MAGIC_STRING_SETDUTYCYCLE,
                        pwm_set_duty_cycle);
  iotjs_jval_set_method(jprototype, IOTJS_MAGIC_STRING_SETDUTYCYCLESYNC,
                        pwm_set_duty_cycle_sync);
  iotjs_jval_set_method(jprototype, IOTJS_MAGIC_STRING_SETENABLE,
                        pwm_set_enable);
  iotjs_jval_set_method(jprototype, IOTJS_MAGIC_STRING_SETENABLESYNC,
                        pwm_set_enable_sync);
  iotjs_jval_set_method(jprototype, IOTJS_MAGIC_STRING_SETFREQUENCY,
                        pwm_set_frequency);
  iotjs_jval_set_method(jprototype, IOTJS_MAGIC_STRING_SETFREQUENCYSYNC,
                        pwm_set_frequency_sync);
  iotjs_jval_set_method(jprototype, IOTJS_MAGIC_STRING_SETPERIOD,
                        pwm_set_period);
  iotjs_jval_set_method(jprototype, IOTJS_MAGIC_STRING_SETPERIODSYNC,
                        pwm_set_period_sync);

  iotjs_jval_set_property_jval(jpwm_cons, IOTJS_MAGIC_STRING_PROTOTYPE,
                               jprototype);

  jerry_release_value(jprototype);

  return jpwm_cons;
}
