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

#if defined(__NUTTX__)


#include "module/iotjs_module_i2c.h"
#include "iotjs_systemio-arm-nuttx.h"


#define I2C_DEFAULT_FREQUENCY 400000


void I2cSetAddress(iotjs_i2c_t* i2c, uint8_t address) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_i2c_t, i2c);
  _this->config.address = address;
  _this->config.addrlen = 7;
}


#define I2C_WORKER_INIT_TEMPLATE                                            \
  iotjs_i2c_reqwrap_t* req_wrap = iotjs_i2c_reqwrap_from_request(work_req); \
  iotjs_i2c_reqdata_t* req_data = iotjs_i2c_reqwrap_data(req_wrap);


void ScanWorker(uv_work_t* work_req) {
  IOTJS_ASSERT(!"Not implemented");
}


void OpenWorker(uv_work_t* work_req) {
  I2C_WORKER_INIT_TEMPLATE;
  iotjs_i2c_t* i2c = iotjs_i2c_instance_from_reqwrap(req_wrap);

  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_i2c_t, i2c);
  _this->i2c_master = iotjs_i2c_config_nuttx(req_data->device);
  if (!_this->i2c_master) {
    DDLOG("I2C OpenWorker : cannot open");
    req_data->error = kI2cErrOpen;
    return;
  }

  _this->config.frequency = I2C_DEFAULT_FREQUENCY;

  req_data->error = kI2cErrOk;
}


void I2cClose(iotjs_i2c_t* i2c) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_i2c_t, i2c);

  iotjs_i2c_unconfig_nuttx(_this->i2c_master);
}


void WriteWorker(uv_work_t* work_req) {
  I2C_WORKER_INIT_TEMPLATE;
  iotjs_i2c_t* i2c = iotjs_i2c_instance_from_reqwrap(req_wrap);
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_i2c_t, i2c);

  uint8_t len = req_data->buf_len;
  uint8_t* data = (uint8_t*)req_data->buf_data;

  IOTJS_ASSERT(!_this->i2c_master);
  IOTJS_ASSERT(len > 0);

  int ret = i2c_write(_this->i2c_master, &_this->config, data, len);
  if (ret < 0) {
    DDLOG("I2C WriteWorker : cannot write - %d", ret);
    req_data->error = kI2cErrWrite;
  } else {
    req_data->error = kI2cErrOk;
  }

  if (req_data->buf_data != NULL) {
    iotjs_buffer_release(req_data->buf_data);
  }

  req_data->error = kI2cErrOk;
}


void WriteByteWorker(uv_work_t* work_req) {
  I2C_WORKER_INIT_TEMPLATE;
  iotjs_i2c_t* i2c = iotjs_i2c_instance_from_reqwrap(req_wrap);
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_i2c_t, i2c);

  IOTJS_ASSERT(!_this->i2c_master);

  int ret = i2c_write(_this->i2c_master, &_this->config, &req_data->byte, 1);
  if (ret < 0) {
    DDLOG("I2C WriteByteWorker : cannot write - %d", ret);
    req_data->error = kI2cErrWrite;
    return;
  }
  req_data->error = kI2cErrOk;
}


void WriteBlockWorker(uv_work_t* work_req) {
  I2C_WORKER_INIT_TEMPLATE;
  iotjs_i2c_t* i2c = iotjs_i2c_instance_from_reqwrap(req_wrap);
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_i2c_t, i2c);

  uint8_t cmd = req_data->cmd;
  uint8_t len = req_data->buf_len;
  char* data = req_data->buf_data;

  // The first element of data array is command.
  iotjs_buffer_reallocate(data, len + 1);
  memmove(data + 1, data, len * sizeof(char));
  data[0] = cmd;

  IOTJS_ASSERT(!_this->i2c_master);

  int ret =
      i2c_write(_this->i2c_master, &_this->config, &req_data->byte, len + 1);
  if (ret < 0) {
    DDLOG("I2C WriteBlockWorker : cannot write - %d", ret);
    req_data->error = kI2cErrWrite;
    return;
  }
  req_data->error = kI2cErrOk;
}


void ReadWorker(uv_work_t* work_req) {
  I2C_WORKER_INIT_TEMPLATE;
  iotjs_i2c_t* i2c = iotjs_i2c_instance_from_reqwrap(req_wrap);
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_i2c_t, i2c);

  uint8_t len = req_data->buf_len;
  req_data->buf_data = iotjs_buffer_allocate(len);

  IOTJS_ASSERT(!_this->i2c_master);
  IOTJS_ASSERT(len > 0);

  int ret = i2c_read(_this->i2c_master, &_this->config,
                     (uint8_t*)req_data->buf_data, len);
  if (ret != 0) {
    DDLOG("I2C ReadWorker : cannot read - %d", ret);
    req_data->error = kI2cErrRead;
    return;
  }
  req_data->error = kI2cErrOk;
}


void ReadByteWorker(uv_work_t* work_req) {
  I2C_WORKER_INIT_TEMPLATE;
  iotjs_i2c_t* i2c = iotjs_i2c_instance_from_reqwrap(req_wrap);
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_i2c_t, i2c);

  IOTJS_ASSERT(!_this->i2c_master);

  int ret = i2c_read(_this->i2c_master, &_this->config, &req_data->byte, 1);
  if (ret < 0) {
    DDLOG("I2C ReadByteWorker : cannot read - %d", ret);
    req_data->error = kI2cErrRead;
    return;
  }
  req_data->error = kI2cErrOk;
}


void ReadBlockWorker(uv_work_t* work_req) {
  I2C_WORKER_INIT_TEMPLATE;
  iotjs_i2c_t* i2c = iotjs_i2c_instance_from_reqwrap(req_wrap);
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_i2c_t, i2c);

  uint8_t cmd = req_data->cmd;
  uint8_t len = req_data->buf_len;
  req_data->buf_data = iotjs_buffer_allocate(len);

  IOTJS_ASSERT(!_this->i2c_master);
  IOTJS_ASSERT(len > 0);

  int ret = i2c_writeread(_this->i2c_master, &_this->config, &cmd, 1,
                          (uint8_t*)req_data->buf_data, len);

  if (ret < 0) {
    DDLOG("I2C ReadBlockWorker : cannot read - %d", ret);
    req_data->error = kI2cErrRead;
    return;
  }
  req_data->error = kI2cErrOk;
}


#endif // __NUTTX__
