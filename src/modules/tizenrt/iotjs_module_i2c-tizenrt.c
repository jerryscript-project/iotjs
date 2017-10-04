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

#if !defined(__TIZENRT__)
#error "Module __FILE__ is for TizenRT only"
#endif

#include <tinyara/config.h>

#if !defined(CONFIG_I2C)
#error "\n\nTizenRT CONFIG_I2C configuration flag required for I2C module\n\n"
#endif

#include <iotbus_i2c.h>

#include "modules/iotjs_module_i2c.h"


struct iotjs_i2c_platform_data_s {
  int bus;
  iotbus_i2c_context_h i2c_context;
};


void i2c_create_platform_data(iotjs_jhandler_t* jhandler, iotjs_i2c_t* i2c,
                              iotjs_i2c_platform_data_t** ppdata) {
  iotjs_i2c_platform_data_t* pdata = IOTJS_ALLOC(iotjs_i2c_platform_data_t);

  DJHANDLER_CHECK_ARGS(2, number, function);
  pdata->bus = JHANDLER_GET_ARG(0, number);
  pdata->i2c_context = NULL;
  *ppdata = pdata;
}


void i2c_destroy_platform_data(iotjs_i2c_platform_data_t* pdata) {
  IOTJS_ASSERT(pdata);
  IOTJS_RELEASE(pdata);
}


#define I2C_WORKER_INIT_TEMPLATE                                            \
  iotjs_i2c_reqwrap_t* req_wrap = iotjs_i2c_reqwrap_from_request(work_req); \
  iotjs_i2c_reqdata_t* req_data = iotjs_i2c_reqwrap_data(req_wrap);


void I2cSetAddress(iotjs_i2c_t* i2c, uint8_t address) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_i2c_t, i2c)
  iotjs_i2c_platform_data_t* pdata = _this->platform_data;
  IOTJS_ASSERT(pdata);
  IOTJS_ASSERT(pdata->i2c_context);

  if (iotbus_i2c_set_address(pdata->i2c_context, address) < 0) {
    DLOG("%s: cannot set address", __func__);
    IOTJS_ASSERT(0);
  }
}


void OpenWorker(uv_work_t* work_req) {
  I2C_WORKER_INIT_TEMPLATE;
  iotjs_i2c_t* i2c = iotjs_i2c_instance_from_reqwrap(req_wrap);
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_i2c_t, i2c)
  iotjs_i2c_platform_data_t* pdata = _this->platform_data;

  IOTJS_ASSERT(pdata);

  // Init i2c context
  pdata->i2c_context = iotbus_i2c_init(pdata->bus);
  if (!pdata->i2c_context) {
    DLOG("%s: cannot open I2C", __func__);
    req_data->error = kI2cErrOpen;
    return;
  }

  // Set i2c frequency
  int ret = iotbus_i2c_set_frequency(pdata->i2c_context, IOTBUS_I2C_STD);
  if (ret < 0) {
    DLOG("%s: cannot set frequency", __func__);
    req_data->error = kI2cErrOpen;
    return;
  }

  req_data->error = kI2cErrOk;
}


void I2cClose(iotjs_i2c_t* i2c) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_i2c_t, i2c);
  iotjs_i2c_platform_data_t* pdata = _this->platform_data;

  if (iotbus_i2c_stop(pdata->i2c_context) < 0) {
    DLOG("%s: cannot close I2C", __func__);
    IOTJS_ASSERT(0);
  }
}


void WriteWorker(uv_work_t* work_req) {
  I2C_WORKER_INIT_TEMPLATE;
  iotjs_i2c_t* i2c = iotjs_i2c_instance_from_reqwrap(req_wrap);
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_i2c_t, i2c)
  iotjs_i2c_platform_data_t* pdata = _this->platform_data;

  uint8_t len = req_data->buf_len;
  uint8_t* data = (uint8_t*)req_data->buf_data;

  IOTJS_ASSERT(pdata);
  IOTJS_ASSERT(pdata->i2c_context);
  IOTJS_ASSERT(len > 0);

  if (iotbus_i2c_write(pdata->i2c_context, data, len) < 0) {
    DLOG("%s: cannot write data", __func__);
    req_data->error = kI2cErrWrite;
  } else {
    req_data->error = kI2cErrOk;
  }

  iotjs_buffer_release(req_data->buf_data);
}


void ReadWorker(uv_work_t* work_req) {
  I2C_WORKER_INIT_TEMPLATE;
  iotjs_i2c_t* i2c = iotjs_i2c_instance_from_reqwrap(req_wrap);
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_i2c_t, i2c)
  iotjs_i2c_platform_data_t* pdata = _this->platform_data;

  uint8_t len = req_data->buf_len;
  req_data->buf_data = iotjs_buffer_allocate(len);

  IOTJS_ASSERT(pdata);
  IOTJS_ASSERT(pdata->i2c_context);
  IOTJS_ASSERT(len > 0);

  if (iotbus_i2c_read(pdata->i2c_context, (uint8_t*)req_data->buf_data, len) <
      0) {
    DLOG("%s: cannot read data", __func__);
    req_data->error = kI2cErrRead;
    return;
  }

  req_data->error = kI2cErrOk;
}
