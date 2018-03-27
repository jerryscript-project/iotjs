/* Copyright 2018-present Samsung Electronics Co., Ltd. and other contributors
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


#include <peripheral_io.h>

#include "modules/iotjs_module_pwm.h"

struct iotjs_pwm_platform_data_s {
  peripheral_pwm_h pwm_h;
};

void iotjs_pwm_create_platform_data(iotjs_pwm_t* pwm) {
  pwm->platform_data = IOTJS_ALLOC(iotjs_pwm_platform_data_t);
  pwm->platform_data->pwm_h = NULL;
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

  if (platform_data->pwm_h == NULL) {
    DLOG("%s - cannot set options", __func__);
    return false;
  }

  return iotjs_pwm_set_period(pwm) && iotjs_pwm_set_dutycycle(pwm);
}

bool iotjs_pwm_open(iotjs_pwm_t* pwm) {
  iotjs_pwm_platform_data_t* platform_data = pwm->platform_data;

  int ret = peripheral_pwm_open(0, (int)pwm->pin, &platform_data->pwm_h);
  if (ret != PERIPHERAL_ERROR_NONE) {
    DLOG("%s : cannot open(%d)", __func__, ret);
    return false;
  }

  return pwm_set_options(pwm);
}

#define PWM_METHOD_HEADER                                        \
  IOTJS_ASSERT(pwm && pwm->platform_data);                       \
  iotjs_pwm_platform_data_t* platform_data = pwm->platform_data; \
  if (platform_data->pwm_h == NULL) {                            \
    DLOG("%s: PWM is not opened", __func__);                     \
    return false;                                                \
  }

bool iotjs_pwm_set_period(iotjs_pwm_t* pwm) {
  PWM_METHOD_HEADER

  uint32_t period_ns = pwm->period * 1E9;
  int ret = peripheral_pwm_set_period(platform_data->pwm_h, period_ns);
  if (ret != PERIPHERAL_ERROR_NONE) {
    DLOG("%s : cannot set period(%d)", __func__, ret);
    return false;
  }

  return true;
}

bool iotjs_pwm_set_dutycycle(iotjs_pwm_t* pwm) {
  PWM_METHOD_HEADER

  uint32_t duty_cycle_ns = pwm->period * pwm->duty_cycle * 1E9;
  int ret = peripheral_pwm_set_duty_cycle(platform_data->pwm_h, duty_cycle_ns);
  if (ret != PERIPHERAL_ERROR_NONE) {
    DLOG("%s : cannot set duty-cycle(%d)", __func__, ret);
    return false;
  }

  return true;
}

bool iotjs_pwm_set_enable(iotjs_pwm_t* pwm) {
  PWM_METHOD_HEADER

  int ret = peripheral_pwm_set_enabled(platform_data->pwm_h, pwm->enable);
  if (ret != PERIPHERAL_ERROR_NONE) {
    DLOG("%s : cannot set enable(%d)", __func__, ret);
    return false;
  }

  return true;
}

bool iotjs_pwm_close(iotjs_pwm_t* pwm) {
  PWM_METHOD_HEADER

  int ret = peripheral_pwm_close(platform_data->pwm_h);
  if (ret != PERIPHERAL_ERROR_NONE) {
    DLOG("%s : cannot close(%d)", __func__, ret);
    return false;
  }
  platform_data->pwm_h = NULL;

  return true;
}
