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
#include "iotjs_systemio-arm-nuttx.h"
#include "module/iotjs_module_pwm.h"


#define PWM_DEVICE_PATH_FORMAT "/dev/pwm%d"
#define PWM_DEVICE_PATH_BUFFER_SIZE 12
#define PWM_MAX_PINNO 17


static int pwm_handler[PWM_MAX_PINNO];


static void iotjs_pwm_get_path(char* buffer, int timer) {
  // Create PWM device path
  snprintf(buffer, PWM_DEVICE_PATH_BUFFER_SIZE - 1, PWM_DEVICE_PATH_FORMAT,
           timer);
}


static bool iotjs_pwm_set_options(int timer, iotjs_pwm_reqdata_t* req_data) {
  int fd = pwm_handler[timer];
  if (fd < 0) {
    DDLOG("PWM - file open failed");
    return false;
  }

  struct pwm_info_s info;
  char path[PWM_DEVICE_PATH_BUFFER_SIZE] = { 0 };
  iotjs_pwm_get_path(path, timer);

  info.frequency = req_data->period;
  info.duty = (req_data->duty_cycle > 100) ? 100 : req_data->duty_cycle;
  info.duty = ((uint32_t)info.duty << 16) / 100; // 16bit timer

  DDLOG("PWM Set Options - path: %s, frequency: %d, duty: %d", path,
        info.frequency, info.duty);

  // Set Pwm info
  if (ioctl(fd, PWMIOC_SETCHARACTERISTICS, (unsigned long)((uintptr_t)&info)) <
      0) {
    return false;
  }

  return true;
}


void iotjs_pwm_initialize() {
  memset(pwm_handler, 0xFF, PWM_MAX_PINNO * sizeof(int));
}


#define PWM_WORKER_INIT_TEMPLATE                                            \
  iotjs_pwm_reqwrap_t* req_wrap = iotjs_pwm_reqwrap_from_request(work_req); \
  iotjs_pwm_reqdata_t* req_data = iotjs_pwm_reqwrap_data(req_wrap);


void ExportWorker(uv_work_t* work_req) {
  PWM_WORKER_INIT_TEMPLATE;

  uint32_t pin = req_data->pin;
  uint32_t timer = SYSIO_GET_TIMER(pin);
  char path[PWM_DEVICE_PATH_BUFFER_SIZE] = { 0 };
  iotjs_pwm_get_path(path, timer);
  struct pwm_lowerhalf_s* pwm = iotjs_pwm_config_nuttx(timer, pin);

  DDLOG("PWM Export - path: %s, timer: %d\n", path, timer);

  if (pwm_register(path, pwm) != 0) {
    req_data->result = kPwmErrExport;
    return;
  }

  // File open
  pwm_handler[timer] = open(path, O_RDONLY);
  if (pwm_handler[timer] < 0) {
    req_data->result = kPwmErrExport;
    return;
  }

  // Set options
  if (req_data->period < 0)
    req_data->period = 1;

  if (req_data->duty_cycle < 0)
    req_data->duty_cycle = 1;

  if (!iotjs_pwm_set_options(timer, req_data)) {
    req_data->result = kPwmErrWrite;
  }

  req_data->result = kPwmErrOk;
}


void SetPeriodWorker(uv_work_t* work_req) {
  IOTJS_ASSERT(!"Not implemented");
}


void SetFrequencyWorker(uv_work_t* work_req) {
  PWM_WORKER_INIT_TEMPLATE;

  uint32_t timer = SYSIO_GET_TIMER(req_data->pin);

  if (!iotjs_pwm_set_options(timer, req_data)) {
    req_data->result = kPwmErrWrite;
  }

  req_data->result = kPwmErrOk;
}


void SetDutyCycleWorker(uv_work_t* work_req) {
  PWM_WORKER_INIT_TEMPLATE;

  uint32_t timer = SYSIO_GET_TIMER(req_data->pin);

  if (!iotjs_pwm_set_options(timer, req_data)) {
    req_data->result = kPwmErrWrite;
  }

  req_data->result = kPwmErrOk;
}


void SetEnableWorker(uv_work_t* work_req) {
  PWM_WORKER_INIT_TEMPLATE;

  uint32_t timer = SYSIO_GET_TIMER(req_data->pin);
  int fd = pwm_handler[timer];
  if (fd < 0) {
    DDLOG("PWM - file open failed");
    req_data->result = kPwmErrWrite;
    return;
  }

  DDLOG("PWM setEnable - timer: %d, enable: %d", timer, req_data->enable);

  int ret;
  if (req_data->enable) {
    ret = ioctl(fd, PWMIOC_START, 0);
  } else {
    ret = ioctl(fd, PWMIOC_STOP, 0);
  }

  if (ret < 0) {
    DDLOG("PWM - setEnable failed - timer: %d", timer);
    req_data->result = kPwmErrWrite;
    return;
  }

  req_data->result = kPwmErrOk;
}


void UnexportWorker(uv_work_t* work_req) {
  PWM_WORKER_INIT_TEMPLATE;

  uint32_t timer = SYSIO_GET_TIMER(req_data->pin);
  int fd = pwm_handler[timer];
  if (fd < 0) {
    DDLOG("PWM - file not opened");
    req_data->result = kPwmErrWrite;
    return;
  }

  DDLOG("PWM unexport - timer: %d", timer);

  // Close file
  close(fd);
  pwm_handler[timer] = -1;

  char path[PWM_DEVICE_PATH_BUFFER_SIZE] = { 0 };
  iotjs_pwm_get_path(path, timer);

  // Release driver
  unregister_driver(path);

  iotjs_gpio_unconfig_nuttx(req_data->pin);

  req_data->result = kPwmErrOk;
}


#endif // __NUTTX__
