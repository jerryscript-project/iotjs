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

#include "stm32_gpio.h"
#include "module/iotjs_module_gpio.h"

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
  0,  // none
  GPIO_INPUT,
  GPIO_OUTPUT | GPIO_OUTPUT_SET | GPIO_FREQUENCY_DEFAULT
};


uint32_t gpioPort[7] = {
  GPIO_PORTA,
  GPIO_PORTB,
  GPIO_PORTC,
  GPIO_PORTD,
  GPIO_PORTE,
  GPIO_PORTF,
  GPIO_PORTG
};


uint32_t gpioPin[GPIO_PINCNT_IN_NUTTXPORT] = {
  GPIO_PIN0,
  GPIO_PIN1,
  GPIO_PIN2,
  GPIO_PIN3,
  GPIO_PIN4,
  GPIO_PIN5,
  GPIO_PIN6,
  GPIO_PIN7,
  GPIO_PIN8,
  GPIO_PIN9,
  GPIO_PIN10,
  GPIO_PIN11,
  GPIO_PIN12,
  GPIO_PIN13,
  GPIO_PIN14,
  GPIO_PIN15
};


uint32_t gpioMode[6] = {
  0,  // none
  GPIO_PULLUP,
  GPIO_PULLDOWN,
  GPIO_FLOAT,
  GPIO_PUSHPULL,
  GPIO_OPENDRAIN
};


namespace iotjs {


// GPIO implementation for arm-nuttx target.
class GpioArmNuttxStm32 : public Gpio {
 public:
  explicit GpioArmNuttxStm32(const iotjs_jval_t* jgpio);

  static GpioArmNuttxStm32* GetInstance();

  virtual int Initialize(GpioReqWrap* gpio_req);
  virtual int Release(GpioReqWrap* gpio_req);
  virtual int Open(GpioReqWrap* gpio_req);
  virtual int Write(GpioReqWrap* gpio_req);
  virtual int Read(GpioReqWrap* gpio_req);

  bool _initialized;
};


GpioArmNuttxStm32::GpioArmNuttxStm32(const iotjs_jval_t* jgpio)
    : Gpio(jgpio)
    , _initialized(false) {
}


GpioArmNuttxStm32* GpioArmNuttxStm32::GetInstance()
{
  return static_cast<GpioArmNuttxStm32*>(Gpio::GetInstance());
}


#define GPIO_WORKER_INIT_TEMPLATE \
  GpioArmNuttxStm32* gpio = GpioArmNuttxStm32::GetInstance(); \
  IOTJS_ASSERT(gpio->_initialized == false); \
  GpioReqWrap* gpio_req = reinterpret_cast<GpioReqWrap*>(work_req->data); \
  GpioReqData* req_data = gpio_req->req(); \


void AfterGPIOWork(uv_work_t* work_req, int status) {
  GpioArmNuttxStm32* gpio = GpioArmNuttxStm32::GetInstance();

  GpioReqWrap* gpio_req = reinterpret_cast<GpioReqWrap*>(work_req->data);
  GpioReqData* req_data = gpio_req->req();

  if (status) {
    req_data->result = kGpioErrSys;
  }

  iotjs_jargs_t jargs = iotjs_jargs_create(2);
  iotjs_jargs_append_number(&jargs, req_data->result);

  switch (req_data->op) {
    case kGpioOpInitize:
    {
      if (req_data->result == kGpioErrOk) {
        gpio->_initialized = true;
      }
      break;
    }
    case kGpioOpRelease:
    {
      if (req_data->result == kGpioErrOk) {
        gpio->_initialized = false;
      }
      break;
    }
    case kGpioOpOpen:
    case kGpioOpWrite:
    {
      break;
    }
    case kGpioOpRead:
    {
      if (req_data->result == kGpioErrOk) {
        iotjs_jargs_append_bool(&jargs, !!req_data->value);
      }
      break;
    }
    default:
    {
      IOTJS_ASSERT(!"Unreachable");
      break;
    }
  }

  iotjs_make_callback(gpio_req->jcallback(), Gpio::GetJGpio(), &jargs);

  delete work_req;
  delete gpio_req;
  iotjs_jargs_destroy(&jargs);
}


void InitializeGpioWorker(uv_work_t* work_req) {
  GpioArmNuttxStm32* gpio = GpioArmNuttxStm32::GetInstance();
  IOTJS_ASSERT(gpio->_initialized == false);

  GpioReqWrap* gpio_req = reinterpret_cast<GpioReqWrap*>(work_req->data);
  GpioReqData* req_data = gpio_req->req();

  DDDLOG("GPIO InitializeGpioWorker()");

  stm32_gpioinit();

  // always return OK
  req_data->result = kGpioErrOk;
}


void ReleaseGpioWorker(uv_work_t* work_req) {
  GpioArmNuttxStm32* gpio = GpioArmNuttxStm32::GetInstance();
  IOTJS_ASSERT(gpio->_initialized == true);

  GpioReqWrap* gpio_req = reinterpret_cast<GpioReqWrap*>(work_req->data);
  GpioReqData* req_data = gpio_req->req();

  DDDLOG("GPIO ReleaseGpioWorker()");

  // always return OK
  req_data->result = kGpioErrOk;
}


void OpenGpioWorker(uv_work_t* work_req) {
  GPIO_WORKER_INIT_TEMPLATE;

  DDDLOG("GPIO OpenGpioWorker() - pin: %d, dir: %d, mode: %d",
         req_data->pin, req_data->dir, req_data->mode);

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
  GPIO_WORKER_INIT_TEMPLATE;

  DDDLOG("GPIO WriteGpioWorker() - pin: %d, value: %d",
         req_data->pin, req_data->value);

  uint32_t port_index = GetGpioPortIndexFromPinNumber(req_data->pin);
  uint32_t pin_index = GetGpioPinIndexFromPinNumber(req_data->pin);
  uint32_t pinset = gpioPort[port_index] | gpioPin[pin_index];
  bool data = req_data->value ? 1 : 0;

  stm32_gpiowrite(pinset, data);

  // always return OK
  req_data->result = kGpioErrOk;
}


void ReadGpioWorker(uv_work_t* work_req) {
  GPIO_WORKER_INIT_TEMPLATE;

  DDDLOG("GPIO ReadGpioWorker() - pin: %d" ,req_data->pin);

  uint32_t port_index = GetGpioPortIndexFromPinNumber(req_data->pin);
  uint32_t pin_index = GetGpioPinIndexFromPinNumber(req_data->pin);
  uint32_t pinset = gpioPort[port_index] | gpioPin[pin_index];

  req_data->value = (uint32_t)stm32_gpioread(pinset);

  // always return OK
  req_data->result = kGpioErrOk;
}


#define GPIO_NUTTX_IMPL_TEMPLATE(op, initialized) \
  do { \
    GpioArmNuttxStm32* gpio = GpioArmNuttxStm32::GetInstance(); \
    IOTJS_ASSERT(gpio->_initialized == initialized); \
    const iotjs_environment_t* env = iotjs_environment_get(); \
    uv_loop_t* loop = iotjs_environment_loop(env); \
    uv_work_t* req = new uv_work_t; \
    req->data = reinterpret_cast<void*>(gpio_req); \
    uv_queue_work(loop, req, op ## GpioWorker, AfterGPIOWork); \
  } while (0)


int GpioArmNuttxStm32::Initialize(GpioReqWrap* gpio_req) {
  GPIO_NUTTX_IMPL_TEMPLATE(Initialize, false);
  return 0;
}


int GpioArmNuttxStm32::Release(GpioReqWrap* gpio_req) {
  GPIO_NUTTX_IMPL_TEMPLATE(Release, true);
  return 0;
}


int GpioArmNuttxStm32::Open(GpioReqWrap* gpio_req) {
  GPIO_NUTTX_IMPL_TEMPLATE(Open, true);
  return 0;
}


int GpioArmNuttxStm32::Write(GpioReqWrap* gpio_req) {
  GPIO_NUTTX_IMPL_TEMPLATE(Write, true);
  return 0;
}


int GpioArmNuttxStm32::Read(GpioReqWrap* gpio_req) {
  GPIO_NUTTX_IMPL_TEMPLATE(Read, true);
  return 0;
}


} // namespace iotjs

#endif /* IOTJS_MODULE_GPIO_ARM_NUTTX_STM32_INL_H */
