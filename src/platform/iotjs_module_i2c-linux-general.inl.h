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
#define I2C_NOCMD 0
#define I2C_SMBUS_BYTE 1
#define I2C_SMBUS_BLOCK_DATA 5
#define I2C_SMBUS_I2C_BLOCK_DATA 8
#define I2C_SMBUS_BLOCK_MAX 32
#define I2C_MAX_ADDRESS 128


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
uint8_t addr;


class I2cLinuxGeneral : public I2c {
 public:
  explicit I2cLinuxGeneral(JObject& ji2c);

  static I2cLinuxGeneral* GetInstance();

  virtual int SetAddress(uint8_t address);
  virtual int Scan(I2cReqWrap* i2c_req);
  virtual int Open(I2cReqWrap* i2c_req);
  virtual int Close();
  virtual int Write(I2cReqWrap* i2c_req);
  virtual int WriteByte(I2cReqWrap* i2c_req);
  virtual int WriteBlock(I2cReqWrap* i2c_req);
  virtual int Read(I2cReqWrap* i2c_req);
  virtual int ReadByte(I2cReqWrap* i2c_req);
  virtual int ReadBlock(I2cReqWrap* i2c_req);
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


int I2cSmbusWriteByte(int fd, uint8_t byte) {
  return I2cSmbusAccess(fd, I2C_SMBUS_WRITE, byte, I2C_SMBUS_BYTE, NULL);
}


int I2cSmbusWriteI2cBlockData(int fd, uint8_t command, uint8_t* values,
                              uint8_t length) {
  I2cSmbusData data;

  if (length > I2C_SMBUS_BLOCK_MAX) {
    length = I2C_SMBUS_BLOCK_MAX;
  }

  for (int i = 1; i <= length; i++) {
    data.block[i] = values[i - 1];
  }
  data.block[0] = length;

  return I2cSmbusAccess(fd, I2C_SMBUS_WRITE, command, I2C_SMBUS_BLOCK_DATA,
                        &data);
}


int I2cSmbusReadByte(int fd) {
  I2cSmbusData data;

  int result = I2cSmbusAccess(fd, I2C_SMBUS_READ, I2C_NOCMD, I2C_SMBUS_BYTE,
                              &data);

  // Mask one byte from result (data.byte).
  return result >= 0 ? 0xFF & data.byte : -1;
}


int I2cSmbusReadI2cBlockData(int fd, uint8_t command, uint8_t* values,
                             uint8_t length) {
  I2cSmbusData data;

  if (length > I2C_SMBUS_BLOCK_MAX) {
    length = I2C_SMBUS_BLOCK_MAX;
  }
  data.block[0] = length;

  int result = I2cSmbusAccess(fd, I2C_SMBUS_READ, command,
                              I2C_SMBUS_I2C_BLOCK_DATA, &data);
  if (result >= 0) {
    for(int i = 1; i <= data.block[0]; i++) {
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

  iotjs_jargs_t jargs = iotjs_jargs_create(2);

  if (status) {
    JObject error = JObject::Error("System error");
    iotjs_jargs_append_obj(&jargs, &error);
  } else {
    switch (req_data->op) {
      case kI2cOpOpen:
      {
        if (req_data->error == kI2cErrOpen) {
          JObject error = JObject::Error("Failed to open I2C device");
          iotjs_jargs_append_obj(&jargs, &error);
        } else {
          iotjs_jargs_append_null(&jargs);
        }
        break;
      }
      case kI2cOpScan:
      {
        iotjs_jargs_append_null(&jargs);
        JObject result = JObject(req_data->buf_len, req_data->buf_data);
        iotjs_jargs_append_obj(&jargs, &result);

        if (req_data->buf_data != NULL) {
          iotjs_buffer_release(req_data->buf_data);
        }
        break;
      }
      case kI2cOpWrite:
      case kI2cOpWriteByte:
      case kI2cOpWriteBlock:
      {
        if (req_data->error == kI2cErrWrite) {
          JObject error = JObject::Error("Cannot write to device");
          iotjs_jargs_append_obj(&jargs, &error);
        } else {
          iotjs_jargs_append_null(&jargs);
        }
        break;
      }
      case kI2cOpRead:
      case kI2cOpReadBlock:
      {
        if (req_data->error == kI2cErrRead) {
          JObject error = JObject::Error("Cannot read from device");
          iotjs_jargs_append_obj(&jargs, &error);
          iotjs_jargs_append_null(&jargs);
        } else if (req_data->error == kI2cErrReadBlock) {
          JObject error = JObject::Error("Error reading length of bytes");
          iotjs_jargs_append_obj(&jargs, &error);
          iotjs_jargs_append_null(&jargs);
        } else {
          iotjs_jargs_append_null(&jargs);
          JObject result = JObject(req_data->buf_len, req_data->buf_data);
          iotjs_jargs_append_obj(&jargs, &result);

          if (req_data->delay > 0) {
            usleep(req_data->delay * 1000);
          }

          if (req_data->buf_data != NULL) {
            iotjs_buffer_release(req_data->buf_data);
          }
        }
        break;
      }
      case kI2cOpReadByte:
      {
        if (req_data->error == kI2cErrRead) {
          JObject error = JObject::Error("Cannot read from device");
          iotjs_jargs_append_obj(&jargs, &error);
          iotjs_jargs_append_null(&jargs);
        } else {
          iotjs_jargs_append_null(&jargs);
          iotjs_jargs_append_number(&jargs, req_data->byte);
        }
        break;
      }
      default:
      {
        IOTJS_ASSERT(!"Unreachable");
        break;
      }
    }
  }

  MakeCallback(i2c_req->jcallback(), *I2c::GetJI2c(), jargs);

  delete work_req;
  delete i2c_req;
  iotjs_jargs_destroy(&jargs);
}


void OpenWorker(uv_work_t* work_req) {
  I2cReqWrap* i2c_req = reinterpret_cast<I2cReqWrap*>(work_req->data);
  I2cReqData* req_data = i2c_req->req();

  fd = open(iotjs_string_data(&req_data->device), O_RDWR);

  if (fd == -1) {
    req_data->error = kI2cErrOpen;
  } else {
    req_data->error = kI2cErrOk;
  }
}


void ScanWorker(uv_work_t* work_req) {
  I2cReqWrap* i2c_req = reinterpret_cast<I2cReqWrap*>(work_req->data);
  I2cReqData* req_data = i2c_req->req();

  int result;
  req_data->buf_data = iotjs_buffer_allocate(I2C_MAX_ADDRESS);
  req_data->buf_len = I2C_MAX_ADDRESS;

  for (int i = 0; i < I2C_MAX_ADDRESS; i++) {
    ioctl(fd, I2C_SLAVE_FORCE, i);

    /* Test address responsiveness
       The default probe method is a quick write, but it is known
       to corrupt the 24RF08 EEPROMs due to a state machine bug,
       and could also irreversibly write-protect some EEPROMs, so
       for address ranges 0x30-0x37 and 0x50-0x5F, we use a byte
       read instead. Also, some bus drivers don't implement
       quick write, so we fallback to a byte read it that case
       too. */
    if ((i >= 0x30 && i <= 0x37) || (i >= 0x50 && i <= 0x5F)) {
      result = I2cSmbusReadByte(fd);
    } else {
      result = I2cSmbusAccess(fd, I2C_SMBUS_WRITE, I2C_NOCMD, 0, NULL);
    }

    if (result >= 0) {
      result = 1;
    }

    req_data->buf_data[i] = result;
  }

  ioctl(fd, I2C_SLAVE_FORCE, addr);
}


void WriteWorker(uv_work_t* work_req) {
  I2cReqWrap* i2c_req = reinterpret_cast<I2cReqWrap*>(work_req->data);
  I2cReqData* req_data = i2c_req->req();

  uint8_t len = req_data->buf_len;
  char* data = req_data->buf_data;

  if (write(fd, data, len) != len) {
    req_data->error = kI2cErrWrite;
  }

  if (req_data->buf_data != NULL) {
    iotjs_buffer_release(req_data->buf_data);
  }
}


void WriteByteWorker(uv_work_t* work_req) {
  I2cReqWrap* i2c_req = reinterpret_cast<I2cReqWrap*>(work_req->data);
  I2cReqData* req_data = i2c_req->req();

  if (I2cSmbusWriteByte(fd, req_data->byte) == -1) {
    req_data->error = kI2cErrWrite;
  }
}


void WriteBlockWorker(uv_work_t* work_req) {
  I2cReqWrap* i2c_req = reinterpret_cast<I2cReqWrap*>(work_req->data);
  I2cReqData* req_data = i2c_req->req();

  uint8_t cmd = req_data->cmd;
  uint8_t len = req_data->buf_len;
  uint8_t* data = reinterpret_cast<uint8_t*>(req_data->buf_data);

  if (I2cSmbusWriteI2cBlockData(fd, cmd, data, len) == -1) {
    req_data->error = kI2cErrWrite;
  }

  if (req_data->buf_data != NULL) {
    iotjs_buffer_release(req_data->buf_data);
  }
}


void ReadWorker(uv_work_t* work_req) {
  I2cReqWrap* i2c_req = reinterpret_cast<I2cReqWrap*>(work_req->data);
  I2cReqData* req_data = i2c_req->req();

  uint8_t len = req_data->buf_len;
  req_data->buf_data = iotjs_buffer_allocate(len);

  if (read(fd, req_data->buf_data, len) != len) {
    req_data->error = kI2cErrRead;
  }
}


void ReadByteWorker(uv_work_t* work_req) {
  I2cReqWrap* i2c_req = reinterpret_cast<I2cReqWrap*>(work_req->data);
  I2cReqData* req_data = i2c_req->req();

  int result = I2cSmbusReadByte(fd);
  if (result == -1) {
    req_data->error = kI2cErrRead;
  } else {
    req_data->byte = result;
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

  req_data->buf_data = iotjs_buffer_allocate(len);
  memcpy(req_data->buf_data, data, len);
}


#define I2C_LINUX_GENERAL_IMPL_TEMPLATE(op) \
  do { \
    I2cLinuxGeneral* i2c = I2cLinuxGeneral::GetInstance(); \
    Environment* env = Environment::GetEnv(); \
    uv_work_t* req = new uv_work_t; \
    req->data = reinterpret_cast<void*>(i2c_req); \
    uv_queue_work(env->loop(), req, op ## Worker, AfterI2cWork); \
  } while (0)


int I2cLinuxGeneral::SetAddress(uint8_t address) {
  addr = address;
  ioctl(fd, I2C_SLAVE_FORCE, addr);
  return 0;
}


int I2cLinuxGeneral::Scan(I2cReqWrap* i2c_req) {
  I2C_LINUX_GENERAL_IMPL_TEMPLATE(Scan);
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
  I2C_LINUX_GENERAL_IMPL_TEMPLATE(Write);
  return 0;
}


int I2cLinuxGeneral::WriteByte(I2cReqWrap* i2c_req) {
  I2C_LINUX_GENERAL_IMPL_TEMPLATE(WriteByte);
  return 0;
}


int I2cLinuxGeneral::WriteBlock(I2cReqWrap* i2c_req) {
  I2C_LINUX_GENERAL_IMPL_TEMPLATE(WriteBlock);
  return 0;
}


int I2cLinuxGeneral::Read(I2cReqWrap* i2c_req) {
  I2C_LINUX_GENERAL_IMPL_TEMPLATE(Read);
  return 0;
}


int I2cLinuxGeneral::ReadByte(I2cReqWrap* i2c_req) {
  I2C_LINUX_GENERAL_IMPL_TEMPLATE(ReadByte);
  return 0;
}


int I2cLinuxGeneral::ReadBlock(I2cReqWrap* i2c_req) {
  I2C_LINUX_GENERAL_IMPL_TEMPLATE(ReadBlock);
  return 0;
}


} // namespace iotjs


#endif /* IOTJS_MODULE_I2C_LINUX_GENERAL_INL_H */
