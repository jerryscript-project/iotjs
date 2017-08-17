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


#ifndef IOTJS_MODULE_I2C_H
#define IOTJS_MODULE_I2C_H

#include "iotjs_def.h"
#include "iotjs_objectwrap.h"
#include "iotjs_reqwrap.h"

typedef enum {
  kI2cOpSetAddress,
  kI2cOpOpen,
  kI2cOpClose,
  kI2cOpWrite,
  kI2cOpRead,
} I2cOp;

typedef enum {
  kI2cErrOk = 0,
  kI2cErrOpen = -1,
  kI2cErrRead = -2,
  kI2cErrWrite = -3,
} I2cError;

typedef struct {
  char* buf_data;
  uint8_t buf_len;
  uint8_t byte;
  uint8_t cmd;
  int32_t delay;

  I2cOp op;
  I2cError error;
} iotjs_i2c_reqdata_t;

// Forward declaration of platform data. These are only used by platform code.
// Generic I2C module never dereferences platform data pointer.
typedef struct iotjs_i2c_platform_data_s iotjs_i2c_platform_data_t;
// This I2c class provides interfaces for I2C operation.
typedef struct {
  iotjs_jobjectwrap_t jobjectwrap;
  iotjs_i2c_platform_data_t* platform_data;
} IOTJS_VALIDATED_STRUCT(iotjs_i2c_t);

typedef struct {
  iotjs_reqwrap_t reqwrap;
  uv_work_t req;
  iotjs_i2c_reqdata_t req_data;
  iotjs_i2c_t* i2c_data;
} IOTJS_VALIDATED_STRUCT(iotjs_i2c_reqwrap_t);


iotjs_i2c_t* iotjs_i2c_instance_from_jval(const iotjs_jval_t* ji2c);
iotjs_i2c_reqwrap_t* iotjs_i2c_reqwrap_from_request(uv_work_t* req);
#define THIS iotjs_i2c_reqwrap_t* i2c_reqwrap
void iotjs_i2c_reqwrap_dispatched(THIS);
uv_work_t* iotjs_i2c_reqwrap_req(THIS);
const iotjs_jval_t* iotjs_i2c_reqwrap_jcallback(THIS);
iotjs_i2c_reqdata_t* iotjs_i2c_reqwrap_data(THIS);
iotjs_i2c_t* iotjs_i2c_instance_from_reqwrap(THIS);
#undef THIS

void I2cSetAddress(iotjs_i2c_t* i2c, uint8_t address);
void OpenWorker(uv_work_t* work_req);
void I2cClose(iotjs_i2c_t* i2c);
void WriteWorker(uv_work_t* work_req);
void ReadWorker(uv_work_t* work_req);

// Platform-related functions; they are implemented
// by platform code (i.e.: linux, nuttx, tizen).
void i2c_create_platform_data(iotjs_jhandler_t* jhandler, iotjs_i2c_t* i2c,
                              iotjs_i2c_platform_data_t** ppdata);
void i2c_destroy_platform_data(iotjs_i2c_platform_data_t* platform_data);

#endif /* IOTJS_MODULE_I2C_H */
