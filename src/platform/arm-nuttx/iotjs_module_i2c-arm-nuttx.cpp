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

#if defined(__NUTTX__)


#include "module/iotjs_module_i2c.h"


namespace iotjs {


// I2C implementeation for arm-nuttx target.
class I2cArmNuttx : public I2c {
 public:
  explicit I2cArmNuttx(const iotjs_jval_t* ji2c);

  static I2cArmNuttx* GetInstance();

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


I2c* I2c::Create(const iotjs_jval_t* ji2c) {
  return new I2cArmNuttx(ji2c);
}


I2cArmNuttx::I2cArmNuttx(const iotjs_jval_t* ji2c)
    : I2c(ji2c) {
}


I2cArmNuttx* I2cArmNuttx::GetInstance()
{
  return static_cast<I2cArmNuttx*>(I2c::GetInstance());
}


int I2cArmNuttx::SetAddress(uint8_t address) {
  IOTJS_ASSERT(!"Not implemented");
  return 0;
}


int I2cArmNuttx::Scan(I2cReqWrap* i2c_req) {
  IOTJS_ASSERT(!"Not implemented");
  return 0;
}


int I2cArmNuttx::Open(I2cReqWrap* i2c_req) {
  IOTJS_ASSERT(!"Not implemented");
  return 0;
}


int I2cArmNuttx::Close() {
  IOTJS_ASSERT(!"Not implemented");
  return 0;
}


int I2cArmNuttx::Write(I2cReqWrap* i2c_req) {
  IOTJS_ASSERT(!"Not implemented");
  return 0;
}


int I2cArmNuttx::WriteByte(I2cReqWrap* i2c_req) {
  IOTJS_ASSERT(!"Not implemented");
  return 0;
}


int I2cArmNuttx::WriteBlock(I2cReqWrap* i2c_req) {
  IOTJS_ASSERT(!"Not implemented");
  return 0;
}


int I2cArmNuttx::Read(I2cReqWrap* i2c_req) {
  IOTJS_ASSERT(!"Not implemented");
  return 0;
}


int I2cArmNuttx::ReadByte(I2cReqWrap* i2c_req) {
  IOTJS_ASSERT(!"Not implemented");
  return 0;
}


int I2cArmNuttx::ReadBlock(I2cReqWrap* i2c_req) {
  IOTJS_ASSERT(!"Not implemented");
  return 0;
}


} // namespace iotjs

#endif // __NUTTX__
