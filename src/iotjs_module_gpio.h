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


#ifndef IOTJS_MODULE_GPIO_H
#define IOTJS_MODULE_GPIO_H


#include "iotjs_def.h"
#include "iotjs_objectwrap.h"
#include "iotjs_reqwrap.h"


namespace iotjs {


enum GpioDirection {
  kGpioDirectionNone = 0,
  kGpioDirectionIn,
  kGpioDirectionOut,
};


enum GpioMode {
  kGpioModeNone = 0,
  kGpioModePullup,
  kGpioModePulldown,
  kGpioModeFloat,
  kGpioModePushpull,
  kGpioModeOpendrain,
};


enum GpioError {
  kGpioErrOk = 0,
  kGpioErrInitialize = -1,
  kGpioErrNotInitialized = -2,
  kGpioErrWrongUse = -98,
  kGpioErrSys = -99,
};


enum GpioOp {
  kGpioOpInitize,
  kGpioOpRelease,
  kGpioOpSetPin,
  kGpioOpWritePin,
  kGpioOpReadPin,
  kGpioOpSetPort,
  kGpioOpWritePort,
  kGpioOpReadPort,
};

struct GpioReqData {
  int32_t pin;
  int32_t value;
  GpioDirection dir;
  GpioMode mode; // only for set pin
  GpioError result;
  GpioOp op;

  void* data; // pointer to GpioReqWrap
};

typedef ReqWrap<GpioReqData> GpioReqWrap;


// This Gpio class provides interfaces for GPIO operation.
class Gpio : public JObjectWrap {
 public:
  explicit Gpio(JObject& jgpio);
  virtual ~Gpio();

  static Gpio* Create(JObject& jgpio);
  static Gpio* GetInstance();
  static JObject* GetJGpio();

  virtual int Initialize(GpioReqWrap* gpio_req) = 0;
  virtual int Release(GpioReqWrap* gpio_req) = 0;
  virtual int SetPin(GpioReqWrap* gpio_req) = 0;
  virtual int WritePin(GpioReqWrap* gpio_req) = 0;
  virtual int ReadPin(GpioReqWrap* gpio_req) = 0;
};


JObject* InitGpio();


} // namespace iotjs


#endif /* IOTJS_MODULE_GPIO_H */
