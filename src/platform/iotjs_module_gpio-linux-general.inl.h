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

#include "iotjs_module_gpio.h"


#define GPIO_INTERFACE "/sys/class/gpio/"
#define GPIO_EXPORT GPIO_INTERFACE "export"
#define GPIO_UNEXPORT GPIO_INTERFACE "unexport"
#define GPIO_PIN_FORMAT "/sys/class/gpio/gpio%d/"
#define GPIO_PIN_FORMAT_DIRECTION GPIO_PIN_FORMAT "direction"
#define GPIO_PIN_FORMAT_EDGE GPIO_PIN_FORMAT "edge"
#define GPIO_PIN_FORMAT_VALUE GPIO_PIN_FORMAT "value"

#define GPIO_MAX_PINNO 63

namespace iotjs {


// Generic GPIO implementation for linux.
// Implementaion used here are based on:
//  https://www.kernel.org/doc/Documentation/gpio/sysfs.txt
class GpioLinuxGeneral : public Gpio {
 public:
  explicit GpioLinuxGeneral(JObject& jgpio);

  static GpioLinuxGeneral* GetInstance();

  virtual int Initialize(GpioReqWrap* gpio_req);
  virtual int Release(GpioReqWrap* gpio_req);
  virtual int SetPin(GpioReqWrap* gpio_req);
  virtual int WritePin(GpioReqWrap* gpio_req);
  virtual int ReadPin(GpioReqWrap* gpio_req);

 public:
  bool _initialized;
  bool _exported_by_this[GPIO_MAX_PINNO + 1];
};


GpioLinuxGeneral::GpioLinuxGeneral(JObject& jgpio)
    : Gpio(jgpio)
    , _initialized(false) {
  for (int i = 0; i < GPIO_MAX_PINNO; ++i) {
    _exported_by_this[i] = false;
  }
}


GpioLinuxGeneral* GpioLinuxGeneral::GetInstance() {
  return static_cast<GpioLinuxGeneral*>(Gpio::GetInstance());
}


// uv fs request wrapper for auto cleanup.
class LocalFsReq : public uv_fs_t {
 public:
  ~LocalFsReq() {
    uv_fs_req_cleanup(this);
  }
};


// Checks if given directory exits.
bool CheckPath(const char* path) {
  Environment* env = Environment::GetEnv();

  DDDLOG("GPIO CheckPath() - path: %s", path);

  // stat for the path.
  LocalFsReq fs_req;
  int err = uv_fs_stat(env->loop(), &fs_req, path, NULL);

  // exist?
  if (err || fs_req.result) {
    return false;
  }

  DDDLOG("GPIO CheckPath() - path exist");

  return true;
}


bool GpioOpenWriteClose(const char* path, char* value) {
  Environment* env = Environment::GetEnv();

  DDDLOG("GPIO GpioOpenWriteClose() - path %s, value: %s", path, value);

  // Open file.
  LocalFsReq fs_open_req;
  int fd = uv_fs_open(env->loop(), &fs_open_req, path, O_WRONLY, 0666, NULL);
  if (fd < 0) {
    DDLOG("GPIO GpioOpenWriteClose() - open %s failed: %d", path, fd);
    return false;
  }

  // Write value.
  LocalFsReq fs_write_req;
  uv_buf_t uvbuf = uv_buf_init(value, strlen(value));
  int err = uv_fs_write(env->loop(), &fs_write_req, fd, &uvbuf, 1, 0, NULL);
  if (err < 0) {
    DDLOG("GPIO GpioOpenWriteClose() - write %s failed: %d", value, err);
    return false;
  }

  // Close file.
  LocalFsReq fs_close_req;
  err = uv_fs_close(env->loop(), &fs_close_req, fd, NULL);
  if (err < 0) {
    DDLOG("GPIO GpioOpenWriteClose() - close failed: %d", err);
    return false;
  }

  return true;
}


bool GpioOpenReadClose(const char* path, char* buffer, int buffer_len) {
  Environment* env = Environment::GetEnv();

  DDDLOG("GPIO GpioOpenReadClose() - path %s", path);

  // Open file.
  LocalFsReq fs_open_req;
  int fd = uv_fs_open(env->loop(), &fs_open_req, path, O_RDONLY, 0666, NULL);
  if (fd < 0) {
    DDLOG("GPIO GpioOpenReadClose() - open %s failed: %d", path, fd);
    return false;
  }

  // Write value.
  LocalFsReq fs_write_req;
  uv_buf_t uvbuf = uv_buf_init(buffer, buffer_len);
  int err = uv_fs_read(env->loop(), &fs_write_req, fd, &uvbuf, 1, 0, NULL);
  if (err < 0) {
    DDLOG("GPIO GpioOpenReadClose() - read failed: %d", err);
    return false;
  }

  DDDLOG("GPIO GpioOpenReadClose() - read value: %s", buffer);

  // Close file.
  LocalFsReq fs_close_req;
  err = uv_fs_close(env->loop(), &fs_close_req, fd, NULL);
  if (err < 0) {
    DDLOG("GPIO GpioOpenReadClose() - close failed: %d", err);
    return false;
  }

  return true;
}


// Export GPIO pin.
bool ExportPin(int32_t pin) {
  IOTJS_ASSERT(pin >= 0 && pin < GPIO_MAX_PINNO);

  DDDLOG("GPIO ExportPin() - pin: %d", pin);

  // See if the pin is already opend.
  char path[64] = {0};
  snprintf(path, 63, GPIO_PIN_FORMAT, pin);
  if (CheckPath(path)) {
    DDDLOG("GPIO ExportPin() - path: %s exist", path);
    return true;
  }

  // Write export pin.
  char buff[10] = {0};
  snprintf(buff, 9, "%d", pin);

  if (GpioOpenWriteClose(GPIO_EXPORT, buff)) {
    GpioLinuxGeneral* gpio = GpioLinuxGeneral::GetInstance();
    gpio->_exported_by_this[pin] = true;
  } else {
    return false;
  }

  // Wait for gpio pin directory creation.
  int count = 0;
  int count_limit = 50;
  char buffer[10];
  while (!CheckPath(path) && count < count_limit) {
    usleep(100 * 1000); // sleep 100 miliseconds.
    count++;
  }
  snprintf(path, 63, GPIO_PIN_FORMAT_DIRECTION, pin);
  while (!GpioOpenReadClose(path, buffer, 10) && count < count_limit) {
    usleep(100 * 1000); // sleep 100 miliseconds.
    count++;
  }
  snprintf(path, 63, GPIO_PIN_FORMAT_EDGE, pin);
  while (!GpioOpenReadClose(path, buffer, 10) && count < count_limit) {
    usleep(100 * 1000); // sleep 100 miliseconds.
    count++;
  }
  snprintf(path, 63, GPIO_PIN_FORMAT_VALUE, pin);
  while (!GpioOpenReadClose(path, buffer, 10) && count < count_limit) {
    usleep(100 * 1000); // sleep 100 miliseconds.
    count++;
  }
  usleep(1000 * 100); // sleep another 1000 milisec.

  return count < count_limit;
}


// Unexport GPIO pin.
bool UnexportPin(int32_t pin) {
  IOTJS_ASSERT(pin >= 0 && pin < GPIO_MAX_PINNO);

  DDDLOG("GPIO UnexportPin() - path: %s", GPIO_UNEXPORT);

  char buff[10] = {0};
  snprintf(buff, 9, "%d", pin);

  if (GpioOpenWriteClose(GPIO_EXPORT, buff)) {
    GpioLinuxGeneral* gpio = GpioLinuxGeneral::GetInstance();
    gpio->_exported_by_this[pin] = false;
    return true;
  } else {
    return false;
  }
}


bool SetPinDirection(int32_t pin, GpioDirection direction) {
  IOTJS_ASSERT(pin >= 0 && pin < GPIO_MAX_PINNO);
  IOTJS_ASSERT(direction == kGpioDirectionIn || direction == kGpioDirectionOut);

  char direction_path[64] = {0};
  snprintf(direction_path, 63, GPIO_PIN_FORMAT_DIRECTION, pin);

  char buffer[4] = {0};
  if (direction == kGpioDirectionIn) {
    strcpy(buffer, "in");
  } else {
    strcpy(buffer, "out");
  }

  DDDLOG("GPIO SetPinDirection() - path: %s, dir: %s", direction_path, buffer);

  return GpioOpenWriteClose(direction_path, buffer);
}


// FIXME: Implement SetPinMode()
bool SetPinMode(int32_t pin, GpioMode mode) {
  IOTJS_ASSERT(pin >= 0 && pin < GPIO_MAX_PINNO);
  IOTJS_ASSERT(mode == kGpioModeNone || mode == kGpioModeOpendrain);

  return true;
}


bool WritePin(int32_t pin, bool value) {
  IOTJS_ASSERT(pin >= 0 && pin < GPIO_MAX_PINNO);

  char value_path[64] = {0};
  snprintf(value_path, 63, GPIO_PIN_FORMAT_VALUE, pin);

  DDDLOG("GPIO WritePin() - path: %s value: %d", value_path, value);

  char buffer[2] = {0};
  buffer[0] = value ? '1' : '0';
  return GpioOpenWriteClose(value_path, buffer);
}


bool ReadPin(int32_t pin, int32_t* value) {
  IOTJS_ASSERT(pin >= 0 && pin < GPIO_MAX_PINNO);

  char value_path[64] = {0};
  snprintf(value_path, 63, GPIO_PIN_FORMAT_VALUE, pin);

  DDDLOG("GPIO ReadPin() - path: %s", value_path);

  char buffer[10] = {0};
  if (GpioOpenReadClose(value_path, buffer, 9)) {
    *value = atoi(buffer) ? 1 : 0;
    return true;
  } else {
    return false;
  }
}


void AfterWork(uv_work_t* work_req, int status) {
  GpioLinuxGeneral* gpio = GpioLinuxGeneral::GetInstance();

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
  GpioLinuxGeneral* gpio = GpioLinuxGeneral::GetInstance();
  IOTJS_ASSERT(gpio->_initialized == false);

  GpioReqWrap* gpio_req = reinterpret_cast<GpioReqWrap*>(work_req->data);
  GpioReqData* req_data = gpio_req->req();

  DDDLOG("GPIO InitializeWorker()");

  // Check if GPIO interface exits.
  if (CheckPath(GPIO_INTERFACE)) {
    req_data->result = kGpioErrOk;
  } else {
    req_data->result = kGpioErrInitialize;
  }
}


void ReleaseWorker(uv_work_t* work_req) {
  GpioLinuxGeneral* gpio = GpioLinuxGeneral::GetInstance();
  IOTJS_ASSERT(gpio->_initialized == true);

  GpioReqWrap* gpio_req = reinterpret_cast<GpioReqWrap*>(work_req->data);
  GpioReqData* req_data = gpio_req->req();

  DDDLOG("GPIO ReleaseWorker()");

  req_data->result = kGpioErrOk;

  // Unexport pins that exported earlier by this module.
  for (int i = 0; i < GPIO_MAX_PINNO; ++i) {
    if (gpio->_exported_by_this[i]) {
      if (!UnexportPin(i)) {
        req_data->result = kGpioErrSys;
      }
    }
  }
}


void SetPinWorker(uv_work_t* work_req) {
  GpioLinuxGeneral* gpio = GpioLinuxGeneral::GetInstance();
  IOTJS_ASSERT(gpio->_initialized == true);

  GpioReqWrap* gpio_req = reinterpret_cast<GpioReqWrap*>(work_req->data);
  GpioReqData* req_data = gpio_req->req();

  DDDLOG("GPIO SetPinWorker() - pin: %d, dir: %d, mode: %d",
         req_data->pin, req_data->dir, req_data->mode);

  if (req_data->dir == kGpioDirectionNone) {
    // Unexport GPIO pin.
    if (!UnexportPin(req_data->pin)) {
      req_data->result = kGpioErrSys;
      return;
    }
  } else {
    // Export GPIO pin.
    if (!ExportPin(req_data->pin)) {
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


void WritePinWorker(uv_work_t* work_req) {
  GpioLinuxGeneral* gpio = GpioLinuxGeneral::GetInstance();
  IOTJS_ASSERT(gpio->_initialized == true);

  GpioReqWrap* gpio_req = reinterpret_cast<GpioReqWrap*>(work_req->data);
  GpioReqData* req_data = gpio_req->req();

  DDDLOG("GPIO WritePinWorker() - pin: %d, value: %d",
         req_data->pin, req_data->value);

  if (WritePin(req_data->pin, req_data->value)) {
    req_data->result = kGpioErrOk;
  } else {
    req_data->result = kGpioErrSys;
  }
}


void ReadPinWorker(uv_work_t* work_req) {
  GpioLinuxGeneral* gpio = GpioLinuxGeneral::GetInstance();
  IOTJS_ASSERT(gpio->_initialized == true);

  GpioReqWrap* gpio_req = reinterpret_cast<GpioReqWrap*>(work_req->data);
  GpioReqData* req_data = gpio_req->req();

  DDDLOG("GPIO ReadPinWorker() - pin: %d" ,req_data->pin);

  if (ReadPin(req_data->pin, &req_data->value)) {
    req_data->result = kGpioErrOk;
  } else {
    req_data->result = kGpioErrSys;
  }
}


#define GPIO_LINUX_GENERAL_IMPL_TEMPLATE(op, initialized) \
  do { \
    GpioLinuxGeneral* gpio = GpioLinuxGeneral::GetInstance(); \
    IOTJS_ASSERT(gpio->_initialized == initialized); \
    Environment* env = Environment::GetEnv(); \
    uv_work_t* req = new uv_work_t; \
    req->data = reinterpret_cast<void*>(gpio_req); \
    uv_queue_work(env->loop(), req, op ## Worker, AfterWork); \
  } while (0)


int GpioLinuxGeneral::Initialize(GpioReqWrap* gpio_req) {
  GPIO_LINUX_GENERAL_IMPL_TEMPLATE(Initialize, false);
  return 0;
}


int GpioLinuxGeneral::Release(GpioReqWrap* gpio_req) {
  GPIO_LINUX_GENERAL_IMPL_TEMPLATE(Release, true);
  return 0;
}


int GpioLinuxGeneral::SetPin(GpioReqWrap* gpio_req) {
  GPIO_LINUX_GENERAL_IMPL_TEMPLATE(SetPin, true);
  return 0;
}


int GpioLinuxGeneral::WritePin(GpioReqWrap* gpio_req) {
  GPIO_LINUX_GENERAL_IMPL_TEMPLATE(WritePin, true);
  return 0;
}


int GpioLinuxGeneral::ReadPin(GpioReqWrap* gpio_req) {
  GPIO_LINUX_GENERAL_IMPL_TEMPLATE(ReadPin, true);
  return 0;
}


} // namespace iotjs


#endif /* IOTJS_MODULE_GPIO_LINUX_GENERAL_INL_H */
