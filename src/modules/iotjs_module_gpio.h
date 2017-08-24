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
  kGpioDirectionIn = 0,
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
  kGpioEdgeNone = 0,
  kGpioEdgeRising,
  kGpioEdgeFalling,
  kGpioEdgeBoth,
} GpioEdge;


typedef enum {
  kGpioOpOpen,
  kGpioOpWrite,
  kGpioOpRead,
  kGpioOpClose,
} GpioOp;


typedef struct {
  bool value;
  bool result;
  GpioOp op;
} iotjs_gpio_reqdata_t;

typedef struct _iotjs_gpio_module_platform_t* iotjs_gpio_module_platform_t;

// This Gpio class provides interfaces for GPIO operation.
typedef struct {
  iotjs_jobjectwrap_t jobjectwrap;
  uint32_t pin;
  GpioDirection direction;
  GpioMode mode;
  GpioEdge edge;
  iotjs_gpio_module_platform_t platform;
} IOTJS_VALIDATED_STRUCT(iotjs_gpio_t);


typedef struct {
  iotjs_reqwrap_t reqwrap;
  uv_work_t req;
  iotjs_gpio_reqdata_t req_data;
  iotjs_gpio_t* gpio_instance;
} IOTJS_VALIDATED_STRUCT(iotjs_gpio_reqwrap_t);


#define THIS iotjs_gpio_reqwrap_t* gpio_reqwrap

iotjs_gpio_reqwrap_t* iotjs_gpio_reqwrap_from_request(uv_work_t* req);
iotjs_gpio_reqdata_t* iotjs_gpio_reqwrap_data(THIS);

iotjs_gpio_t* iotjs_gpio_instance_from_reqwrap(THIS);

#undef THIS


#define GPIO_WORKER_INIT                                                      \
  iotjs_gpio_reqwrap_t* req_wrap = iotjs_gpio_reqwrap_from_request(work_req); \
  iotjs_gpio_reqdata_t* req_data = iotjs_gpio_reqwrap_data(req_wrap);         \
  iotjs_gpio_t* gpio = iotjs_gpio_instance_from_reqwrap(req_wrap);


void iotjs_gpio_open_worker(uv_work_t* work_req);
bool iotjs_gpio_write(iotjs_gpio_t* gpio, bool value);
int iotjs_gpio_read(iotjs_gpio_t* gpio);
bool iotjs_gpio_close(iotjs_gpio_t* gpio);
void iotjs_gpio_platform_create(iotjs_gpio_t_impl_t* gpio);
void iotjs_gpio_platform_destroy(iotjs_gpio_t_impl_t* gpio);

#endif /* IOTJS_MODULE_GPIO_H */
