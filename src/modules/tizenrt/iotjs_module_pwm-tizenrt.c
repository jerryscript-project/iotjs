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

struct iotjs_pwm_platform_data_s {
  iotbus_pwm_context_h ctx;
};

void iotjs_pwm_create_platform_data(iotjs_pwm_t* pwm) {
  pwm->platform_data = IOTJS_ALLOC(iotjs_pwm_platform_data_t);
}

void iotjs_pwm_destroy_platform_data(iotjs_pwm_platform_data_t* pdata) {
  IOTJS_RELEASE(pdata);
}

jerry_value_t iotjs_pwm_set_platform_config(iotjs_pwm_t* pwm,
                                            const jerry_value_t jconfig) {
  return jerry_create_undefined();
}

static bool pwm_set_options(iotjs_pwm_t* pwm) {
  iotjs_pwm_platform_data_t* platform_data = pwm->platform_data;

  if (platform_data->ctx == NULL) {
    DLOG("%s - file open failed", __func__);
    return false;
  }

  DDDLOG("%s - period: %d, duty: %d", __func__, (int)pwm->period,
         (int)(pwm->duty_cycle * 100));

  return iotjs_pwm_set_dutycycle(pwm) && iotjs_pwm_set_period(pwm);
}

bool iotjs_pwm_open(iotjs_pwm_t* pwm) {
  iotjs_pwm_platform_data_t* platform_data = pwm->platform_data;

  platform_data->ctx = iotbus_pwm_open(0, (int)pwm->pin);
  if (platform_data->ctx == NULL) {
    DLOG("%s - file open failed", __func__);
    return false;
  }

  if (!pwm_set_options(pwm)) {
    return false;
  }

  return true;
}

bool iotjs_pwm_set_period(iotjs_pwm_t* pwm) {
  iotjs_pwm_platform_data_t* platform_data = pwm->platform_data;

  iotbus_pwm_context_h ctx = platform_data->ctx;
  if (ctx == NULL) {
    DLOG("%s - file open failed", __func__);
    return false;
  }

  uint32_t period_us = (uint32_t)(1000000 * pwm->period);

  return iotbus_pwm_set_period(ctx, period_us) == 0;
}

bool iotjs_pwm_set_dutycycle(iotjs_pwm_t* pwm) {
  iotjs_pwm_platform_data_t* platform_data = pwm->platform_data;

  iotbus_pwm_context_h ctx = platform_data->ctx;
  if (ctx == NULL) {
    DLOG("%s - file open failed", __func__);
    return false;
  }

  uint32_t duty_cycle_per_cent = (uint32_t)(pwm->duty_cycle * 100);

  return iotbus_pwm_set_duty_cycle(ctx, duty_cycle_per_cent) == 0;
}

bool iotjs_pwm_set_enable(iotjs_pwm_t* pwm) {
  iotjs_pwm_platform_data_t* platform_data = pwm->platform_data;

  iotbus_pwm_context_h ctx = platform_data->ctx;
  if (ctx == NULL) {
    DLOG("%s - file open failed", __func__);
    return false;
  }

  DDDLOG("%s - enable: %d", __func__, pwm->enable);

  int ret;
  if (pwm->enable) {
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
  iotjs_pwm_platform_data_t* platform_data = pwm->platform_data;

  iotbus_pwm_context_h ctx = platform_data->ctx;
  if (ctx == NULL) {
    DLOG("%s - file not opened", __func__);
    return false;
  }

  DDDLOG("%s", __func__);

  iotbus_pwm_close(ctx);
  platform_data->ctx = NULL;

  return true;
}


#endif // __TIZENRT__
