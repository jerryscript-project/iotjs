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

#if defined(__NUTTX__) && defined(TARGET_BOARD_STM32F7NUCLEO)

#include "iotjs_systemio-nuttx.h"
#include "stm32_gpio.h"

#include "iotjs_def.h"
#include "modules/iotjs_module_stm32f7nucleo.h"

#if ENABLE_MODULE_ADC
static void iotjs_pin_initialize_adc(jerry_value_t jobj) {
  unsigned int number_bit;

// ADC pin name is "ADC(number)_(timer)".
#define SET_ADC_CONSTANT(number, timer)        \
  number_bit = (GPIO_ADC##number##_IN##timer); \
  number_bit |= (ADC_NUMBER(number));          \
  number_bit |= (SYSIO_TIMER_NUMBER(timer));   \
  iotjs_jval_set_property_number(jobj, "ADC" #number "_" #timer, number_bit);

#define SET_ADC_CONSTANT_NUMBER(number) SET_ADC_CONSTANT(number, 3);

  SET_ADC_CONSTANT_NUMBER(1);

#undef SET_ADC_CONSTANT_NUMBER
#undef SET_ADC_CONSTANT
}
#endif /* ENABLE_MODULE_ADC */

#if ENABLE_MODULE_GPIO

static void iotjs_pin_initialize_gpio(jerry_value_t jobj) {
// Set GPIO pin from configuration bits of nuttx.
// GPIO pin name is "P(port)(pin)".
#define SET_GPIO_CONSTANT(port, pin)                   \
  iotjs_jval_set_property_number(jobj, "P" #port #pin, \
                                 (GPIO_PORT##port | GPIO_PIN##pin));

#define SET_GPIO_CONSTANT_PORT(port) \
  SET_GPIO_CONSTANT(port, 0);        \
  SET_GPIO_CONSTANT(port, 1);        \
  SET_GPIO_CONSTANT(port, 2);        \
  SET_GPIO_CONSTANT(port, 3);        \
  SET_GPIO_CONSTANT(port, 4);        \
  SET_GPIO_CONSTANT(port, 5);        \
  SET_GPIO_CONSTANT(port, 6);        \
  SET_GPIO_CONSTANT(port, 7);        \
  SET_GPIO_CONSTANT(port, 8);        \
  SET_GPIO_CONSTANT(port, 9);        \
  SET_GPIO_CONSTANT(port, 10);       \
  SET_GPIO_CONSTANT(port, 11);       \
  SET_GPIO_CONSTANT(port, 12);       \
  SET_GPIO_CONSTANT(port, 13);       \
  SET_GPIO_CONSTANT(port, 14);       \
  SET_GPIO_CONSTANT(port, 15);

  SET_GPIO_CONSTANT_PORT(A);
  SET_GPIO_CONSTANT_PORT(B);
  SET_GPIO_CONSTANT_PORT(C);
  SET_GPIO_CONSTANT_PORT(D);
  SET_GPIO_CONSTANT_PORT(E);

  SET_GPIO_CONSTANT(H, 0);
  SET_GPIO_CONSTANT(H, 1);

#undef SET_GPIO_CONSTANT_PORT
#undef SET_GPIO_CONSTANT
}

#endif /* ENABLE_MODULE_GPIO */


#if ENABLE_MODULE_PWM

static void iotjs_pin_initialize_pwm(jerry_value_t jobj) {
  unsigned int timer_bit;

// Set PWM pin from configuration bits of nuttx.
// PWM pin name is "PWM(timer).CH(channel)_(n)".
#define SET_GPIO_CONSTANT(timer, channel, order)                        \
  timer_bit = (GPIO_TIM##timer##_CH##channel##OUT_##order);             \
  timer_bit |= (SYSIO_TIMER_NUMBER(timer));                             \
  iotjs_jval_set_property_number(jtim##timer, "CH" #channel "_" #order, \
                                 timer_bit);

#define SET_GPIO_CONSTANT_CHANNEL(timer, channel) \
  SET_GPIO_CONSTANT(timer, channel, 1);

#define SET_GPIO_CONSTANT_TIM(timer)                 \
  jerry_value_t jtim##timer = jerry_create_object(); \
  iotjs_jval_set_property_jval(jobj, "PWM" #timer, jtim##timer);


#define SET_GPIO_CONSTANT_TIM_1(timer) \
  SET_GPIO_CONSTANT_TIM(timer);        \
  SET_GPIO_CONSTANT_CHANNEL(timer, 1);

  SET_GPIO_CONSTANT_TIM_1(1);
  jerry_release_value(jtim1);

  SET_GPIO_CONSTANT_TIM_1(2);
  jerry_release_value(jtim2);

  SET_GPIO_CONSTANT_TIM_1(3);
  jerry_release_value(jtim3);

  SET_GPIO_CONSTANT_TIM_1(4);
  jerry_release_value(jtim4);

#undef SET_GPIO_CONSTANT_TIM_1
#undef SET_GPIO_CONSTANT_TIM_2
#undef SET_GPIO_CONSTANT_TIM
#undef SET_GPIO_CONSTANT_CHANNEL
#undef SET_GPIO_CONSTANT
}

#endif /* ENABLE_MODULE_PWM */


void iotjs_stm32f7nucleo_pin_initialize(jerry_value_t jobj) {
  jerry_value_t jpin = jerry_create_object();
  iotjs_jval_set_property_jval(jobj, "pin", jpin);

#if ENABLE_MODULE_ADC
  iotjs_pin_initialize_adc(jpin);
#endif /* ENABLE_MODULE_ADC */

#if ENABLE_MODULE_GPIO
  iotjs_pin_initialize_gpio(jpin);
#endif /* ENABLE_MODULE_GPIO */

#if ENABLE_MODULE_PWM
  iotjs_pin_initialize_pwm(jpin);
#endif /* ENABLE_MODULE_PWM */

  jerry_release_value(jpin);
}


#endif // __NUTTX__
