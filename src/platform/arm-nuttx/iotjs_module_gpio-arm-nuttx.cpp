/* Copyright 2015 Samsung Electronics Co., Ltd.
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

#include <nuttx/gpio.h>

#include "iotjs_module_gpio.h"

#define GPIO_PINCNT_IN_NUTTXPORT 16
#define GPIO_PORTCNT_IN_NUTTXPORT 2

// Nuttx gpio output mode bit
#define GPIO_OPENDRAIN (1 << 9)
#define GPIO_PUSHPULL (0)

// Change iotjs pin or port number to nuttx gpio pin or port index
static inline int32_t GetGpioPortIndexFromPinNumber(int32_t pin) {
  return (pin / GPIO_PINCNT_IN_NUTTXPORT);
}


static inline int32_t GetGpioPinIndexFromPinNumber(int32_t pin) {
  return (pin % GPIO_PINCNT_IN_NUTTXPORT);
}


static inline int32_t GetGpioPortIndexFromPortNumber(int32_t port) {
  return (port / GPIO_PORTCNT_IN_NUTTXPORT);
}


static inline int32_t GetGpioPinIndexFromPortNumber(int32_t port) {
  return ((port % GPIO_PORTCNT_IN_NUTTXPORT) * GPIO_PINCNT_IN_IOTJS);
}


uint32_t gpioDirection[3] = {
  0,  // none
  GPIO_INPUT,
  GPIO_OUTPUT
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


uint32_t gpioPin[GPIO_MAX_PINNO] = {
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


// GPIO implementeation for arm-nuttx target.
class GpioArmNuttx : public Gpio {
 public:
  explicit GpioArmNuttx(JObject& jgpio);

  static GpioArmNuttx* GetInstance();

  virtual int Initialize(GpioReqWrap* gpio_req);
  virtual int Release(GpioReqWrap* gpio_req);
  virtual int SetPin(GpioReqWrap* gpio_req);
  virtual int WritePin(GpioReqWrap* gpio_req);
  virtual int ReadPin(GpioReqWrap* gpio_req);
  virtual int SetPort(GpioReqWrap* gpio_req);
  virtual int WritePort(GpioReqWrap* gpio_req);
  virtual int ReadPort(GpioReqWrap* gpio_req);

  bool _initialized;
  int _fd;
};


Gpio* Gpio::Create(JObject& jgpio) {
  return new GpioArmNuttx(jgpio);
}


GpioArmNuttx::GpioArmNuttx(JObject& jgpio)
    : Gpio(jgpio)
    , _initialized(false)
    , _fd(NULL) {
}


GpioArmNuttx* GpioArmNuttx::GetInstance()
{
  return static_cast<GpioArmNuttx*>(Gpio::GetInstance());
}


#define GPIO_WORKER_INIT_TEMPLATE \
  GpioArmNuttx* gpio = GpioArmNuttx::GetInstance(); \
  IOTJS_ASSERT(gpio->_initialized == false); \
  GpioReqWrap* gpio_req = reinterpret_cast<GpioReqWrap*>(work_req->data); \
  GpioReqData* req_data = gpio_req->req(); \


void AfterWork(uv_work_t* work_req, int status) {
  GpioArmNuttx* gpio = GpioArmNuttx::GetInstance();

  GpioReqWrap* gpio_req = reinterpret_cast<GpioReqWrap*>(work_req->data);
  GpioReqData* req_data = gpio_req->req();

  if (status) {
    req_data->result = kGpioErrSys;
  }

  JArgList jargs(2);
  jargs.Add(JVal::Number(req_data->result));

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
    case kGpioOpSetPin:
    case kGpioOpWritePin:
    {
      break;
    }
    case kGpioOpReadPin:
    {
      if (req_data->result == kGpioErrOk) {
        jargs.Add(JVal::Bool(!!req_data->value));
      }
      break;
    }
    case kGpioOpSetPort:
    case kGpioOpWritePort:
    {
      break;
    }
    case kGpioOpReadPort:
    {
      if (req_data->result == kGpioErrOk) {
        jargs.Add(JVal::Number(req_data->value));
      }
      break;
    }
    default:
    {
      IOTJS_ASSERT(!"Unreachable");
      break;
    }
  }

  MakeCallback(gpio_req->jcallback(), *Gpio::GetJGpio(), jargs);

  delete work_req;
  delete gpio_req;
}


void InitializeWorker(uv_work_t* work_req) {
  GpioArmNuttx* gpio = GpioArmNuttx::GetInstance();
  IOTJS_ASSERT(gpio->_initialized == false);

  GpioReqWrap* gpio_req = reinterpret_cast<GpioReqWrap*>(work_req->data);
  GpioReqData* req_data = gpio_req->req();

  DDDLOG("GPIO InitializeWorker()");

  // Check gpio file
  const char* devfilepath = "/dev/gpio";
  gpio->_fd = open(devfilepath, O_RDWR);
  DDDLOG("gpio> %s : fd(%d)", devfilepath, gpio->_fd);

  if (gpio->_fd) {
    req_data->result = kGpioErrOk;
  } else {
    req_data->result = kGpioErrInitialize;
  }
}


void ReleaseWorker(uv_work_t* work_req) {
  GpioArmNuttx* gpio = GpioArmNuttx::GetInstance();
  IOTJS_ASSERT(gpio->_initialized == true);

  GpioReqWrap* gpio_req = reinterpret_cast<GpioReqWrap*>(work_req->data);
  GpioReqData* req_data = gpio_req->req();

  DDDLOG("GPIO ReleaseWorker()");

  if (gpio->_fd) {
    close(gpio->_fd);
  }
  req_data->result = kGpioErrOk;
}


void SetPinWorker(uv_work_t* work_req) {
  GPIO_WORKER_INIT_TEMPLATE;

  DDDLOG("GPIO SetPinWorker() - pin: %d, dir: %d, mode: %d",
         req_data->pin, req_data->dir, req_data->mode);

  struct gpioioctl_config_s cdata;
  int32_t port_index = GetGpioPortIndexFromPinNumber(req_data->pin);
  int32_t pin_index = GetGpioPinIndexFromPinNumber(req_data->pin);

  // Set pin direction and mode
  cdata.port = gpioDirection[req_data->dir] | gpioMode[req_data->mode] |
               gpioPort[port_index] | gpioPin[pin_index];

  if (!gpio_ioctl_config(&cdata)) {
    req_data->result = kGpioErrOk;
  } else {
    req_data->result = kGpioErrSys;
  }
}


void WritePinWorker(uv_work_t* work_req) {
  GPIO_WORKER_INIT_TEMPLATE;

  DDDLOG("GPIO WritePinWorker() - pin: %d, value: %d",
         req_data->pin, req_data->value);

  struct gpioioctl_write_s wdata;
  int32_t port_index = GetGpioPortIndexFromPinNumber(req_data->pin);
  int32_t pin_index = GetGpioPinIndexFromPinNumber(req_data->pin);

  wdata.port = gpioPort[port_index] | gpioPin[pin_index];
  wdata.data = req_data->value;

  if (!gpio_ioctl_write(&wdata)) {
    req_data->result = kGpioErrOk;
  } else {
    req_data->result = kGpioErrSys;
  }
}


void ReadPinWorker(uv_work_t* work_req) {
  GPIO_WORKER_INIT_TEMPLATE;

  DDDLOG("GPIO ReadPinWorker() - pin: %d" ,req_data->pin);

  struct gpioioctl_read_s rdata;
  int32_t port_index = GetGpioPortIndexFromPinNumber(req_data->pin);
  int32_t pin_index = GetGpioPinIndexFromPinNumber(req_data->pin);

  rdata.port = gpioPort[port_index] | gpioPin[pin_index];
  rdata.data = 0;

  if (!gpio_ioctl_read(&rdata)) {
    req_data->value = (int32_t)rdata.data;
    req_data->result = kGpioErrOk;
  } else {
    req_data->result = kGpioErrSys;
  }
}


void SetPortWorker(uv_work_t* work_req) {
  GPIO_WORKER_INIT_TEMPLATE;

  DDDLOG("GPIO SetPortWorker() - port: %d, dir: %d, mode: %d",
         req_data->pin, req_data->dir, req_data->mode);

  struct gpioioctl_config_s cdata;
  int32_t port_index = GetGpioPortIndexFromPortNumber(req_data->pin);
  int32_t pin_index = GetGpioPinIndexFromPortNumber(req_data->pin);

  for (int offset = 0; offset < GPIO_PINCNT_IN_IOTJS; offset++) {
    cdata.port = gpioDirection[req_data->dir] | gpioMode[req_data->mode] |
                 gpioPort[port_index] | gpioPin[pin_index + offset];

    if (gpio_ioctl_config(&cdata)) {
      req_data->result = kGpioErrSys;
      return;
    }
  }

  req_data->result = kGpioErrOk;
}


void WritePortWorker(uv_work_t* work_req) {
  GPIO_WORKER_INIT_TEMPLATE;

  DDDLOG("GPIO WritePortWorker() - pin: %d, value: %d",
         req_data->pin, req_data->value);

  struct gpioioctl_write_s wdata;
  int32_t port_index = GetGpioPortIndexFromPortNumber(req_data->pin);
  int32_t pin_index = GetGpioPinIndexFromPortNumber(req_data->pin);

  for (int offset = 0; offset < GPIO_PINCNT_IN_IOTJS; offset++) {
    wdata.port = gpioPort[port_index] | gpioPin[pin_index + offset];
    wdata.data = req_data->value & (1 << offset);

    if (gpio_ioctl_write(&wdata)) {
      req_data->result = kGpioErrSys;
    }
  }

  req_data->result = kGpioErrOk;
}


void ReadPortWorker(uv_work_t* work_req) {
  GPIO_WORKER_INIT_TEMPLATE;

  DDDLOG("GPIO ReadPortWorker() - pin: %d" ,req_data->pin);

  struct gpioioctl_read_s rdata;
  int32_t port_index = GetGpioPortIndexFromPortNumber(req_data->pin);
  int32_t pin_index = GetGpioPinIndexFromPortNumber(req_data->pin);

  req_data->value = 0x00;

  for (int offset = 0; offset < GPIO_PINCNT_IN_IOTJS; offset++) {
    rdata.port = gpioPort[port_index] | gpioPin[pin_index + offset];

    if (gpio_ioctl_read(&rdata)) {
      req_data->result = kGpioErrSys;
      return;
    }

    if (rdata.data) {
      req_data->value |= (1 << offset);
    }
  }

  req_data->result = kGpioErrOk;
}


#define GPIO_NUTTX_IMPL_TEMPLATE(op, initialized) \
  do { \
    GpioArmNuttx* gpio = GpioArmNuttx::GetInstance(); \
    IOTJS_ASSERT(gpio->_initialized == initialized); \
    Environment* env = Environment::GetEnv(); \
    uv_work_t* req = new uv_work_t; \
    req->data = reinterpret_cast<void*>(gpio_req); \
    uv_queue_work(env->loop(), req, op ## Worker, AfterWork); \
  } while (0)


int GpioArmNuttx::Initialize(GpioReqWrap* gpio_req) {
  GPIO_NUTTX_IMPL_TEMPLATE(Initialize, false);
  return 0;
}


int GpioArmNuttx::Release(GpioReqWrap* gpio_req) {
  GPIO_NUTTX_IMPL_TEMPLATE(Release, true);
  return 0;
}


int GpioArmNuttx::SetPin(GpioReqWrap* gpio_req) {
  GPIO_NUTTX_IMPL_TEMPLATE(SetPin, true);
  return 0;
}


int GpioArmNuttx::WritePin(GpioReqWrap* gpio_req) {
  GPIO_NUTTX_IMPL_TEMPLATE(WritePin, true);
  return 0;
}


int GpioArmNuttx::ReadPin(GpioReqWrap* gpio_req) {
  GPIO_NUTTX_IMPL_TEMPLATE(ReadPin, true);
  return 0;
}


int GpioArmNuttx::SetPort(GpioReqWrap* gpio_req) {
  GPIO_NUTTX_IMPL_TEMPLATE(SetPort, true);
  return 0;
}


int GpioArmNuttx::WritePort(GpioReqWrap* gpio_req) {
  GPIO_NUTTX_IMPL_TEMPLATE(WritePort, true);
  return 0;
}


int GpioArmNuttx::ReadPort(GpioReqWrap* gpio_req) {
  GPIO_NUTTX_IMPL_TEMPLATE(ReadPort, true);
  return 0;
}


} // namespace iotjs

#endif // __NUTTX__
