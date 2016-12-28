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

#if defined(__NUTTX__)
#include <nuttx/i2c/i2c_master.h>
#endif

typedef enum {
  kI2cOpSetAddress,
  kI2cOpScan,
  kI2cOpOpen,
  kI2cOpClose,
  kI2cOpWrite,
  kI2cOpWriteByte,
  kI2cOpWriteBlock,
  kI2cOpRead,
  kI2cOpReadByte,
  kI2cOpReadBlock,
} I2cOp;

typedef enum {
  kI2cErrOk = 0,
  kI2cErrOpen = -1,
  kI2cErrRead = -2,
  kI2cErrReadBlock = -3,
  kI2cErrWrite = -4,
} I2cError;


typedef struct {
#if defined(__LINUX__)
  iotjs_string_t device;
#elif defined(__NUTTX__)
  uint32_t device;
#endif
  char* buf_data;
  uint8_t buf_len;
  uint8_t byte;
  uint8_t cmd;
  int32_t delay;

  I2cOp op;
  I2cError error;
} iotjs_i2c_reqdata_t;


// This I2c class provides interfaces for I2C operation.
typedef struct {
#if defined(__NUTTX__)
  struct i2c_master_s* i2c_master;
  struct i2c_config_s config;
#endif
  iotjs_jobjectwrap_t jobjectwrap;
} IOTJS_VALIDATED_STRUCT(iotjs_i2c_t);


typedef struct {
  iotjs_reqwrap_t reqwrap;
  uv_work_t req;
  iotjs_i2c_reqdata_t req_data;
  iotjs_i2c_t* i2c_data;
} IOTJS_VALIDATED_STRUCT(iotjs_i2c_reqwrap_t);


iotjs_i2c_t* iotjs_i2c_create(const iotjs_jval_t* ji2c);
iotjs_i2c_t* iotjs_i2c_instance_from_jval(const iotjs_jval_t* ji2c);

#define THIS iotjs_i2c_reqwrap_t* i2c_reqwrap
iotjs_i2c_reqwrap_t* iotjs_i2c_reqwrap_create(const iotjs_jval_t* jcallback,
                                              const iotjs_jval_t* ji2c,
                                              I2cOp op);
void iotjs_i2c_reqwrap_dispatched(THIS);
uv_work_t* iotjs_i2c_reqwrap_req(THIS);
const iotjs_jval_t* iotjs_i2c_reqwrap_jcallback(THIS);
iotjs_i2c_reqwrap_t* iotjs_i2c_reqwrap_from_request(uv_work_t* req);
iotjs_i2c_reqdata_t* iotjs_i2c_reqwrap_data(THIS);
iotjs_i2c_t* iotjs_i2c_instance_from_reqwrap(THIS);
#undef THIS


void I2cSetAddress(iotjs_i2c_t* i2c, uint8_t address);
void ScanWorker(uv_work_t* work_req);
void OpenWorker(uv_work_t* work_req);
void I2cClose(iotjs_i2c_t* i2c);
void WriteWorker(uv_work_t* work_req);
void WriteByteWorker(uv_work_t* work_req);
void WriteBlockWorker(uv_work_t* work_req);
void ReadWorker(uv_work_t* work_req);
void ReadByteWorker(uv_work_t* work_req);
void ReadBlockWorker(uv_work_t* work_req);


#endif /* IOTJS_MODULE_I2C_H */
