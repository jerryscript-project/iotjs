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

#ifndef IOTJS_MODULE_GPIO_LINUX_GENERAL_INL_H
#define IOTJS_MODULE_GPIO_LINUX_GENERAL_INL_H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "iotjs_systemio-linux.h"
#include "module/iotjs_module_gpio.h"


#define GPIO_INTERFACE "/sys/class/gpio/"
#define GPIO_EXPORT "export"
#define GPIO_UNEXPORT "unexport"
#define GPIO_DIRECTION "direction"
#define GPIO_EDGE "edge"
#define GPIO_VALUE "value"
#define GPIO_PIN_INTERFACE "gpio%d/"
#define GPIO_PIN_FORMAT_EXPORT GPIO_INTERFACE "export"
#define GPIO_PIN_FORMAT_UNEXPORT GPIO_INTERFACE "unexport"
#define GPIO_PIN_FORMAT GPIO_INTERFACE GPIO_PIN_INTERFACE
#define GPIO_PIN_FORMAT_DIRECTION GPIO_PIN_FORMAT GPIO_DIRECTION
#define GPIO_PIN_FORMAT_EDGE GPIO_PIN_FORMAT GPIO_EDGE
#define GPIO_PIN_FORMAT_VALUE GPIO_PIN_FORMAT GPIO_VALUE

#define GPIO_PATH_BUFFER_SIZE DEVICE_IO_PATH_BUFFER_SIZE
#define GPIO_PIN_BUFFER_SIZE DEVICE_IO_PIN_BUFFER_SIZE
#define GPIO_VALUE_BUFFER_SIZE 10


// Implementation used here are based on:
//  https://www.kernel.org/doc/Documentation/gpio/sysfs.txt


// Unexport GPIO pin.
bool UnexportPin(int32_t pin) {
  DDDLOG("GPIO UnexportPin() - path: %s", GPIO_PIN_FORMAT_UNEXPORT);

  char buff[GPIO_PIN_BUFFER_SIZE] = { 0 };
  snprintf(buff, GPIO_PIN_BUFFER_SIZE - 1, "%d", pin);

  if (!DeviceOpenWriteClose(GPIO_PIN_FORMAT_UNEXPORT, buff)) {
    return false;
  }

  return true;
}


bool SetPinDirection(int32_t pin, GpioDirection direction) {
  IOTJS_ASSERT(direction == kGpioDirectionIn || direction == kGpioDirectionOut);

  char direction_path[GPIO_PATH_BUFFER_SIZE] = { 0 };
  snprintf(direction_path, GPIO_PATH_BUFFER_SIZE - 1, GPIO_PIN_FORMAT_DIRECTION,
           pin);

  char buffer[4] = { 0 };
  if (direction == kGpioDirectionIn) {
    strcpy(buffer, "in");
  } else {
    strcpy(buffer, "out");
  }

  DDDLOG("GPIO SetPinDirection() - path: %s, dir: %s", direction_path, buffer);

  return DeviceOpenWriteClose(direction_path, buffer);
}


// FIXME: Implement SetPinMode()
bool SetPinMode(int32_t pin, GpioMode mode) {
  IOTJS_ASSERT(mode != kGpioModeNone && mode != kGpioModeOpendrain);

  return true;
}


#define GPIO_WORKER_INIT_TEMPLATE(initialized)                                \
  IOTJS_ASSERT(iotjs_gpio_initialized() == initialized);                      \
  iotjs_gpio_reqwrap_t* req_wrap = iotjs_gpio_reqwrap_from_request(work_req); \
  iotjs_gpio_reqdata_t* req_data = iotjs_gpio_reqwrap_data(req_wrap);


void InitializeGpioWorker(uv_work_t* work_req) {
  GPIO_WORKER_INIT_TEMPLATE(false);
  DDDLOG("GPIO InitializeGpioWorker()");

  // Check if GPIO interface exits.
  if (DeviceCheckPath(GPIO_INTERFACE)) {
    req_data->result = kGpioErrOk;
  } else {
    req_data->result = kGpioErrInitialize;
  }
}


void ReleaseGpioWorker(uv_work_t* work_req) {
  GPIO_WORKER_INIT_TEMPLATE(true);
  DDDLOG("GPIO ReleaseGpioWorker()");

  req_data->result = kGpioErrOk;

  const iotjs_environment_t* env = iotjs_environment_get();
  uv_loop_t* loop = iotjs_environment_loop(env);

  int pin_number;
  static uv_fs_t scandir_req;
  uv_dirent_t dent;

  uv_fs_scandir(loop, &scandir_req, GPIO_INTERFACE, 0, NULL);

  while (UV_EOF != uv_fs_scandir_next(&scandir_req, &dent)) {
    if (sscanf(dent.name, GPIO_PIN_INTERFACE, &pin_number) == 1) {
      if (!UnexportPin(pin_number)) {
        req_data->result = kGpioErrSys;
      }
    }
  }
}


void OpenGpioWorker(uv_work_t* work_req) {
  GPIO_WORKER_INIT_TEMPLATE(true);
  uint32_t pin = req_data->pin;
  DDDLOG("Gpio OpenGpioWorker() - pin: %d, dir: %d, mode: %d", req_data->pin,
         req_data->dir, req_data->mode);

  if (req_data->dir == kGpioDirectionNone) {
    // Unexport GPIO pin.
    if (!UnexportPin(req_data->pin)) {
      req_data->result = kGpioErrSys;
      return;
    }
  } else {
    // Export GPIO pin.
    char exported_path[GPIO_PATH_BUFFER_SIZE] = { 0 };
    snprintf(exported_path, GPIO_PATH_BUFFER_SIZE - 1, GPIO_PIN_FORMAT, pin);

    const char* created_files[] = { GPIO_DIRECTION, GPIO_EDGE, GPIO_VALUE };
    int created_files_length = sizeof(created_files) / sizeof(created_files[0]);

    if (!DeviceExport(GPIO_PIN_FORMAT_EXPORT, pin, exported_path, created_files,
                      created_files_length)) {
      req_data->result = kGpioErrSys;
      return;
    }
    // Set direction.
    if (!SetPinDirection(pin, req_data->dir)) {
      req_data->result = kGpioErrSys;
      return;
    }
    // Set mode.
    if (!SetPinMode(pin, req_data->mode)) {
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

  char value_path[GPIO_PATH_BUFFER_SIZE] = { 0 };
  snprintf(value_path, GPIO_PATH_BUFFER_SIZE - 1, GPIO_PIN_FORMAT_VALUE,
           req_data->pin);

  char buffer[2] = { 0 };
  buffer[0] = req_data->value ? '1' : '0';

  if (DeviceOpenWriteClose(value_path, buffer)) {
    req_data->result = kGpioErrOk;
  } else {
    req_data->result = kGpioErrSys;
  }
}


void ReadGpioWorker(uv_work_t* work_req) {
  GPIO_WORKER_INIT_TEMPLATE(true);
  uint32_t pin = req_data->pin;
  DDDLOG("Gpio ReadGpioWorker() - pin: %d", pin);

  char value_path[GPIO_PATH_BUFFER_SIZE] = { 0 };
  snprintf(value_path, GPIO_PATH_BUFFER_SIZE - 1, GPIO_PIN_FORMAT_VALUE, pin);

  char buffer[GPIO_VALUE_BUFFER_SIZE];
  if (DeviceOpenReadClose(value_path, buffer, GPIO_VALUE_BUFFER_SIZE - 1)) {
    req_data->result = kGpioErrOk;
    req_data->value = atoi(buffer) ? true : false;
  } else {
    req_data->result = kGpioErrSys;
  }
}


#endif /* IOTJS_MODULE_GPIO_LINUX_GENERAL_INL_H */
