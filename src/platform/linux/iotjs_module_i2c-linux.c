/* The MIT License (MIT)
 *
 * Copyright (c) 2005-2014 RoadNarrows LLC.
 * http://roadnarrows.com
 * All Rights Reserved
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/* Copyright 2016-present Samsung Electronics Co., Ltd. and other contributors
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


/* Some functions are modified from the RoadNarrows-robotics i2c library.
 * (distributed under the MIT license.)
 */


#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>


#include "modules/iotjs_module_i2c.h"


#define I2C_SLAVE_FORCE 0x0706


#define I2C_WORKER_INIT_TEMPLATE                                            \
  iotjs_i2c_reqwrap_t* req_wrap = iotjs_i2c_reqwrap_from_request(work_req); \
  iotjs_i2c_reqdata_t* req_data = iotjs_i2c_reqwrap_data(req_wrap);

#define IOTJS_I2C_METHOD_HEADER(arg)              \
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_i2c_t, arg) \
  iotjs_i2c_platform_data_t* platform_data = _this->platform_data;

struct iotjs_i2c_platform_data_s {
  iotjs_string_t device;
  int device_fd;
  uint8_t addr;
};

void i2c_create_platform_data(iotjs_jhandler_t* jhandler, iotjs_i2c_t* i2c,
                              iotjs_i2c_platform_data_t** ppdata) {
  iotjs_i2c_platform_data_t* pdata = IOTJS_ALLOC(iotjs_i2c_platform_data_t);

  pdata->device = iotjs_string_create("");
  DJHANDLER_CHECK_ARGS(2, string, function);
  iotjs_string_t device = JHANDLER_GET_ARG(0, string);
  iotjs_string_append(&pdata->device, iotjs_string_data(&device),
                      iotjs_string_size(&device));
  pdata->device_fd = -1;
  *ppdata = pdata;
}

void i2c_destroy_platform_data(iotjs_i2c_platform_data_t* pdata) {
  iotjs_string_destroy(&pdata->device);
  IOTJS_RELEASE(pdata);
}

void I2cSetAddress(iotjs_i2c_t* i2c, uint8_t address) {
  IOTJS_I2C_METHOD_HEADER(i2c);
  platform_data->addr = address;
  ioctl(platform_data->device_fd, I2C_SLAVE_FORCE, platform_data->addr);
}

void OpenWorker(uv_work_t* work_req) {
  I2C_WORKER_INIT_TEMPLATE;
  iotjs_i2c_t* i2c = iotjs_i2c_instance_from_reqwrap(req_wrap);
  IOTJS_I2C_METHOD_HEADER(i2c);

  platform_data->device_fd =
      open(iotjs_string_data(&platform_data->device), O_RDWR);

  if (platform_data->device_fd == -1) {
    req_data->error = kI2cErrOpen;
  } else {
    req_data->error = kI2cErrOk;
  }
}

void I2cClose(iotjs_i2c_t* i2c) {
  IOTJS_I2C_METHOD_HEADER(i2c);

  if (platform_data->device_fd >= 0) {
    close(platform_data->device_fd);
    platform_data->device_fd = -1;
  }
}

void WriteWorker(uv_work_t* work_req) {
  I2C_WORKER_INIT_TEMPLATE;
  iotjs_i2c_t* i2c = iotjs_i2c_instance_from_reqwrap(req_wrap);
  IOTJS_I2C_METHOD_HEADER(i2c);

  uint8_t len = req_data->buf_len;
  char* data = req_data->buf_data;

  if (write(platform_data->device_fd, data, len) != len) {
    req_data->error = kI2cErrWrite;
  }

  if (req_data->buf_data != NULL) {
    iotjs_buffer_release(req_data->buf_data);
  }
}

void ReadWorker(uv_work_t* work_req) {
  I2C_WORKER_INIT_TEMPLATE;
  iotjs_i2c_t* i2c = iotjs_i2c_instance_from_reqwrap(req_wrap);
  IOTJS_I2C_METHOD_HEADER(i2c);

  uint8_t len = req_data->buf_len;
  req_data->buf_data = iotjs_buffer_allocate(len);

  if (read(platform_data->device_fd, req_data->buf_data, len) != len) {
    req_data->error = kI2cErrRead;
  }
}
