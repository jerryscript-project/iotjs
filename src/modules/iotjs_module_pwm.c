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

static iotjs_pwm_t* iotjs_pwm_instance_from_jval(const iotjs_jval_t* jpwm);

IOTJS_DEFINE_NATIVE_HANDLE_INFO_THIS_MODULE(pwm);


static iotjs_pwm_t* iotjs_pwm_create(const iotjs_jval_t* jpwm) {
  iotjs_pwm_t* pwm = IOTJS_ALLOC(iotjs_pwm_t);
  IOTJS_VALIDATED_STRUCT_CONSTRUCTOR(iotjs_pwm_t, pwm);
  iotjs_jobjectwrap_initialize(&_this->jobjectwrap, jpwm,
                               &this_module_native_info);

  _this->period = -1;
  _this->duty_cycle = 0;
#if defined(__NUTTX__)
  _this->device_fd = -1;
#endif
  return pwm;
}


static void iotjs_pwm_destroy(iotjs_pwm_t* pwm) {
  IOTJS_VALIDATED_STRUCT_DESTRUCTOR(iotjs_pwm_t, pwm);
  iotjs_jobjectwrap_destroy(&_this->jobjectwrap);
#if defined(__linux__)
  iotjs_string_destroy(&_this->device);
#endif
  IOTJS_RELEASE(pwm);
}


#define THIS iotjs_pwm_reqwrap_t* pwm_reqwrap


static iotjs_pwm_reqwrap_t* iotjs_pwm_reqwrap_create(
    const iotjs_jval_t* jcallback, iotjs_pwm_t* pwm, PwmOp op) {
  iotjs_pwm_reqwrap_t* pwm_reqwrap = IOTJS_ALLOC(iotjs_pwm_reqwrap_t);
  IOTJS_VALIDATED_STRUCT_CONSTRUCTOR(iotjs_pwm_reqwrap_t, pwm_reqwrap);

  iotjs_reqwrap_initialize(&_this->reqwrap, jcallback, (uv_req_t*)&_this->req);

  _this->req_data.op = op;
  _this->pwm_instance = pwm;
  _this->req_data.caller = NULL;

  return pwm_reqwrap;
}


static void iotjs_pwm_reqwrap_destroy(THIS) {
  IOTJS_VALIDATED_STRUCT_DESTRUCTOR(iotjs_pwm_reqwrap_t, pwm_reqwrap);
  iotjs_reqwrap_destroy(&_this->reqwrap);
  IOTJS_RELEASE(pwm_reqwrap);
}


static void iotjs_pwm_reqwrap_dispatched(THIS) {
  IOTJS_VALIDATABLE_STRUCT_METHOD_VALIDATE(iotjs_pwm_reqwrap_t, pwm_reqwrap);
  iotjs_pwm_reqwrap_destroy(pwm_reqwrap);
}


static uv_work_t* iotjs_pwm_reqwrap_req(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_pwm_reqwrap_t, pwm_reqwrap);
  return &_this->req;
}


static const iotjs_jval_t* iotjs_pwm_reqwrap_jcallback(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_pwm_reqwrap_t, pwm_reqwrap);
  return iotjs_reqwrap_jcallback(&_this->reqwrap);
}


static iotjs_pwm_t* iotjs_pwm_instance_from_jval(const iotjs_jval_t* jpwm) {
  uintptr_t handle = iotjs_jval_get_object_native_handle(jpwm);
  return (iotjs_pwm_t*)handle;
}


iotjs_pwm_reqwrap_t* iotjs_pwm_reqwrap_from_request(uv_work_t* req) {
  return (iotjs_pwm_reqwrap_t*)(iotjs_reqwrap_from_request((uv_req_t*)req));
}


iotjs_pwm_reqdata_t* iotjs_pwm_reqwrap_data(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_pwm_reqwrap_t, pwm_reqwrap);
  return &_this->req_data;
}


iotjs_pwm_t* iotjs_pwm_instance_from_reqwrap(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_pwm_reqwrap_t, pwm_reqwrap);
  return _this->pwm_instance;
}


static void iotjs_pwm_set_configuration(const iotjs_jval_t* jconfiguration,
                                        iotjs_pwm_t* pwm) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_pwm_t, pwm);

  iotjs_jval_t jpin =
      iotjs_jval_get_property(jconfiguration, IOTJS_MAGIC_STRING_PIN);
  _this->pin = iotjs_jval_as_number(&jpin);

#if defined(__linux__)
  iotjs_jval_t jchip =
      iotjs_jval_get_property(jconfiguration, IOTJS_MAGIC_STRING_CHIP);
  _this->chip = iotjs_jval_as_number(&jchip);
  iotjs_jval_destroy(&jchip);
#endif

  iotjs_jval_t jperiod =
      iotjs_jval_get_property(jconfiguration, IOTJS_MAGIC_STRING_PERIOD);
  if (iotjs_jval_is_number(&jperiod))
    _this->period = iotjs_jval_as_number(&jperiod);

  iotjs_jval_t jduty_cycle =
      iotjs_jval_get_property(jconfiguration, IOTJS_MAGIC_STRING_DUTYCYCLE);
  if (iotjs_jval_is_number(&jduty_cycle))
    _this->duty_cycle = iotjs_jval_as_number(&jduty_cycle);

  iotjs_jval_destroy(&jpin);
  iotjs_jval_destroy(&jperiod);
  iotjs_jval_destroy(&jduty_cycle);
}

#undef THIS


static void iotjs_pwm_common_worker(uv_work_t* work_req) {
  PWM_WORKER_INIT;

  IOTJS_ASSERT(req_data->caller != NULL);

  if (!req_data->caller(pwm)) {
    req_data->result = false;
    return;
  }

  req_data->result = true;
}


static void iotjs_pwm_after_worker(uv_work_t* work_req, int status) {
  iotjs_pwm_reqwrap_t* req_wrap = iotjs_pwm_reqwrap_from_request(work_req);
  iotjs_pwm_reqdata_t* req_data = iotjs_pwm_reqwrap_data(req_wrap);
  iotjs_jargs_t jargs = iotjs_jargs_create(1);
  bool result = req_data->result;

  if (status) {
    iotjs_jval_t error = iotjs_jval_create_error("System error");
    iotjs_jargs_append_jval(&jargs, &error);
    iotjs_jval_destroy(&error);
  } else {
    switch (req_data->op) {
      case kPwmOpOpen:
        if (!result) {
          iotjs_jargs_append_error(&jargs, "Failed to open PWM device");
        } else {
          iotjs_jargs_append_null(&jargs);
        }
        break;
      case kPwmOpSetDutyCycle:
        if (!result) {
          iotjs_jargs_append_error(&jargs, "Failed to set duty-cycle");
        } else {
          iotjs_jargs_append_null(&jargs);
        }
        break;
      case kPwmOpSetPeriod:
        if (!result) {
          iotjs_jargs_append_error(&jargs, "Failed to set period");
        } else {
          iotjs_jargs_append_null(&jargs);
        }
        break;
      case kPwmOpSetEnable:
        if (!result) {
          iotjs_jargs_append_error(&jargs, "Failed to set enable");
        } else {
          iotjs_jargs_append_null(&jargs);
        }
        break;
      case kPwmOpClose:
        if (!result) {
          iotjs_jargs_append_error(&jargs, "Cannot close PWM device");
        } else {
          iotjs_jargs_append_null(&jargs);
        }
        break;
      default: {
        IOTJS_ASSERT(!"Unreachable");
        break;
      }
    }
  }

  const iotjs_jval_t* jcallback = iotjs_pwm_reqwrap_jcallback(req_wrap);
  iotjs_make_callback(jcallback, iotjs_jval_get_undefined(), &jargs);

  iotjs_jargs_destroy(&jargs);

  iotjs_pwm_reqwrap_dispatched(req_wrap);
}


#define PWM_ASYNC(call, this, jcallback, op)                           \
  do {                                                                 \
    uv_loop_t* loop = iotjs_environment_loop(iotjs_environment_get()); \
    iotjs_pwm_reqwrap_t* req_wrap =                                    \
        iotjs_pwm_reqwrap_create(jcallback, this, op);                 \
    uv_work_t* req = iotjs_pwm_reqwrap_req(req_wrap);                  \
    uv_queue_work(loop, req, iotjs_pwm_##call##_worker,                \
                  iotjs_pwm_after_worker);                             \
  } while (0)


#define PWM_ASYNC_COMMON_WORKER(call, this, jcallback, op)                     \
  do {                                                                         \
    uv_loop_t* loop = iotjs_environment_loop(iotjs_environment_get());         \
    iotjs_pwm_reqwrap_t* req_wrap =                                            \
        iotjs_pwm_reqwrap_create(jcallback, this, op);                         \
    uv_work_t* req = iotjs_pwm_reqwrap_req(req_wrap);                          \
    iotjs_pwm_reqdata_t* req_data = iotjs_pwm_reqwrap_data(req_wrap);          \
    req_data->caller = call;                                                   \
    uv_queue_work(loop, req, iotjs_pwm_common_worker, iotjs_pwm_after_worker); \
  } while (0)


JHANDLER_FUNCTION(PWMConstructor) {
  DJHANDLER_CHECK_THIS(object);
  DJHANDLER_CHECK_ARGS(2, object, function);

  // Create PWM object
  const iotjs_jval_t* jpwm = JHANDLER_GET_THIS(object);
  iotjs_pwm_t* pwm = iotjs_pwm_create(jpwm);
  IOTJS_ASSERT(pwm == iotjs_pwm_instance_from_jval(jpwm));

  const iotjs_jval_t* jconfiguration = JHANDLER_GET_ARG(0, object);
  const iotjs_jval_t* jcallback = JHANDLER_GET_ARG(1, function);

  // Set configuration
  iotjs_pwm_set_configuration(jconfiguration, pwm);

  PWM_ASYNC(open, pwm, jcallback, kPwmOpOpen);
}


JHANDLER_FUNCTION(SetPeriod) {
  JHANDLER_DECLARE_THIS_PTR(pwm, pwm);

  DJHANDLER_CHECK_ARGS(1, number);
  DJHANDLER_CHECK_ARG_IF_EXIST(1, function);

  const iotjs_jval_t* jcallback = JHANDLER_GET_ARG_IF_EXIST(1, function);

  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_pwm_t, pwm);
  _this->period = JHANDLER_GET_ARG(0, number);

  if (jcallback) {
    PWM_ASYNC_COMMON_WORKER(iotjs_pwm_set_period, pwm, jcallback,
                            kPwmOpSetPeriod);
  } else {
    if (!iotjs_pwm_set_period(pwm)) {
      JHANDLER_THROW(COMMON, "PWM SetPeriod Error");
    }
  }

  iotjs_jhandler_return_null(jhandler);
}


JHANDLER_FUNCTION(SetDutyCycle) {
  JHANDLER_DECLARE_THIS_PTR(pwm, pwm);

  DJHANDLER_CHECK_ARGS(1, number);
  DJHANDLER_CHECK_ARG_IF_EXIST(1, function);

  const iotjs_jval_t* jcallback = JHANDLER_GET_ARG_IF_EXIST(1, function);

  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_pwm_t, pwm);
  _this->duty_cycle = JHANDLER_GET_ARG(0, number);

  if (jcallback) {
    PWM_ASYNC_COMMON_WORKER(iotjs_pwm_set_dutycycle, pwm, jcallback,
                            kPwmOpSetDutyCycle);
  } else {
    if (!iotjs_pwm_set_dutycycle(pwm)) {
      JHANDLER_THROW(COMMON, "PWM SetDutyCycle Error");
    }
  }

  iotjs_jhandler_return_null(jhandler);
}


JHANDLER_FUNCTION(SetEnable) {
  JHANDLER_DECLARE_THIS_PTR(pwm, pwm);

  DJHANDLER_CHECK_ARGS(1, boolean);
  DJHANDLER_CHECK_ARG_IF_EXIST(1, function);

  const iotjs_jval_t* jcallback = JHANDLER_GET_ARG_IF_EXIST(1, function);

  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_pwm_t, pwm);
  _this->enable = JHANDLER_GET_ARG(0, boolean);

  if (jcallback) {
    PWM_ASYNC_COMMON_WORKER(iotjs_pwm_set_enable, pwm, jcallback,
                            kPwmOpSetEnable);
  } else {
    if (!iotjs_pwm_set_enable(pwm)) {
      JHANDLER_THROW(COMMON, "PWM SetEnabe Error");
    }
  }

  iotjs_jhandler_return_null(jhandler);
}


JHANDLER_FUNCTION(Close) {
  JHANDLER_DECLARE_THIS_PTR(pwm, pwm);
  DJHANDLER_CHECK_ARG_IF_EXIST(0, function);

  const iotjs_jval_t* jcallback = JHANDLER_GET_ARG_IF_EXIST(0, function);

  if (jcallback) {
    PWM_ASYNC_COMMON_WORKER(iotjs_pwm_close, pwm, jcallback, kPwmOpClose);
  } else {
    if (!iotjs_pwm_close(pwm)) {
      JHANDLER_THROW(COMMON, "PWM Close Error");
    }
  }

  iotjs_jhandler_return_null(jhandler);
}


iotjs_jval_t InitPwm() {
  iotjs_jval_t jpwm_constructor =
      iotjs_jval_create_function_with_dispatch(PWMConstructor);

  iotjs_jval_t jprototype = iotjs_jval_create_object();

  iotjs_jval_set_method(&jprototype, IOTJS_MAGIC_STRING_SETPERIOD, SetPeriod);
  iotjs_jval_set_method(&jprototype, IOTJS_MAGIC_STRING_SETDUTYCYCLE,
                        SetDutyCycle);
  iotjs_jval_set_method(&jprototype, IOTJS_MAGIC_STRING_SETENABLE, SetEnable);
  iotjs_jval_set_method(&jprototype, IOTJS_MAGIC_STRING_CLOSE, Close);

  iotjs_jval_set_property_jval(&jpwm_constructor, IOTJS_MAGIC_STRING_PROTOTYPE,
                               &jprototype);

  iotjs_jval_destroy(&jprototype);

  return jpwm_constructor;
}
