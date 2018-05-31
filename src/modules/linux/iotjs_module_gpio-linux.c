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


#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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

struct iotjs_gpio_platform_data_s {
  int value_fd;
  uv_thread_t thread;
  uv_mutex_t mutex;
};

// Implementation used here are based on:
//  https://www.kernel.org/doc/Documentation/gpio/sysfs.txt


static const char* gpio_edge_string[] = { "none", "rising", "falling", "both" };


static int gpio_get_value_fd(iotjs_gpio_t* gpio) {
  int fd;

  uv_mutex_lock(&gpio->platform_data->mutex);
  fd = gpio->platform_data->value_fd;
  uv_mutex_unlock(&gpio->platform_data->mutex);

  return fd;
}


static void gpio_set_value_fd(iotjs_gpio_t* gpio, int fd) {
  uv_mutex_lock(&gpio->platform_data->mutex);
  gpio->platform_data->value_fd = fd;
  uv_mutex_unlock(&gpio->platform_data->mutex);
}


static void gpio_emit_change_event(iotjs_gpio_t* gpio) {
  jerry_value_t jgpio = gpio->jobject;
  jerry_value_t jonChange = iotjs_jval_get_property(jgpio, "onChange");
  IOTJS_ASSERT(jerry_value_is_function(jonChange));

  jerry_value_t jres = jerry_call_function(jonChange, jgpio, NULL, 0);
  IOTJS_ASSERT(!jerry_value_is_error(jres));

  jerry_release_value(jres);
  jerry_release_value(jonChange);
}


static bool gpio_clear_dummy_value(int fd) {
  char buffer[1];

  if (lseek(fd, 0, SEEK_SET) < 0) {
    DLOG("GPIO Error in lseek");
    return false;
  }

  if (read(fd, &buffer, 1) < 0) {
    DLOG("GPIO Error in read");
    return false;
  }

  return true;
}


static int gpio_edge_poll(struct pollfd* pollfd) {
  int ret;

  // Wait edge
  if ((ret = poll(pollfd, 1, -1)) > 0) {
    if (!gpio_clear_dummy_value(pollfd->fd))
      return -1;
  }

  return ret;
}


static void gpio_edge_detection_cb(void* data) {
  int fd;
  iotjs_gpio_t* gpio = (iotjs_gpio_t*)data;
  struct pollfd pollfd;

  if ((fd = gpio_get_value_fd(gpio)) < 0) {
    DLOG("GPIO Error: cannot start edge detection");
    return;
  }

  memset(&pollfd, 0, sizeof(pollfd));
  pollfd.fd = fd;
  pollfd.events = POLLPRI | POLLERR;

  if (!gpio_clear_dummy_value(fd))
    return;

  while (true) {
    if ((fd = gpio_get_value_fd(gpio)) < 0)
      return;

    if (gpio_edge_poll(&pollfd) > 0) {
      gpio_emit_change_event(gpio);
    } else {
      DLOG("GPIO Error on poll: %s", strerror(errno));
    }
  }
}


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


static bool gpio_set_edge(iotjs_gpio_t* gpio) {
  char edge_path[GPIO_PATH_BUFFER_SIZE];
  snprintf(edge_path, GPIO_PATH_BUFFER_SIZE, GPIO_PIN_FORMAT_EDGE, gpio->pin);
  iotjs_systemio_open_write_close(edge_path, gpio_edge_string[gpio->edge]);

  if (gpio->direction == kGpioDirectionIn && gpio->edge != kGpioEdgeNone) {
    char value_path[GPIO_PATH_BUFFER_SIZE];
    snprintf(value_path, GPIO_PATH_BUFFER_SIZE, GPIO_PIN_FORMAT_VALUE,
             gpio->pin);
    if ((gpio->platform_data->value_fd = open(value_path, O_RDONLY)) < 0) {
      DLOG("GPIO Error in open");
      return false;
    }


    // Create edge detection thread
    // When the GPIO pin is closed, thread is terminated.
    int ret = uv_thread_create(&gpio->platform_data->thread,
                               gpio_edge_detection_cb, (void*)gpio);
    if (ret < 0) {
      DLOG("GPIO Error in uv_thread_create");
    }
    return false;
  }

  return true;
}


void iotjs_gpio_create_platform_data(iotjs_gpio_t* gpio) {
  gpio->platform_data = IOTJS_ALLOC(iotjs_gpio_platform_data_t);
  gpio->platform_data->value_fd = -1;
  uv_mutex_init(&gpio->platform_data->mutex);
}


void iotjs_gpio_destroy_platform_data(
    iotjs_gpio_platform_data_t* platform_data) {
  IOTJS_RELEASE(platform_data);
}


bool iotjs_gpio_write(iotjs_gpio_t* gpio) {
  char value_path[GPIO_PATH_BUFFER_SIZE];
  snprintf(value_path, GPIO_PATH_BUFFER_SIZE, GPIO_PIN_FORMAT_VALUE, gpio->pin);

  const char* buffer = gpio->value ? "1" : "0";

  DDDLOG("%s - pin: %d, value: %d", __func__, gpio->pin, gpio->value);

  return iotjs_systemio_open_write_close(value_path, buffer);
}


bool iotjs_gpio_read(iotjs_gpio_t* gpio) {
  char buffer[GPIO_VALUE_BUFFER_SIZE];
  char value_path[GPIO_PATH_BUFFER_SIZE];
  snprintf(value_path, GPIO_PATH_BUFFER_SIZE, GPIO_PIN_FORMAT_VALUE, gpio->pin);

  if (!iotjs_systemio_open_read_close(value_path, buffer,
                                      GPIO_VALUE_BUFFER_SIZE - 1)) {
    return false;
  }

  gpio->value = atoi(buffer) != 0;

  return true;
}


bool iotjs_gpio_close(iotjs_gpio_t* gpio) {
  char buff[GPIO_PIN_BUFFER_SIZE];
  snprintf(buff, GPIO_PIN_BUFFER_SIZE, "%d", gpio->pin);

  gpio_set_value_fd(gpio, -1);
  close(gpio->platform_data->value_fd);

  return iotjs_systemio_open_write_close(GPIO_PIN_FORMAT_UNEXPORT, buff);
}


bool iotjs_gpio_open(iotjs_gpio_t* gpio) {
  DDDLOG("%s - pin: %d, dir: %d, mode: %d", __func__, gpio->pin,
         gpio->direction, gpio->mode);

  // Open GPIO pin.
  char exported_path[GPIO_PATH_BUFFER_SIZE];
  snprintf(exported_path, GPIO_PATH_BUFFER_SIZE, GPIO_PIN_FORMAT, gpio->pin);

  const char* created_files[] = { GPIO_DIRECTION, GPIO_EDGE, GPIO_VALUE };
  int created_files_length = sizeof(created_files) / sizeof(created_files[0]);

  if (!iotjs_systemio_device_open(GPIO_PIN_FORMAT_EXPORT, gpio->pin,
                                  exported_path, created_files,
                                  created_files_length)) {
    return false;
  }

  // Set direction.
  if (!gpio_set_direction(gpio->pin, gpio->direction)) {
    return false;
  }

  // Set mode.
  if (!gpio_set_mode(gpio->pin, gpio->mode)) {
    return false;
  }

  // Set edge.
  if (!gpio_set_edge(gpio)) {
    return false;
  }

  return true;
}


bool iotjs_gpio_set_direction(iotjs_gpio_t* gpio) {
  if (!gpio_set_direction(gpio->pin, gpio->direction)) {
    DLOG("%s, Cannot set direction.", __func__);
    return false;
  }
  return true;
}
