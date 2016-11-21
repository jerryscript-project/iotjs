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

#ifndef IOTJS_MODULE_GPIO_ARM_NUTTX_STM32_INL_H
#define IOTJS_MODULE_GPIO_ARM_NUTTX_STM32_INL_H

#include "module/iotjs_module_gpio.h"
#include "stm32_gpio.h"

#define GPIO_FREQUENCY_DEFAULT GPIO_SPEED_25MHz
#define GPIO_PINCNT_IN_NUTTXPORT 16

#define GPIO_CONFIG_OK 0

// Change iotjs pin or port number to nuttx gpio pin or port index
static inline uint32_t GetGpioPortIndexFromPinNumber(int32_t pin) {
  return (pin / GPIO_PINCNT_IN_NUTTXPORT);
}


static inline uint32_t GetGpioPinIndexFromPinNumber(int32_t pin) {
  return (pin % GPIO_PINCNT_IN_NUTTXPORT);
}


uint32_t gpioDirection[3] = {
  0, // none
  GPIO_INPUT, GPIO_OUTPUT | GPIO_OUTPUT_SET | GPIO_FREQUENCY_DEFAULT,
};


uint32_t gpioPort[7] = {
  GPIO_PORTA, GPIO_PORTB, GPIO_PORTC, GPIO_PORTD,
  GPIO_PORTE, GPIO_PORTF, GPIO_PORTG,
};


uint32_t gpioPin[GPIO_PINCNT_IN_NUTTXPORT] = {
  GPIO_PIN0,  GPIO_PIN1,  GPIO_PIN2,  GPIO_PIN3,  GPIO_PIN4,  GPIO_PIN5,
  GPIO_PIN6,  GPIO_PIN7,  GPIO_PIN8,  GPIO_PIN9,  GPIO_PIN10, GPIO_PIN11,
  GPIO_PIN12, GPIO_PIN13, GPIO_PIN14, GPIO_PIN15,
};


uint32_t gpioMode[6] = {
  0, // none
  GPIO_PULLUP, GPIO_PULLDOWN, GPIO_FLOAT, GPIO_PUSHPULL, GPIO_OPENDRAIN,
};


#define GPIO_WORKER_INIT_TEMPLATE(initialized)                                \
  IOTJS_ASSERT(iotjs_gpio_initialized() == initialized);                      \
  iotjs_gpio_reqwrap_t* req_wrap = iotjs_gpio_reqwrap_from_request(work_req); \
  iotjs_gpio_reqdata_t* req_data = iotjs_gpio_reqwrap_data(req_wrap);


void InitializeGpioWorker(uv_work_t* work_req) {
  GPIO_WORKER_INIT_TEMPLATE(false);
  DDDLOG("GPIO InitializeGpioWorker()");

  stm32_gpioinit();

  // always return OK
  req_data->result = kGpioErrOk;
}


void ReleaseGpioWorker(uv_work_t* work_req) {
  GPIO_WORKER_INIT_TEMPLATE(true);
  DDDLOG("GPIO ReleaseGpioWorker()");

  // always return OK
  req_data->result = kGpioErrOk;
}


void OpenGpioWorker(uv_work_t* work_req) {
  GPIO_WORKER_INIT_TEMPLATE(true);
  DDDLOG("Gpio OpenGpioWorker() - pin: %d, dir: %d, mode: %d", req_data->pin,
         req_data->dir, req_data->mode);

  uint32_t port_index = GetGpioPortIndexFromPinNumber(req_data->pin);
  uint32_t pin_index = GetGpioPinIndexFromPinNumber(req_data->pin);
  uint32_t cfgset = 0;

  if (req_data->dir == kGpioDirectionNone) {
    cfgset = gpioPort[port_index] | gpioPin[pin_index];

    if (stm32_unconfiggpio(cfgset) != GPIO_CONFIG_OK) {
      req_data->result = kGpioErrSys;
      return;
    }
  } else {
    // Set pin direction and mode
    cfgset = gpioDirection[req_data->dir] | gpioMode[req_data->mode] |
             gpioPort[port_index] | gpioPin[pin_index];

    if (stm32_configgpio(cfgset) != GPIO_CONFIG_OK) {
      req_data->result = kGpioErrSys;
      return;
    }
  }

  req_data->result = kGpioErrOk;
}


void WriteGpioWorker(uv_work_t* work_req) {
  GPIO_WORKER_INIT_TEMPLATE(true);
  DDDLOG("Gpio WriteGpioWorker() - pin: %d, value: %d", req_data->pin,
         req_data->value);

  uint32_t port_index = GetGpioPortIndexFromPinNumber(req_data->pin);
  uint32_t pin_index = GetGpioPinIndexFromPinNumber(req_data->pin);
  uint32_t pinset = gpioPort[port_index] | gpioPin[pin_index];
  bool value = req_data->value;

  stm32_gpiowrite(pinset, value);

  // always return OK
  req_data->result = kGpioErrOk;
}


void ReadGpioWorker(uv_work_t* work_req) {
  GPIO_WORKER_INIT_TEMPLATE(true);
  DDDLOG("Gpio ReadGpioWorker() - pin: %d", req_data->pin);

  uint32_t port_index = GetGpioPortIndexFromPinNumber(req_data->pin);
  uint32_t pin_index = GetGpioPinIndexFromPinNumber(req_data->pin);
  uint32_t pinset = gpioPort[port_index] | gpioPin[pin_index];

  // always return OK
  req_data->result = kGpioErrOk;
  req_data->value = stm32_gpioread(pinset);
}


#endif /* IOTJS_MODULE_GPIO_ARM_NUTTX_STM32_INL_H */
