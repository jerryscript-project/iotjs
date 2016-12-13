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

#if defined(__NUTTX__) && TARGET_BOARD == STM32F4DIS


#include "iotjs_def.h"
#include "module/iotjs_module_stm32f4dis.h"
#include "iotjs_systemio-arm-nuttx.h"
#include "stm32_gpio.h"


#if ENABLE_MODULE_ADC
static void iotjs_pin_initialize_adc(const iotjs_jval_t* jobj) {
  unsigned int number_bit;

// ADC pin name is "ADC.(number)_(timer)".
#define SET_ADC_CONSTANT(number, timer)        \
  number_bit = (GPIO_ADC##number##_IN##timer); \
  number_bit |= (ADC_NUMBER(number));          \
  number_bit |= (SYSIO_TIMER_NUMBER(timer));   \
  iotjs_jval_set_property_number(jobj, "ADC" #number "_" #timer, number_bit);

#define SET_ADC_CONSTANT_NUMBER(number) \
  SET_ADC_CONSTANT(number, 0);          \
  SET_ADC_CONSTANT(number, 1);          \
  SET_ADC_CONSTANT(number, 2);          \
  SET_ADC_CONSTANT(number, 3);          \
  SET_ADC_CONSTANT(number, 4);          \
  SET_ADC_CONSTANT(number, 5);          \
  SET_ADC_CONSTANT(number, 6);          \
  SET_ADC_CONSTANT(number, 7);          \
  SET_ADC_CONSTANT(number, 8);          \
  SET_ADC_CONSTANT(number, 9);          \
  SET_ADC_CONSTANT(number, 10);         \
  SET_ADC_CONSTANT(number, 11);         \
  SET_ADC_CONSTANT(number, 12);         \
  SET_ADC_CONSTANT(number, 13);         \
  SET_ADC_CONSTANT(number, 14);         \
  SET_ADC_CONSTANT(number, 15);

  SET_ADC_CONSTANT_NUMBER(1);
  SET_ADC_CONSTANT_NUMBER(2);
  SET_ADC_CONSTANT_NUMBER(3);

#undef SET_ADC_CONSTANT_NUMBER
#undef SET_ADC_CONSTANT
}
#endif /* ENABLE_MODULE_ADC */


#if ENABLE_MODULE_GPIO

static void iotjs_pin_initialize_gpio(const iotjs_jval_t* jobj) {
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

static void iotjs_pin_initialize_pwm(const iotjs_jval_t* jobj) {
  unsigned int timer_bit;

// Set PWM pin from configuration bits of nuttx.
// PWM pin name is "PWM(timer).CH(channel)_(n)".
#define SET_GPIO_CONSTANT(timer, channel, order)                         \
  timer_bit = (GPIO_TIM##timer##_CH##channel##OUT_##order);              \
  timer_bit |= (SYSIO_TIMER_NUMBER(timer));                              \
  iotjs_jval_set_property_number(&jtim##timer, "CH" #channel "_" #order, \
                                 timer_bit);

#define SET_GPIO_CONSTANT_CHANNEL(timer, channel) \
  SET_GPIO_CONSTANT(timer, channel, 1);           \
  SET_GPIO_CONSTANT(timer, channel, 2);

#define SET_GPIO_CONSTANT_TIM(timer)                     \
  iotjs_jval_t jtim##timer = iotjs_jval_create_object(); \
  iotjs_jval_set_property_jval(jobj, "PWM" #timer, &jtim##timer);

#define SET_GPIO_CONSTANT_TIM_1(timer) \
  SET_GPIO_CONSTANT_TIM(timer);        \
  SET_GPIO_CONSTANT_CHANNEL(timer, 1); \
  iotjs_jval_destroy(&jtim##timer);

#define SET_GPIO_CONSTANT_TIM_2(timer) \
  SET_GPIO_CONSTANT_TIM(timer);        \
  SET_GPIO_CONSTANT_CHANNEL(timer, 1); \
  SET_GPIO_CONSTANT_CHANNEL(timer, 2); \
  iotjs_jval_destroy(&jtim##timer);

#define SET_GPIO_CONSTANT_TIM_4(timer) \
  SET_GPIO_CONSTANT_TIM(timer);        \
  SET_GPIO_CONSTANT_CHANNEL(timer, 1); \
  SET_GPIO_CONSTANT_CHANNEL(timer, 2); \
  SET_GPIO_CONSTANT_CHANNEL(timer, 3); \
  SET_GPIO_CONSTANT_CHANNEL(timer, 4); \
  iotjs_jval_destroy(&jtim##timer);

  SET_GPIO_CONSTANT_TIM_4(1); // PA8, PE9, PA9, PE11, PA10, PE13, PA11, PE14
  SET_GPIO_CONSTANT_TIM_4(2); // PA0, PA15, PA1, PB3, PA2, PB10, PA3, PB11
  iotjs_jval_set_property_number(&jtim2, "CH1_3", GPIO_TIM2_CH1OUT_3); // PA5
  SET_GPIO_CONSTANT_TIM_4(3); // PA6, PB4, PA7, PB5, PB0, PC8, PB1, PC9
  iotjs_jval_set_property_number(&jtim3, "CH1_3", GPIO_TIM3_CH1OUT_3); // PC6
  iotjs_jval_set_property_number(&jtim3, "CH2_3", GPIO_TIM3_CH2OUT_3); // PC7
  SET_GPIO_CONSTANT_TIM_4(4);  // PB6, PD12, PB7, PD13, PB8, PD14, PB9, PD15
  SET_GPIO_CONSTANT_TIM_4(5);  // PA0, PH10, PA1, PH11, PA2, PH12, PA3, PI0
  SET_GPIO_CONSTANT_TIM_4(8);  // PC6, PI5, PC7, PI6, PC8, PI7, PC9, PI2
  SET_GPIO_CONSTANT_TIM_2(9);  // PA2, PE5, PA3, PE6
  SET_GPIO_CONSTANT_TIM_1(10); // PB8, PF6
  SET_GPIO_CONSTANT_TIM_1(11); // PB9, PF7
  SET_GPIO_CONSTANT_TIM_2(12); // PH6, PB14, PB15, PH9
  SET_GPIO_CONSTANT_TIM_1(13); // PA6, PF8
  SET_GPIO_CONSTANT_TIM_1(14); // PA7, PF9

#undef SET_GPIO_CONSTANT_TIM_4
#undef SET_GPIO_CONSTANT_TIM_2
#undef SET_GPIO_CONSTANT_TIM_1
#undef SET_GPIO_CONSTANT_TIM
#undef SET_GPIO_CONSTANT_CHANNEL
#undef SET_GPIO_CONSTANT
}

#endif /* ENABLE_MODULE_PWM */


void iotjs_stm32f4dis_pin_initialize(const iotjs_jval_t* jobj) {
  iotjs_jval_t jpin = iotjs_jval_create_object();
  iotjs_jval_set_property_jval(jobj, "pin", &jpin);

#if ENABLE_MODULE_ADC
  iotjs_pin_initialize_adc(&jpin);
#endif /* ENABLE_MODULE_ADC */

#if ENABLE_MODULE_GPIO
  iotjs_pin_initialize_gpio(&jpin);
#endif /* ENABLE_MODULE_GPIO */

#if ENABLE_MODULE_PWM
  iotjs_pin_initialize_pwm(&jpin);
#endif /* ENABLE_MODULE_PWM */

  iotjs_jval_destroy(&jpin);
}


#endif // __NUTTX__
