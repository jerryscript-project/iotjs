/* Copyright 2016 Samsung Electronics Co., Ltd.
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
  iotjs_string_t device;
  char* buf_data;
  uint8_t buf_len;
  uint8_t byte;
  uint8_t cmd;
  int32_t delay;

  I2cOp op;
  I2cError error;
} iotjs_i2creqdata_t;


typedef struct {
  iotjs_reqwrap_t reqwrap;
  uv_work_t req;
  iotjs_i2creqdata_t req_data;
} IOTJS_VALIDATED_STRUCT(iotjs_i2creqwrap_t);


#define THIS iotjs_i2creqwrap_t* i2creqwrap
void iotjs_i2creqwrap_initialize(THIS, const iotjs_jval_t* jcallback, I2cOp op);
void iotjs_i2creqwrap_destroy(THIS);
uv_work_t* iotjs_i2creqwrap_req(THIS);
const iotjs_jval_t* iotjs_i2creqwrap_jcallback(THIS);
iotjs_i2creqwrap_t* iotjs_i2creqwrap_from_request(uv_work_t* req);
iotjs_i2creqdata_t* iotjs_i2creqwrap_data(THIS);
#undef THIS


// This I2c class provides interfaces for I2C operation.
typedef struct {
  iotjs_jobjectwrap_t jobjectwrap;
} IOTJS_VALIDATED_STRUCT(iotjs_i2c_t);

iotjs_i2c_t* iotjs_i2c_create(const iotjs_jval_t* ji2c);
void iotjs_i2c_destroy(iotjs_i2c_t* i2c);
const iotjs_jval_t* iotjs_i2c_get_ji2c();
iotjs_i2c_t* iotjs_i2c_get_instance();


void I2cSetAddress(uint8_t address);
void ScanWorker(uv_work_t* work_req);
void OpenWorker(uv_work_t* work_req);
void I2cClose();
void WriteWorker(uv_work_t* work_req);
void WriteByteWorker(uv_work_t* work_req);
void WriteBlockWorker(uv_work_t* work_req);
void ReadWorker(uv_work_t* work_req);
void ReadByteWorker(uv_work_t* work_req);
void ReadBlockWorker(uv_work_t* work_req);


#endif /* IOTJS_MODULE_I2C_H */
