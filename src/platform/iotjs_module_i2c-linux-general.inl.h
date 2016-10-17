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
#include <linux/i2c-dev.h>


#include "module/iotjs_module_i2c.h"


namespace iotjs {


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


JObject makeI2cArray(char* data, int len) {
  // FIXME: Need to use JS array. (Not implemented)
  JObject jarray;
  JObject jlen(len);
  jarray.SetProperty("length", jlen);

  char index_str[3];

  for (int i = 0; i < len; i++) {
    sprintf(index_str, "%d", i);
    JObject v(data[i]);
    jarray.SetProperty(index_str, v);
  }

  return jarray;
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

        JObject result = makeI2cArray(req_data->buf_data, req_data->buf_len);
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

  int8_t cmd = req_data->cmd;
  int32_t len = req_data->buf_len;
  uint8_t data[len];

  if (i2c_smbus_read_i2c_block_data(fd, cmd, len, data) != len) {
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
