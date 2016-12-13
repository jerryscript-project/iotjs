/* Copyright 2015-present Samsung Electronics Co., Ltd. and other contributors
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


typedef enum {
  kGpioDirectionNone = 0,
  kGpioDirectionIn,
  kGpioDirectionOut,
} GpioDirection;


typedef enum {
  kGpioModeNone = 0,
  kGpioModePullup,
  kGpioModePulldown,
  kGpioModeFloat,
  kGpioModePushpull,
  kGpioModeOpendrain,
} GpioMode;


typedef enum {
  kGpioErrOk = 0,
  kGpioErrInitialize = -1,
  kGpioErrNotInitialized = -2,
  kGpioErrWrongUse = -98,
  kGpioErrSys = -99,
} GpioError;


typedef enum {
  kGpioOpInitize,
  kGpioOpRelease,
  kGpioOpOpen,
  kGpioOpWrite,
  kGpioOpRead,
} GpioOp;


typedef struct {
  uint32_t pin;
  bool value;
  GpioDirection dir;
  GpioMode mode; // only for set pin
  GpioError result;
  GpioOp op;
} iotjs_gpio_reqdata_t;


typedef struct {
  iotjs_reqwrap_t reqwrap;
  uv_work_t req;
  iotjs_gpio_reqdata_t req_data;
} IOTJS_VALIDATED_STRUCT(iotjs_gpio_reqwrap_t);

#define THIS iotjs_gpio_reqwrap_t* gpio_reqwrap
iotjs_gpio_reqwrap_t* iotjs_gpio_reqwrap_create(const iotjs_jval_t* jcallback,
                                                GpioOp op);
void iotjs_gpio_reqwrap_dispatched(THIS);
uv_work_t* iotjs_gpio_reqwrap_req(THIS);
const iotjs_jval_t* iotjs_gpio_reqwrap_jcallback(THIS);
iotjs_gpio_reqwrap_t* iotjs_gpio_reqwrap_from_request(uv_work_t* req);
iotjs_gpio_reqdata_t* iotjs_gpio_reqwrap_data(THIS);
#undef THIS


// This Gpio class provides interfaces for GPIO operation.
typedef struct {
  iotjs_jobjectwrap_t jobjectwrap;
  bool initialized;
} IOTJS_VALIDATED_STRUCT(iotjs_gpio_t);

iotjs_gpio_t* iotjs_gpio_create(const iotjs_jval_t* jgpio);
const iotjs_jval_t* iotjs_gpio_get_jgpio();
iotjs_gpio_t* iotjs_gpio_get_instance();
bool iotjs_gpio_initialized();
void iotjs_gpio_set_initialized(iotjs_gpio_t* gpio, bool initialized);


void InitializeGpioWorker(uv_work_t* work_req);
void ReleaseGpioWorker(uv_work_t* work_req);
void OpenGpioWorker(uv_work_t* work_req);
void WriteGpioWorker(uv_work_t* work_req);
void ReadGpioWorker(uv_work_t* work_req);


#endif /* IOTJS_MODULE_GPIO_H */
