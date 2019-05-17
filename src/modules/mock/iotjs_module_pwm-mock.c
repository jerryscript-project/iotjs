/* Copyright 2019-present Samsung Electronics Co., Ltd. and other contributors
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

#include "modules/iotjs_module_pwm.h"

struct iotjs_pwm_platform_data_s {
  bool is_open;
};

void iotjs_pwm_create_platform_data(iotjs_pwm_t* pwm) {
  pwm->platform_data = IOTJS_ALLOC(iotjs_pwm_platform_data_t);
}

void iotjs_pwm_destroy_platform_data(iotjs_pwm_platform_data_t* pdata) {
  IOTJS_RELEASE(pdata);
}

jerry_value_t iotjs_pwm_set_platform_config(iotjs_pwm_t* pwm,
                                            const jerry_value_t jconfig) {
  IOTJS_UNUSED(pwm);
  IOTJS_UNUSED(jconfig);
  if (jerry_value_is_object(jconfig)) {
    return jerry_create_undefined();
  } else {
    return JS_CREATE_ERROR(COMMON, "Config must be an object.");
  }
}

bool iotjs_pwm_open(iotjs_pwm_t* pwm) {
  iotjs_pwm_platform_data_t* platform_data = pwm->platform_data;
  IOTJS_ASSERT(platform_data != NULL);

  if (platform_data->is_open) {
    return false; // pin is open already
  }

  platform_data->is_open = true;
  return true;
}

bool iotjs_pwm_set_period(iotjs_pwm_t* pwm) {
  iotjs_pwm_platform_data_t* platform_data = pwm->platform_data;
  IOTJS_ASSERT(platform_data != NULL);
  return true;
}

bool iotjs_pwm_set_dutycycle(iotjs_pwm_t* pwm) {
  iotjs_pwm_platform_data_t* platform_data = pwm->platform_data;
  IOTJS_ASSERT(platform_data != NULL);
  return true;
}

bool iotjs_pwm_set_enable(iotjs_pwm_t* pwm) {
  iotjs_pwm_platform_data_t* platform_data = pwm->platform_data;
  IOTJS_ASSERT(platform_data != NULL);
  return true;
}

bool iotjs_pwm_close(iotjs_pwm_t* pwm) {
  iotjs_pwm_platform_data_t* platform_data = pwm->platform_data;
  IOTJS_ASSERT(platform_data != NULL);

  if (!platform_data->is_open) {
    return false; // pin is not open
  }

  platform_data->is_open = false;
  return true;
}
