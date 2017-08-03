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

#ifndef IOTJS_MODULE_PWM_LINUX_GENERAL_INL_H
#define IOTJS_MODULE_PWM_LINUX_GENERAL_INL_H


#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "iotjs_systemio-linux.h"
#include "modules/iotjs_module_pwm.h"


// Generic PWM implementation for linux.


#define PWM_INTERFACE "/sys/class/pwm/pwmchip%d/"
#define PWM_PIN_INTERFACE "pwm%d/"
#define PWM_PIN_FORMAT PWM_INTERFACE PWM_PIN_INTERFACE
#define PWM_EXPORT PWM_INTERFACE "export"
#define PWM_UNEXPORT PWM_INTERFACE "unexport"
#define PWM_PIN_DUTYCYCLE "duty_cycle"
#define PWM_PIN_PERIOD "period"
#define PWM_PIN_ENABlE "enable"

#define PWM_PATH_BUFFER_SIZE 64
#define PWM_VALUE_BUFFER_SIZE 32


// Generate device path for specified PWM device.
// The path may include node suffix if passed ('enable', 'period', 'duty_cycle')
// Pointer to a allocated string is returned, or null in case of error.
// If PWM_PIN_FORMAT format results in an empty string,
// NULL is returned (and fileName is ignored).
static char* generate_device_subpath(iotjs_string_t* device,
                                     const char* fileName) {
  char* devicePath = NULL;
  // Do not print anything, only calculate resulting string length.
  size_t prefixSize = iotjs_string_size(device);
  if (prefixSize > 0) {
    size_t suffixSize = fileName ? strlen(fileName) : 0;
    devicePath = iotjs_buffer_allocate(prefixSize + suffixSize + 1);
    if (devicePath) {
      // Do not need to check bounds, the buffer is of exact required size.
      memcpy(devicePath, iotjs_string_data(device), prefixSize);
      memcpy(devicePath + prefixSize, fileName, suffixSize);
      devicePath[prefixSize + suffixSize] = 0;
    }
  }
  return devicePath;
}

// Limit period to [0..1]s
static double adjust_period(double period) {
  if (period < 0) {
    return 0.0;
  } else if (period > 1) {
    return 1.0;
  } else {
    return period;
  }
}


void iotjs_pwm_open_worker(uv_work_t* work_req) {
  PWM_WORKER_INIT;
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_pwm_t, pwm);

  char path[PWM_PATH_BUFFER_SIZE] = { 0 };
  if (snprintf(path, PWM_PATH_BUFFER_SIZE, PWM_PIN_FORMAT, _this->chip,
               _this->pin) < 0) {
    req_data->result = false;
    return;
  }

  _this->device = iotjs_string_create_with_size(path, strlen(path));

  // See if the PWM is already opened.
  if (!iotjs_systemio_check_path(path)) {
    // Write exporting PWM path
    char export_path[PWM_PATH_BUFFER_SIZE] = { 0 };
    snprintf(export_path, PWM_PATH_BUFFER_SIZE, PWM_EXPORT, _this->chip);

    const char* created_files[] = { PWM_PIN_DUTYCYCLE, PWM_PIN_PERIOD,
                                    PWM_PIN_ENABlE };
    int created_files_length = sizeof(created_files) / sizeof(created_files[0]);
    if (!iotjs_systemio_device_open(export_path, _this->pin, path,
                                    created_files, created_files_length)) {
      req_data->result = false;
      return;
    }
  }

  // Set options.
  if (_this->period >= 0) {
    if (!iotjs_pwm_set_period(pwm)) {
      req_data->result = false;
      return;
    }
    if (_this->duty_cycle >= 0) {
      if (!iotjs_pwm_set_dutycycle(pwm)) {
        req_data->result = false;
        return;
      }
    }
  }

  DDDLOG("%s - path: %s", __func__, path);

  req_data->result = true;
}


bool iotjs_pwm_set_period(iotjs_pwm_t* pwm) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_pwm_t, pwm);

  bool result = false;
  if (isfinite(_this->period) && _this->period >= 0.0) {
    char* devicePath = generate_device_subpath(&_this->device, PWM_PIN_PERIOD);
    if (devicePath) {
      // Linux API uses nanoseconds, thus 1E9
      unsigned int value = (unsigned)(adjust_period(_this->period) * 1.E9);
      DDDLOG("%s - path: %s, value: %fs", __func__, devicePath, 1.E-9 * value);
      char buf[PWM_VALUE_BUFFER_SIZE];
      if (snprintf(buf, sizeof(buf), "%d", value) > 0) {
        result = iotjs_systemio_open_write_close(devicePath, buf);
      }
      iotjs_buffer_release(devicePath);
    }
  }
  return result;
}


bool iotjs_pwm_set_dutycycle(iotjs_pwm_t* pwm) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_pwm_t, pwm);

  bool result = false;
  double dutyCycle = _this->duty_cycle;
  if (isfinite(_this->period) && _this->period >= 0.0 && isfinite(dutyCycle) &&
      0.0 <= dutyCycle && dutyCycle <= 1.0) {
    char* devicePath =
        generate_device_subpath(&_this->device, PWM_PIN_DUTYCYCLE);
    if (devicePath) {
      double period = adjust_period(_this->period);
      // Linux API uses nanoseconds, thus 1E9
      unsigned dutyCycleValue = (unsigned)(period * _this->duty_cycle * 1E9);

      DDDLOG("%s - path: %s, value: %d\n", __func__, devicePath,
             dutyCycleValue);
      char buf[PWM_VALUE_BUFFER_SIZE];
      if (snprintf(buf, sizeof(buf), "%d", dutyCycleValue) < 0) {
        return false;
      }

      result = iotjs_systemio_open_write_close(devicePath, buf);
      iotjs_buffer_release(devicePath);
    }
  }
  return result;
}


bool iotjs_pwm_set_enable(iotjs_pwm_t* pwm) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_pwm_t, pwm);

  bool result = false;

  char* devicePath = generate_device_subpath(&_this->device, PWM_PIN_ENABlE);
  if (devicePath) {
    char value[4];
    if (snprintf(value, sizeof(value), "%d", _this->enable) < 0) {
      return false;
    }

    DDDLOG("%s - path: %s, set: %d\n", __func__, devicePath, _this->enable);
    char buf[PWM_VALUE_BUFFER_SIZE];
    if (snprintf(buf, sizeof(buf), "%d", _this->enable) < 0) {
      return false;
    }

    result = iotjs_systemio_open_write_close(devicePath, buf);
    iotjs_buffer_release(devicePath);
  }
  return result;
}


bool iotjs_pwm_close(iotjs_pwm_t* pwm) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_pwm_t, pwm);

  char path[PWM_PATH_BUFFER_SIZE] = { 0 };
  if (snprintf(path, PWM_PATH_BUFFER_SIZE, PWM_PIN_FORMAT, _this->chip,
               _this->pin) < 0) {
    return false;
  }

  if (iotjs_systemio_check_path(path)) {
    // Write exporting pin path
    char unexport_path[PWM_PATH_BUFFER_SIZE] = { 0 };
    if (snprintf(unexport_path, PWM_PATH_BUFFER_SIZE, PWM_UNEXPORT,
                 _this->chip) < 0) {
      return false;
    }

    iotjs_systemio_device_close(unexport_path, _this->pin);
  }

  DDDLOG("%s- path: %s", __func__, path);

  return true;
}


#endif /* IOTJS_MODULE_PWM_LINUX_GENERAL_INL_H */
