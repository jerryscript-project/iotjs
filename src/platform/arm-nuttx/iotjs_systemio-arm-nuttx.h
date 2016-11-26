/* Copyright 2016 Samsung Electronics Co., Ltd.
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

#ifndef IOTJS_SYSTEMIO_ARM_NUTTX_H
#define IOTJS_SYSTEMIO_ARM_NUTTX_H

#if ENABLE_MODULE_PWM

#include <nuttx/drivers/pwm.h>

#define PWM_DEVICE_PATH_FORMAT "/dev/pwm%d"
#define PWM_DEVICE_PATH_BUFFER_SIZE 12

struct pwm_lowerhalf_s* iotjs_pwm_config_nuttx(int timer, int pin);
void iotjs_pwm_unconfig_nuttx(int pin);

#endif /* ENABLE_MODULE_PWM */

#endif /* IOTJS_SYSTEMIO_ARM_NUTTX_H */
