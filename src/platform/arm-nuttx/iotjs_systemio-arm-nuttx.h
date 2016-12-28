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

#ifndef IOTJS_SYSTEMIO_ARM_NUTTX_H
#define IOTJS_SYSTEMIO_ARM_NUTTX_H


void iotjs_gpio_unconfig_nuttx(int pin);


#if ENABLE_MODULE_ADC || ENABLE_MODULE_PWM

#define SYSIO_TIMER_PIN_SHIFT 21 /* Bits 21-24: Timer number */
#define SYSIO_TIMER_PIN_MASK 15
#define SYSIO_TIMER_NUMBER(n) ((n) << SYSIO_TIMER_PIN_SHIFT)
#define SYSIO_GET_TIMER(n) \
  (((n) >> SYSIO_TIMER_PIN_SHIFT) & SYSIO_TIMER_PIN_MASK)

#endif /* ENABLE_MODULE_ADC || ENABLE_MODULE_PWM */


#if ENABLE_MODULE_ADC

#include <nuttx/analog/adc.h>

#define ADC_NUMBER_SHIFT 25 /* Bits 25-26: ADC number */
#define ADC_NUMBER_MASK 3
#define ADC_NUMBER(n) ((n) << ADC_NUMBER_SHIFT)
#define ADC_GET_NUMBER(n) (((n) >> ADC_NUMBER_SHIFT) & ADC_NUMBER_MASK)

struct adc_dev_s* iotjs_adc_config_nuttx(int number, int timer, int pin);

#endif /* ENABLE_MODULE_ADC */


#if ENABLE_MODULE_I2C

#include <nuttx/i2c/i2c_master.h>

struct i2c_master_s* iotjs_i2c_config_nuttx(int port);
int iotjs_i2c_unconfig_nuttx(struct i2c_master_s* i2c);

#endif /* ENABLE_MODULE_I2C */


#if ENABLE_MODULE_PWM

#include <nuttx/drivers/pwm.h>

struct pwm_lowerhalf_s* iotjs_pwm_config_nuttx(int timer, int pin);

#endif /* ENABLE_MODULE_PWM */


#endif /* IOTJS_SYSTEMIO_ARM_NUTTX_H */
