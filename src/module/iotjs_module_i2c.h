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


namespace iotjs {

enum I2cOp {
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
};

enum I2cError {
  kI2cErrOk = 0,
  kI2cErrOpen = -1,
  kI2cErrRead = -2,
  kI2cErrReadBlock = -3,
  kI2cErrWrite = -4,
};

struct I2cReqData {
  iotjs_string_t device;
  char* buf_data;
  uint8_t buf_len;
  uint8_t byte;
  uint8_t cmd;
  int32_t delay;

  I2cOp op;
  I2cError error;

  void* data; // pointer to I2cReqWrap
};

typedef ReqWrap<I2cReqData> I2cReqWrap;


// This I2c class provides interfaces for I2C operation.
class I2c : public JObjectWrap {
 public:
  explicit I2c(JObject& ji2c);
  virtual ~I2c();

  static I2c* Create(JObject& ji2c);
  static I2c* GetInstance();
  static JObject* GetJI2c();

  virtual int SetAddress(uint8_t address) = 0;
  virtual int Scan(I2cReqWrap* i2c_req) = 0;
  virtual int Open(I2cReqWrap* i2c_req) = 0;
  virtual int Close() = 0;
  virtual int Write(I2cReqWrap* i2c_req) = 0;
  virtual int WriteByte(I2cReqWrap* i2c_req) = 0;
  virtual int WriteBlock(I2cReqWrap* i2c_req) = 0;
  virtual int Read(I2cReqWrap* i2c_req) = 0;
  virtual int ReadByte(I2cReqWrap* i2c_req) = 0;
  virtual int ReadBlock(I2cReqWrap* i2c_req) = 0;
};


JObject* InitI2c();


} // namespace iotjs


#endif /* IOTJS_MODULE_I2C_H */
