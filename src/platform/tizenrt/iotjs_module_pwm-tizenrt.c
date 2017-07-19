/* Copyright 2017-present Samsung Electronics Co., Ltd. and other contributors
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

#if defined(__TIZENRT__)

#include <stdlib.h>
#include <sys/ioctl.h>

#include <iotbus_error.h>
#include <iotbus_pwm.h>

#include "modules/iotjs_module_pwm.h"

static bool iotjs_pwm_set_options(iotjs_pwm_t* pwm) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_pwm_t, pwm);

  iotbus_pwm_context_h ctx = _this->ctx;
  if (ctx == NULL) {
    DLOG("%s - file open failed", __func__);
    return false;
  }

  DDDLOG("%s - period: %d, duty: %d", __func__, (int)_this->period,
         (int)(_this->duty_cycle * 100));

  return iotjs_pwm_set_dutycycle(pwm) && iotjs_pwm_set_period(pwm);
}

void iotjs_pwm_open_worker(uv_work_t* work_req) {
  PWM_WORKER_INIT;
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_pwm_t, pwm);

  _this->ctx = iotbus_pwm_open(0, (int)_this->pin);
  if (_this->ctx == NULL) {
    DLOG("%s - file open failed", __func__);
    req_data->result = false;
    return;
  }

  if (!iotjs_pwm_set_options(pwm)) {
    req_data->result = false;
    return;
  }

  req_data->result = true;
}

bool iotjs_pwm_set_period(iotjs_pwm_t* pwm) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_pwm_t, pwm);

  iotbus_pwm_context_h ctx = _this->ctx;
  if (ctx == NULL) {
    DLOG("%s - file open failed", __func__);
    return false;
  }

  uint32_t period_us = (uint32_t)(1000000 * _this->period);

  return iotbus_pwm_set_period(ctx, period_us) == 0;
}


bool iotjs_pwm_set_dutycycle(iotjs_pwm_t* pwm) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_pwm_t, pwm);

  iotbus_pwm_context_h ctx = _this->ctx;
  if (ctx == NULL) {
    DLOG("%s - file open failed", __func__);
    return false;
  }

  uint32_t duty_cycle_per_cent = (uint32_t)(_this->duty_cycle * 100);

  return iotbus_pwm_set_duty_cycle(ctx, duty_cycle_per_cent) == 0;
}


bool iotjs_pwm_set_enable(iotjs_pwm_t* pwm) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_pwm_t, pwm);

  iotbus_pwm_context_h ctx = _this->ctx;
  if (ctx == NULL) {
    DLOG("%s - file open failed", __func__);
    return false;
  }

  DDDLOG("%s - enable: %d", __func__, _this->enable);

  int ret;
  if (_this->enable) {
    ret = iotbus_pwm_set_enabled(ctx, IOTBUS_PWM_ENABLE);
  } else {
    ret = iotbus_pwm_set_enabled(ctx, IOTBUS_PWM_DISABLE);
  }

  if (ret < 0) {
    DLOG("%s - setEnable failed", __func__);
    return false;
  }

  return true;
}

bool iotjs_pwm_close(iotjs_pwm_t* pwm) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_pwm_t, pwm);

  iotbus_pwm_context_h ctx = _this->ctx;
  if (ctx == NULL) {
    DLOG("%s - file not opened", __func__);
    return false;
  }

  DDDLOG("%s", __func__);

  iotbus_pwm_close(ctx);
  _this->ctx = NULL;

  return true;
}


#endif // __TIZENRT__
