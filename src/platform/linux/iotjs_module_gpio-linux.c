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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "iotjs_systemio-linux.h"
#include "modules/iotjs_module_gpio.h"


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


static bool gpio_set_direction(uint32_t pin, GpioDirection direction) {
  IOTJS_ASSERT(direction == kGpioDirectionIn || direction == kGpioDirectionOut);

  char direction_path[GPIO_PATH_BUFFER_SIZE];
  snprintf(direction_path, GPIO_PATH_BUFFER_SIZE, GPIO_PIN_FORMAT_DIRECTION,
           pin);

  const char* buffer = (direction == kGpioDirectionIn) ? "in" : "out";

  DDDLOG("%s - path: %s, dir: %s", __func__, direction_path, buffer);

  return iotjs_systemio_open_write_close(direction_path, buffer);
}


// FIXME: Implement SetPinMode()
static bool gpio_set_mode(uint32_t pin, GpioMode mode) {
  return true;
}


bool iotjs_gpio_write(iotjs_gpio_t* gpio, bool value) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_gpio_t, gpio);

  char value_path[GPIO_PATH_BUFFER_SIZE];
  snprintf(value_path, GPIO_PATH_BUFFER_SIZE, GPIO_PIN_FORMAT_VALUE,
           _this->pin);

  const char* buffer = value ? "1" : "0";

  DDDLOG("%s - pin: %d, value: %d", __func__, _this->pin, value);

  return iotjs_systemio_open_write_close(value_path, buffer);
}


int iotjs_gpio_read(iotjs_gpio_t* gpio) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_gpio_t, gpio);

  char buffer[GPIO_VALUE_BUFFER_SIZE];
  char value_path[GPIO_PATH_BUFFER_SIZE];
  snprintf(value_path, GPIO_PATH_BUFFER_SIZE, GPIO_PIN_FORMAT_VALUE,
           _this->pin);

  if (!iotjs_systemio_open_read_close(value_path, buffer,
                                      GPIO_VALUE_BUFFER_SIZE - 1)) {
    return -1;
  }

  return atoi(buffer);
}


bool iotjs_gpio_close(iotjs_gpio_t* gpio) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_gpio_t, gpio);

  char buff[GPIO_PIN_BUFFER_SIZE];
  snprintf(buff, GPIO_PIN_BUFFER_SIZE, "%d", _this->pin);

  return iotjs_systemio_open_write_close(GPIO_PIN_FORMAT_UNEXPORT, buff);
}


void iotjs_gpio_open_worker(uv_work_t* work_req) {
  GPIO_WORKER_INIT;
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_gpio_t, gpio);

  DDDLOG("%s - pin: %d, dir: %d, mode: %d", __func__, _this->pin,
         _this->direction, _this->mode);

  // Open GPIO pin.
  char exported_path[GPIO_PATH_BUFFER_SIZE];
  snprintf(exported_path, GPIO_PATH_BUFFER_SIZE, GPIO_PIN_FORMAT, _this->pin);

  const char* created_files[] = { GPIO_DIRECTION, GPIO_EDGE, GPIO_VALUE };
  int created_files_length = sizeof(created_files) / sizeof(created_files[0]);

  if (!iotjs_systemio_device_open(GPIO_PIN_FORMAT_EXPORT, _this->pin,
                                  exported_path, created_files,
                                  created_files_length)) {
    req_data->result = false;
    return;
  }
  // Set direction.
  if (!gpio_set_direction(_this->pin, _this->direction)) {
    req_data->result = false;
    return;
  }
  // Set mode.
  if (!gpio_set_mode(_this->pin, _this->mode)) {
    req_data->result = false;
    return;
  }

  req_data->result = true;
}
