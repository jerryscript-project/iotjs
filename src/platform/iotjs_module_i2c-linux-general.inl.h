/* The MIT License (MIT)
 *
 * Copyright (c) 2005-2014 RoadNarrows LLC.
 * htpt://roadnarrows.com
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


/* Some functions are modified from the RoadNarrows-robotics i2c library.
 * (distributed under the MIT license.)
 */


#ifndef IOTJS_MODULE_I2C_LINUX_GENERAL_INL_H
#define IOTJS_MODULE_I2C_LINUX_GENERAL_INL_H


#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>


#include "module/iotjs_module_i2c.h"


#define I2C_SLAVE_FORCE 0x0706
#define I2C_SMBUS 0x0720
#define I2C_SMBUS_BLOCK_MAX 32
#define I2C_SMBUS_READ 1
#define I2C_SMBUS_WRITE 0
#define I2C_SMBUS_I2C_BLOCK_DATA 8


namespace iotjs {


typedef union I2cSmbusDataUnion {
  uint8_t byte;
  unsigned short word;
  uint8_t block[I2C_SMBUS_BLOCK_MAX + 2];
} I2cSmbusData;


typedef struct I2cSmbusIoctlDataStruct
{
  uint8_t read_write;
  uint8_t command;
  int size;
  I2cSmbusData* data;
} I2cSmbusIoctlData;


int fd;
int8_t addr;


class I2cLinuxGeneral : public I2c {
 public:
  explicit I2cLinuxGeneral(JObject& ji2c);

  static I2cLinuxGeneral* GetInstance();

  virtual int SetAddress(int8_t address);
  virtual int Scan(I2cReqWrap* i2c_req);
  virtual int Open(I2cReqWrap* i2c_req);
  virtual int Close();
  virtual int Write(I2cReqWrap* i2c_rea);
  virtual int WriteByte(I2cReqWrap* i2c_rea);
  virtual int WriteBlock(I2cReqWrap* i2c_rea);
  virtual int Read(I2cReqWrap* i2c_rea);
  virtual int ReadByte(I2cReqWrap* i2c_rea);
  virtual int ReadBlock(I2cReqWrap* i2c_rea);
};


I2cLinuxGeneral::I2cLinuxGeneral(JObject& ji2c)
    : I2c(ji2c) {
}


I2cLinuxGeneral* I2cLinuxGeneral::GetInstance() {
  return static_cast<I2cLinuxGeneral*>(I2c::GetInstance());
}

int I2cSmbusAccess(int fd, uint8_t read_write, uint8_t command, int size,
                   I2cSmbusData* data) {
  I2cSmbusIoctlData args;

  args.read_write = read_write;
  args.command = command;
  args.size = size;
  args.data = data;

  return ioctl(fd, I2C_SMBUS, &args);
}


int I2cSmbusReadI2cBlockData(int fd, uint8_t command, uint8_t* values,
                             uint8_t length) {
  I2cSmbusData data;
  int i, result;

  data.block[0] = length;
  result = I2cSmbusAccess(fd, I2C_SMBUS_READ, command, I2C_SMBUS_I2C_BLOCK_DATA,
                          &data);
  if (result >= 0) {
    for(i = 1; i <= data.block[0]; i++) {
      values[i - 1] = data.block[i];
    }
    result = data.block[0];
  }

  return result;
}


void AfterI2cWork(uv_work_t* work_req, int status) {
  I2cLinuxGeneral* i2c = I2cLinuxGeneral::GetInstance();

  I2cReqWrap* i2c_req = reinterpret_cast<I2cReqWrap*>(work_req->data);
  I2cReqData* req_data = i2c_req->req();

  JArgList jargs(2);

  switch (req_data->op) {
    case kI2cOpOpen:
    {
      if (req_data->error == kI2cErrOpen) {
        JObject error = JObject::Error("Failed to open I2C device.");
        jargs.Add(error);
      } else {
        jargs.Add(JVal::Null());
      }
      break;
    }
    case kI2cOpReadBlock:
    {
      if (req_data->error == kI2cErrReadBlock) {
        JObject error = JObject::Error("Error reading length of bytes.");
        jargs.Add(error);
      } else {
        jargs.Add(JVal::Null());

        JObject result = JObject(req_data->buf_len, req_data->buf_data);
        jargs.Add(result);

        ReleaseBuffer(req_data->buf_data);
      }
      break;
    }
  }

  MakeCallback(i2c_req->jcallback(), *I2c::GetJI2c(), jargs);

  delete work_req;
  delete i2c_req;
}


void OpenWorker(uv_work_t* work_req) {
  I2cLinuxGeneral* i2c = I2cLinuxGeneral::GetInstance();

  I2cReqWrap* i2c_req = reinterpret_cast<I2cReqWrap*>(work_req->data);
  I2cReqData* req_data = i2c_req->req();

  fd = open((req_data->device).data(), O_RDWR);

  if (fd == -1) {
    req_data->error = kI2cErrOpen;
  } else {
    req_data->error = kI2cErrOk;
  }
}


void ReadBlockWorker(uv_work_t* work_req) {
  I2cReqWrap* i2c_req = reinterpret_cast<I2cReqWrap*>(work_req->data);
  I2cReqData* req_data = i2c_req->req();

  uint8_t cmd = req_data->cmd;
  uint8_t len = req_data->buf_len;
  uint8_t data[I2C_SMBUS_BLOCK_MAX + 2];

  if (I2cSmbusReadI2cBlockData(fd, cmd, data, len) != len) {
    req_data->error = kI2cErrReadBlock;
  }

  req_data->buf_data = AllocBuffer(len);
  memcpy(req_data->buf_data, data, len);

  if (req_data->delay != 0) {
  }
}


#define I2C_LINUX_GENERAL_IMPL_TEMPLATE(op) \
  do { \
    I2cLinuxGeneral* i2c = I2cLinuxGeneral::GetInstance(); \
    Environment* env = Environment::GetEnv(); \
    uv_work_t* req = new uv_work_t; \
    req->data = reinterpret_cast<void*>(i2c_req); \
    uv_queue_work(env->loop(), req, op ## Worker, AfterI2cWork); \
  } while (0)


int I2cLinuxGeneral::SetAddress(int8_t address) {
  addr = address;
  ioctl(fd, I2C_SLAVE_FORCE, addr);
  return 0;
}


int I2cLinuxGeneral::Scan(I2cReqWrap* i2c_req) {
  IOTJS_ASSERT(!"Not implemented");
  return 0;
}


int I2cLinuxGeneral::Open(I2cReqWrap* i2c_req) {
  I2C_LINUX_GENERAL_IMPL_TEMPLATE(Open);
  return 0;
}


int I2cLinuxGeneral::Close() {
  if (fd > 0) {
    close(fd);
  }
  return 0;
}


int I2cLinuxGeneral::Write(I2cReqWrap* i2c_req) {
  IOTJS_ASSERT(!"Not implemented");
  return 0;
}


int I2cLinuxGeneral::WriteByte(I2cReqWrap* i2c_req) {
  IOTJS_ASSERT(!"Not implemented");
  return 0;
}


int I2cLinuxGeneral::WriteBlock(I2cReqWrap* i2c_req) {
  IOTJS_ASSERT(!"Not implemented");
  return 0;
}


int I2cLinuxGeneral::Read(I2cReqWrap* i2c_req) {
  IOTJS_ASSERT(!"Not implemented");
  return 0;
}


int I2cLinuxGeneral::ReadByte(I2cReqWrap* i2c_req) {
  IOTJS_ASSERT(!"Not implemented");
  return 0;
}


int I2cLinuxGeneral::ReadBlock(I2cReqWrap* i2c_req) {
  I2C_LINUX_GENERAL_IMPL_TEMPLATE(ReadBlock);
  return 0;
}


} // namespace iotjs


#endif /* IOTJS_MODULE_I2C_LINUX_GENERAL_INL_H */
