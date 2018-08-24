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
#include "iotjs_module_periph_common.h"


typedef enum {
  kGpioDirectionIn = 0,
  kGpioDirectionOut,
  __kGpioDirectionMax
} GpioDirection;

typedef enum {
  kGpioModeNone = 0,
  kGpioModePullup,
  kGpioModePulldown,
  kGpioModeFloat,
  kGpioModePushpull,
  kGpioModeOpendrain,
  __kGpioModeMax
} GpioMode;

typedef enum {
  kGpioEdgeNone = 0,
  kGpioEdgeRising,
  kGpioEdgeFalling,
  kGpioEdgeBoth,
  __kGpioEdgeMax
} GpioEdge;

typedef struct iotjs_gpio_platform_data_s iotjs_gpio_platform_data_t;

// This Gpio class provides interfaces for GPIO operation.
typedef struct {
  jerry_value_t jobject;
  iotjs_gpio_platform_data_t* platform_data;

  bool value;
  uint32_t pin;
  GpioDirection direction;
  GpioMode mode;
  GpioEdge edge;
} iotjs_gpio_t;

bool iotjs_gpio_open(iotjs_gpio_t* gpio);
bool iotjs_gpio_write(iotjs_gpio_t* gpio);
bool iotjs_gpio_read(iotjs_gpio_t* gpio);
bool iotjs_gpio_close(iotjs_gpio_t* gpio);
bool iotjs_gpio_set_direction(iotjs_gpio_t* gpio);

// Platform-related functions; they are implemented
// by platform code (i.e.: linux, nuttx, tizen).
void iotjs_gpio_create_platform_data(iotjs_gpio_t* gpio);
void iotjs_gpio_destroy_platform_data(
    iotjs_gpio_platform_data_t* platform_data);

#endif /* IOTJS_MODULE_GPIO_H */
