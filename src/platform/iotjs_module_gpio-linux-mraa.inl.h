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

#ifndef IOTJS_MODULE_GPIO_LINUX_MRAA_INL_H
#define IOTJS_MODULE_GPIO_LINUX_MRAA_INL_H


#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "iotjs_module_gpio.h"
#include "mraa.h"

#define GPIO_MAX_PINNO 63

namespace iotjs {


// MRAA GPIO implementation for linux.
// Implementaion used here are based on:
//  http://iotdk.intel.com/docs/master/mraa/
class GpioLinuxMraa : public Gpio {
 public:
  explicit GpioLinuxMraa(JObject& jgpio);

  static GpioLinuxMraa* GetInstance();

  virtual int Initialize(GpioReqWrap* gpio_req);
  virtual int Release(GpioReqWrap* gpio_req);
  virtual int SetPin(GpioReqWrap* gpio_req);
  virtual int WritePin(GpioReqWrap* gpio_req);
  virtual int ReadPin(GpioReqWrap* gpio_req);

 public:
  bool _ExportPin(int32_t pin);
  bool _UnexportPin(int32_t pin);
  bool _SetPinDirection(int32_t pin, GpioDirection direction);
  bool _SetPinMode(int32_t pin, GpioMode mode);
  bool _WritePin(int32_t pin, bool value);
  bool _ReadPin(int32_t pin, int32_t* value);
  bool _initialized;
  mraa_gpio_context _pin[GPIO_MAX_PINNO + 1];
};


GpioLinuxMraa::GpioLinuxMraa(JObject& jgpio)
    : Gpio(jgpio)
    , _initialized(false) {
  for (int i = 0; i < GPIO_MAX_PINNO; ++i) {
    _pin[i] = NULL;
  }
}


GpioLinuxMraa* GpioLinuxMraa::GetInstance() {
  return static_cast<GpioLinuxMraa*>(Gpio::GetInstance());
}






// Export GPIO pin.
bool GpioLinuxMraa::_ExportPin(int32_t pin) {
  IOTJS_ASSERT(pin >= 0 && pin < GPIO_MAX_PINNO);

  if (this->_pin[pin] == NULL) {
    this->_pin[pin] = mraa_gpio_init(pin);
  }

  return true;
}


// Unexport GPIO pin.
bool GpioLinuxMraa::_UnexportPin(int32_t pin) {
  IOTJS_ASSERT(pin >= 0 && pin < GPIO_MAX_PINNO);
  int ret;
  if (this->_pin[pin]) {
    ret = mraa_gpio_close(this->_pin[pin]);
    if (ret) {
      DDLOG("GPIO _UnexportPin() - fail: %d", ret);
      return false;
    }
    this->_pin[pin] = NULL;
  }

  return true;
}


bool GpioLinuxMraa::_SetPinDirection(int32_t pin, GpioDirection direction) {
  IOTJS_ASSERT(pin >= 0 && pin < GPIO_MAX_PINNO);
  IOTJS_ASSERT(direction == kGpioDirectionIn || direction == kGpioDirectionOut);
  IOTJS_ASSERT(this->_pin[pin]);

  int ret;
  if (direction == kGpioDirectionIn) {
    ret = mraa_gpio_dir(this->_pin[pin], MRAA_GPIO_IN);
  } else {
    ret = mraa_gpio_dir(this->_pin[pin], MRAA_GPIO_OUT);
  }

  if (ret) {
    DDLOG("GPIO _SetPinDirection() - fail: %d", ret);
    return false;
  }
  else {
    return true;
  }
}


// FIXME: Implement SetPinMode()
bool GpioLinuxMraa::_SetPinMode(int32_t pin, GpioMode mode) {
  IOTJS_ASSERT(pin >= 0 && pin < GPIO_MAX_PINNO);
  IOTJS_ASSERT(mode == kGpioModeNone || mode == kGpioModeOpendrain);

  return true;
}


bool GpioLinuxMraa::_WritePin(int32_t pin, bool value) {
  IOTJS_ASSERT(pin >= 0 && pin < GPIO_MAX_PINNO);
  IOTJS_ASSERT(this->_pin[pin]);
  int ret = 0;
  int v = value ? 1 : 0;
  ret = mraa_gpio_write(this->_pin[pin], v);
  if (ret) {
    DDLOG("GPIO _WritePin() - write fail: %d", ret);
    return false;
  }
  else {
    return true;
  }
}


bool GpioLinuxMraa::_ReadPin(int32_t pin, int32_t* value) {
  IOTJS_ASSERT(pin >= 0 && pin < GPIO_MAX_PINNO);
  IOTJS_ASSERT(this->_pin[pin]);
  int ret;
  ret = mraa_gpio_read(this->_pin[pin]);
  if (ret < 0) {
    DDLOG("GPIO _ReadPin() - read fail: %d", ret);
    return false;
  }
  *value = ret;
  return true;
}


void AfterWork(uv_work_t* work_req, int status) {
  GpioLinuxMraa* gpio = GpioLinuxMraa::GetInstance();

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
        jargs.Add(JVal::Bool(req_data->value));
      }
      break;
    }

    case kGpioOpSetPort:
    case kGpioOpWritePort:
    case kGpioOpReadPort:
    {
      IOTJS_ASSERT(!"Not implemented");
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
  GpioLinuxMraa* gpio = GpioLinuxMraa::GetInstance();
  IOTJS_ASSERT(gpio->_initialized == false);

  GpioReqWrap* gpio_req = reinterpret_cast<GpioReqWrap*>(work_req->data);
  GpioReqData* req_data = gpio_req->req();

  DDDLOG("GPIO InitializeWorker()");

  // always return OK
  req_data->result = kGpioErrOk;
}


void ReleaseWorker(uv_work_t* work_req) {
  GpioLinuxMraa* gpio = GpioLinuxMraa::GetInstance();
  IOTJS_ASSERT(gpio->_initialized == true);

  GpioReqWrap* gpio_req = reinterpret_cast<GpioReqWrap*>(work_req->data);
  GpioReqData* req_data = gpio_req->req();

  DDDLOG("GPIO ReleaseWorker()");

  req_data->result = kGpioErrOk;

  // Unexport pins that exported earlier by this module.
  for (int i = 0; i < GPIO_MAX_PINNO; ++i) {
    if (gpio->_pin[i]) {
      if (!gpio->_UnexportPin(i)) {
        req_data->result = kGpioErrSys;
      }
    }
  }
}


void SetPinWorker(uv_work_t* work_req) {
  GpioLinuxMraa* gpio = GpioLinuxMraa::GetInstance();
  IOTJS_ASSERT(gpio->_initialized == true);

  GpioReqWrap* gpio_req = reinterpret_cast<GpioReqWrap*>(work_req->data);
  GpioReqData* req_data = gpio_req->req();

  DDDLOG("GPIO SetPinWorker() - pin: %d, dir: %d, mode: %d",
         req_data->pin, req_data->dir, req_data->mode);

  if (req_data->dir == kGpioDirectionNone) {
    // Unexport GPIO pin.
    if (!gpio->_UnexportPin(req_data->pin)) {
      req_data->result = kGpioErrSys;
      return;
    }
  } else {
    // Export GPIO pin.
    if (!gpio->_ExportPin(req_data->pin)) {
      req_data->result = kGpioErrSys;
      return;
    }
    // Set direction.
    if (!gpio->_SetPinDirection(req_data->pin, req_data->dir)) {
      req_data->result = kGpioErrSys;
      return;
    }
    // Set mode.
    if (!gpio->_SetPinMode(req_data->pin, req_data->mode)) {
      req_data->result = kGpioErrSys;
      return;
    }
  }

  req_data->result = kGpioErrOk;
}


void WritePinWorker(uv_work_t* work_req) {
  GpioLinuxMraa* gpio = GpioLinuxMraa::GetInstance();
  IOTJS_ASSERT(gpio->_initialized == true);

  GpioReqWrap* gpio_req = reinterpret_cast<GpioReqWrap*>(work_req->data);
  GpioReqData* req_data = gpio_req->req();

  DDDLOG("GPIO WritePinWorker() - pin: %d, value: %d",
         req_data->pin, req_data->value);

  if (gpio->_WritePin(req_data->pin, req_data->value)) {
    req_data->result = kGpioErrOk;
  } else {
    req_data->result = kGpioErrSys;
  }
}


void ReadPinWorker(uv_work_t* work_req) {
  GpioLinuxMraa* gpio = GpioLinuxMraa::GetInstance();
  IOTJS_ASSERT(gpio->_initialized == true);

  GpioReqWrap* gpio_req = reinterpret_cast<GpioReqWrap*>(work_req->data);
  GpioReqData* req_data = gpio_req->req();

  DDDLOG("GPIO ReadPinWorker() - pin: %d" ,req_data->pin);

  if (gpio->_ReadPin(req_data->pin, &req_data->value)) {
    req_data->result = kGpioErrOk;
  } else {
    req_data->result = kGpioErrSys;
  }
}


#define GPIO_LINUX_MRAA_IMPL_TEMPLATE(op, initialized) \
  do { \
    GpioLinuxMraa* gpio = GpioLinuxMraa::GetInstance(); \
    IOTJS_ASSERT(gpio->_initialized == initialized); \
    Environment* env = Environment::GetEnv(); \
    uv_work_t* req = new uv_work_t; \
    req->data = reinterpret_cast<void*>(gpio_req); \
    uv_queue_work(env->loop(), req, op ## Worker, AfterWork); \
  } while (0)


int GpioLinuxMraa::Initialize(GpioReqWrap* gpio_req) {
  GPIO_LINUX_MRAA_IMPL_TEMPLATE(Initialize, false);
  return 0;
}


int GpioLinuxMraa::Release(GpioReqWrap* gpio_req) {
  GPIO_LINUX_MRAA_IMPL_TEMPLATE(Release, true);
  return 0;
}


int GpioLinuxMraa::SetPin(GpioReqWrap* gpio_req) {
  GPIO_LINUX_MRAA_IMPL_TEMPLATE(SetPin, true);
  return 0;
}


int GpioLinuxMraa::WritePin(GpioReqWrap* gpio_req) {
  GPIO_LINUX_MRAA_IMPL_TEMPLATE(WritePin, true);
  return 0;
}


int GpioLinuxMraa::ReadPin(GpioReqWrap* gpio_req) {
  GPIO_LINUX_MRAA_IMPL_TEMPLATE(ReadPin, true);
  return 0;
}


} // namespace iotjs


#endif /* IOTJS_MODULE_GPIO_LINUX_MRAA_INL_H */
