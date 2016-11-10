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

#ifndef IOTJS_MODULE_GPIO_LINUX_GENERAL_INL_H
#define IOTJS_MODULE_GPIO_LINUX_GENERAL_INL_H


#include <string.h>
#include <stdlib.h>
#include <stdio.h>
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


namespace iotjs {


// Generic GPIO implementation for linux.
// Implementation used here are based on:
//  https://www.kernel.org/doc/Documentation/gpio/sysfs.txt
class GpioLinuxGeneral : public Gpio {
 public:
  explicit GpioLinuxGeneral(const iotjs_jval_t* jgpio);

  static GpioLinuxGeneral* GetInstance();

  virtual int Initialize(GpioReqWrap* gpio_req);
  virtual int Release(GpioReqWrap* gpio_req);
  virtual int Open(GpioReqWrap* gpio_req);
  virtual int Write(GpioReqWrap* gpio_req);
  virtual int Read(GpioReqWrap* gpio_req);

 public:
  bool _initialized;
};


GpioLinuxGeneral::GpioLinuxGeneral(const iotjs_jval_t* jgpio)
    : Gpio(jgpio)
    , _initialized(false) {
}


GpioLinuxGeneral* GpioLinuxGeneral::GetInstance() {
  return static_cast<GpioLinuxGeneral*>(Gpio::GetInstance());
}


// Unexport GPIO pin.
bool UnexportPin(int32_t pin) {
  DDDLOG("GPIO UnexportPin() - path: %s", GPIO_PIN_FORMAT_UNEXPORT);

  char buff[GPIO_PIN_BUFFER_SIZE] = {0};
  snprintf(buff, GPIO_PIN_BUFFER_SIZE - 1, "%d", pin);

  if (!DeviceOpenWriteClose(GPIO_PIN_FORMAT_UNEXPORT, buff)) {
    return false;
  }

  return true;
}


bool SetPinDirection(int32_t pin, GpioDirection direction) {
  IOTJS_ASSERT(direction == kGpioDirectionIn || direction == kGpioDirectionOut);

  char direction_path[GPIO_PATH_BUFFER_SIZE] = {0};
  snprintf(direction_path, GPIO_PATH_BUFFER_SIZE - 1,
           GPIO_PIN_FORMAT_DIRECTION, pin);

  char buffer[4] = {0};
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


void AfterWork(uv_work_t* work_req, int status) {
  GpioLinuxGeneral* gpio = GpioLinuxGeneral::GetInstance();

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
        iotjs_jargs_append_bool(&jargs, req_data->value);
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

  iotjs_jargs_destroy(&jargs);

  delete work_req;
  delete gpio_req;
}


void InitializeGpioWorker(uv_work_t* work_req) {
  GpioLinuxGeneral* gpio = GpioLinuxGeneral::GetInstance();
  IOTJS_ASSERT(gpio->_initialized == false);

  GpioReqWrap* gpio_req = reinterpret_cast<GpioReqWrap*>(work_req->data);
  GpioReqData* req_data = gpio_req->req();

  DDDLOG("GPIO InitializeGpioWorker()");

  // Check if GPIO interface exits.
  if (DeviceCheckPath(GPIO_INTERFACE)) {
    req_data->result = kGpioErrOk;
  } else {
    req_data->result = kGpioErrInitialize;
  }
}


void ReleaseGpioWorker(uv_work_t* work_req) {
  GpioLinuxGeneral* gpio = GpioLinuxGeneral::GetInstance();
  IOTJS_ASSERT(gpio->_initialized == true);

  GpioReqWrap* gpio_req = reinterpret_cast<GpioReqWrap*>(work_req->data);
  GpioReqData* req_data = gpio_req->req();

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
  GpioLinuxGeneral* gpio = GpioLinuxGeneral::GetInstance();
  IOTJS_ASSERT(gpio->_initialized == true);

  GpioReqWrap* gpio_req = reinterpret_cast<GpioReqWrap*>(work_req->data);
  GpioReqData* req_data = gpio_req->req();
  uint32_t pin = req_data->pin;

  DDDLOG("GPIO OpenGpioWorker() - pin: %d, dir: %d, mode: %d",
         req_data->pin, req_data->dir, req_data->mode);

  if (req_data->dir == kGpioDirectionNone) {
    // Unexport GPIO pin.
    if (!UnexportPin(req_data->pin)) {
      req_data->result = kGpioErrSys;
      return;
    }
  } else {

    // Export GPIO pin.
    char exported_path[GPIO_PATH_BUFFER_SIZE] = {0};
    snprintf(exported_path, GPIO_PATH_BUFFER_SIZE - 1, GPIO_PIN_FORMAT, pin);

    const char* created_files[] = {GPIO_DIRECTION, GPIO_EDGE, GPIO_VALUE};
    int created_files_length = sizeof(created_files) / sizeof(created_files[0]);

    if (!DeviceExport(GPIO_PIN_FORMAT_EXPORT, pin, exported_path, created_files,
                      created_files_length)) {
      req_data->result = kGpioErrSys;
      return;
    }
    // Set direction.
    if (!SetPinDirection(req_data->pin, req_data->dir)) {
      req_data->result = kGpioErrSys;
      return;
    }
    // Set mode.
    if (!SetPinMode(req_data->pin, req_data->mode)) {
      req_data->result = kGpioErrSys;
      return;
    }
  }

  req_data->result = kGpioErrOk;
}


void WriteGpioWorker(uv_work_t* work_req) {
  GpioLinuxGeneral* gpio = GpioLinuxGeneral::GetInstance();
  IOTJS_ASSERT(gpio->_initialized == true);

  GpioReqWrap* gpio_req = reinterpret_cast<GpioReqWrap*>(work_req->data);
  GpioReqData* req_data = gpio_req->req();

  uint32_t pin = req_data->pin;
  uint32_t value = req_data->value;

  DDDLOG("GPIO WriteGpioWorker() - pin: %d, value: %d",
         pin, value);

  char value_path[GPIO_PATH_BUFFER_SIZE] = {0};
  snprintf(value_path, GPIO_PATH_BUFFER_SIZE - 1,
           GPIO_PIN_FORMAT_VALUE, pin);

  char buffer[2] = {0};
  buffer[0] = value ? '1' : '0';

  if (DeviceOpenWriteClose(value_path, buffer)) {
    req_data->result = kGpioErrOk;
  } else {
    req_data->result = kGpioErrSys;
  }
}


void ReadGpioWorker(uv_work_t* work_req) {
  GpioLinuxGeneral* gpio = GpioLinuxGeneral::GetInstance();
  IOTJS_ASSERT(gpio->_initialized == true);

  GpioReqWrap* gpio_req = reinterpret_cast<GpioReqWrap*>(work_req->data);
  GpioReqData* req_data = gpio_req->req();

  uint32_t pin = req_data->pin;

  DDDLOG("GPIO ReadGpioWorker() - pin: %d" ,pin);

  char value_path[GPIO_PATH_BUFFER_SIZE] = {0};
  snprintf(value_path, GPIO_PATH_BUFFER_SIZE - 1, GPIO_PIN_FORMAT_VALUE, pin);

  char buffer[GPIO_VALUE_BUFFER_SIZE];
  if (DeviceOpenReadClose(value_path, buffer, GPIO_VALUE_BUFFER_SIZE - 1)) {
    req_data->value = atoi(buffer) ? 1 : 0;
    req_data->result = kGpioErrOk;
  } else {
    req_data->result = kGpioErrSys;
  }
}


#define GPIO_LINUX_GENERAL_IMPL_TEMPLATE(op, initialized) \
  do { \
    GpioLinuxGeneral* gpio = GpioLinuxGeneral::GetInstance(); \
    IOTJS_ASSERT(gpio->_initialized == initialized); \
    const iotjs_environment_t* env = iotjs_environment_get(); \
    uv_work_t* req = new uv_work_t; \
    req->data = reinterpret_cast<void*>(gpio_req); \
    uv_queue_work(iotjs_environment_loop(env), req, op ## GpioWorker, \
                  AfterWork); \
  } while (0)


int GpioLinuxGeneral::Initialize(GpioReqWrap* gpio_req) {
  GPIO_LINUX_GENERAL_IMPL_TEMPLATE(Initialize, false);
  return 0;
}


int GpioLinuxGeneral::Release(GpioReqWrap* gpio_req) {
  GPIO_LINUX_GENERAL_IMPL_TEMPLATE(Release, true);
  return 0;
}


int GpioLinuxGeneral::Open(GpioReqWrap* gpio_req) {
  GPIO_LINUX_GENERAL_IMPL_TEMPLATE(Open, true);
  return 0;
}


int GpioLinuxGeneral::Write(GpioReqWrap* gpio_req) {
  GPIO_LINUX_GENERAL_IMPL_TEMPLATE(Write, true);
  return 0;
}


int GpioLinuxGeneral::Read(GpioReqWrap* gpio_req) {
  GPIO_LINUX_GENERAL_IMPL_TEMPLATE(Read, true);
  return 0;
}


} // namespace iotjs


#endif /* IOTJS_MODULE_GPIO_LINUX_GENERAL_INL_H */
