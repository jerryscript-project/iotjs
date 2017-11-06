/* Copyright 2017-present Samsung Electronics Co., Ltd. and other contributors
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

#include "modules/iotjs_module_i2c.h"
#include <peripheral_io.h>
#include <stdint.h>


#define I2C_WORKER_INIT_TEMPLATE                                            \
  iotjs_i2c_reqwrap_t* req_wrap = iotjs_i2c_reqwrap_from_request(work_req); \
  iotjs_i2c_reqdata_t* req_data = iotjs_i2c_reqwrap_data(req_wrap);

#define IOTJS_I2C_METHOD_HEADER(arg)              \
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_i2c_t, arg) \
  iotjs_i2c_platform_data_t* platform_data = _this->platform_data;

typedef struct _peripheral_i2c_s* peripheral_i2c_h;

struct iotjs_i2c_platform_data_s {
  int bus;
  int address;
  peripheral_i2c_h handle;
};

void i2c_create_platform_data(iotjs_jhandler_t* jhandler, iotjs_i2c_t* i2c,
                              iotjs_i2c_platform_data_t** ppdata) {
  iotjs_i2c_platform_data_t* pdata = IOTJS_ALLOC(iotjs_i2c_platform_data_t);

  // TODO: consider allowing one step init: new I2C.open(bus, address, callback)
  // as opposed to current new I2C.open(bus, callback)
  DJHANDLER_CHECK_ARGS(2, number, function);
  pdata->bus = JHANDLER_GET_ARG(0, number);

  pdata->address = -1;
  pdata->handle = NULL;
  // Note: address still unset, presumably will be
  // done by callback invoked by AfterI2CWork.
  *ppdata = pdata;
}

void i2c_destroy_platform_data(iotjs_i2c_platform_data_t* pdata) {
  (void)pdata;
}

// The address can be set just once.
void I2cSetAddress(iotjs_i2c_t* i2c, uint8_t address) {
  IOTJS_I2C_METHOD_HEADER(i2c);

  if (platform_data->address == -1) {
    // Perform delayed construction.
    platform_data->address = address;
    if (peripheral_i2c_open(platform_data->bus, platform_data->address,
                            &platform_data->handle) < 0) {
      // TODO: report the error at this point
    }
  } else if (platform_data->address != address) {
    // TODO: report error OR recreate the object with new slave address
  }
}

void OpenWorker(uv_work_t* work_req) {
  I2C_WORKER_INIT_TEMPLATE;
  // Tizen does not allow to control the master, just individual devices.
  // Because of this, construction is delayed until the address is known.
  req_data->error = kI2cErrOk;
}

void I2cClose(iotjs_i2c_t* i2c) {
  IOTJS_I2C_METHOD_HEADER(i2c);

  if (platform_data->handle != NULL) {
    peripheral_i2c_close(platform_data->handle);
    platform_data->handle = NULL;
  }
}

void WriteWorker(uv_work_t* work_req) {
  I2C_WORKER_INIT_TEMPLATE;
  iotjs_i2c_t* i2c = iotjs_i2c_instance_from_reqwrap(req_wrap);
  IOTJS_I2C_METHOD_HEADER(i2c);

  req_data->error = kI2cErrOk;
  if (peripheral_i2c_write(platform_data->handle,
                           (unsigned char*)req_data->buf_data,
                           req_data->buf_len) < 0) {
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

  req_data->error = kI2cErrOk;
  if (peripheral_i2c_read(platform_data->handle,
                          (unsigned char*)req_data->buf_data,
                          req_data->buf_len) < 0) {
    req_data->error = kI2cErrWrite;
  }
}
