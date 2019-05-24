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

#if defined(__NUTTX__)


#include <nuttx/drivers/pwm.h>
#include <stdlib.h>
#include <sys/ioctl.h>

#include "iotjs_def.h"
#include "iotjs_systemio-nuttx.h"
#include "modules/iotjs_module_pwm.h"


#define PWM_DEVICE_PATH_FORMAT "/dev/pwm%d"
#define PWM_DEVICE_PATH_BUFFER_SIZE 12

struct iotjs_pwm_platform_data_s {
  int device_fd;
};

void iotjs_pwm_create_platform_data(iotjs_pwm_t* pwm) {
  pwm->platform_data = IOTJS_ALLOC(iotjs_pwm_platform_data_t);
  pwm->platform_data->device_fd = -1;
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

  int fd = platform_data->device_fd;
  if (fd < 0) {
    DLOG("%s - file open failed", __func__);
    return false;
  }

  struct pwm_info_s info;

  // Clamp so that the value inverted fits into uint32
  if (pwm->period < 2.33E-10)
    pwm->period = 2.33E-10;
  info.frequency = (uint32_t)(1.0 / pwm->period);

  double duty_value = pwm->duty_cycle * (1 << 16); // 16 bit timer
  if (duty_value > 0xffff)
    duty_value = 0xffff;
  else if (duty_value < 1)
    duty_value = 1;
  info.duty = (ub16_t)duty_value;

  DDDLOG("%s - frequency: %d, duty: %d", __func__, info.frequency, info.duty);

  // Set Pwm info
  if (ioctl(fd, PWMIOC_SETCHARACTERISTICS, (unsigned long)((uintptr_t)&info)) <
      0) {
    return false;
  }

  return true;
}

bool iotjs_pwm_open(iotjs_pwm_t* pwm) {
  int timer = SYSIO_GET_TIMER(pwm->pin);
  char path[PWM_DEVICE_PATH_BUFFER_SIZE] = { 0 };

  if (snprintf(path, PWM_DEVICE_PATH_BUFFER_SIZE, PWM_DEVICE_PATH_FORMAT,
               (timer - 1)) < 0) {
    return false;
  }

  if (access(path, F_OK) != 0) {
    struct pwm_lowerhalf_s* pwm_lowerhalf =
        iotjs_pwm_config_nuttx(timer, pwm->pin);

    DDDLOG("%s - path: %s, timer: %d\n", __func__, path, timer);

    if (pwm_register(path, pwm_lowerhalf) != 0) {
      return false;
    }
  }

  // File open
  iotjs_pwm_platform_data_t* platform_data = pwm->platform_data;
  platform_data->device_fd = open(path, O_RDONLY);
  if (platform_data->device_fd < 0) {
    DLOG("%s - file open failed", __func__);
    return false;
  }

  if (!pwm_set_options(pwm)) {
    return false;
  }

  return true;
}

bool iotjs_pwm_set_period(iotjs_pwm_t* pwm) {
  return pwm_set_options(pwm);
}

bool iotjs_pwm_set_dutycycle(iotjs_pwm_t* pwm) {
  return pwm_set_options(pwm);
}

bool iotjs_pwm_set_enable(iotjs_pwm_t* pwm) {
  iotjs_pwm_platform_data_t* platform_data = pwm->platform_data;

  int fd = platform_data->device_fd;
  if (fd < 0) {
    DLOG("%s - file open failed", __func__);
    return false;
  }

  DDDLOG("%s - enable: %d", __func__, pwm->enable);

  int ret;
  if (pwm->enable) {
    ret = ioctl(fd, PWMIOC_START, 0);
  } else {
    ret = ioctl(fd, PWMIOC_STOP, 0);
  }

  if (ret < 0) {
    DLOG("%s - setEnable failed", __func__);
    return false;
  }

  return true;
}

bool iotjs_pwm_close(iotjs_pwm_t* pwm) {
  iotjs_pwm_platform_data_t* platform_data = pwm->platform_data;

  int fd = platform_data->device_fd;
  if (fd < 0) {
    DLOG("%s - file not opened", __func__);
    return false;
  }

  DDDLOG("%s", __func__);

  // Close file
  close(fd);
  platform_data->device_fd = -1;

  uint32_t timer = SYSIO_GET_TIMER(pwm->pin);
  char path[PWM_DEVICE_PATH_BUFFER_SIZE] = { 0 };
  if (snprintf(path, PWM_DEVICE_PATH_BUFFER_SIZE - 1, PWM_DEVICE_PATH_FORMAT,
               timer) < 0) {
    return false;
  }

  // Release driver
  unregister_driver(path);

  iotjs_gpio_unconfig_nuttx(pwm->pin);

  return true;
}


#endif // __NUTTX__
