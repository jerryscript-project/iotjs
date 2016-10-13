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


#include "iotjs_def.h"
#include "iotjs_objectwrap.h"
#include "iotjs_module_i2c.h"
#include "iotjs_module_buffer.h"


namespace iotjs {


I2c::I2c(JObject& ji2c)
    : JObjectWrap(ji2c) {
}


I2c::~I2c() {
}


JObject* I2c::GetJI2c() {
  Module* module = GetBuiltinModule(MODULE_I2C);
  JObject* ji2c = module->module;
  IOTJS_ASSERT(ji2c != NULL);

  return ji2c;
}


I2c* I2c::GetInstance() {
  I2c* i2c = reinterpret_cast<I2c*>(I2c::GetJI2c()->GetNative());
  IOTJS_ASSERT(i2c != NULL);

  return i2c;
}


JHANDLER_FUNCTION(SetAddress) {
  JHANDLER_CHECK(handler.GetArgLength() == 1);
  JHANDLER_CHECK(handler.GetArg(0)->IsNumber());

  I2c* i2c = I2c::GetInstance();
  i2c->SetAddress(handler.GetArg(0)->GetNumber());

  handler.Return(JVal::Null());
}


JHANDLER_FUNCTION(Scan) {
  JHANDLER_CHECK(handler.GetArgLength() == 1);
  JHANDLER_CHECK(handler.GetArg(0)->IsFunction());

  I2cReqWrap* req_wrap = new I2cReqWrap(*handler.GetArg(0));
  I2cReqData* req_data = req_wrap->req();

  req_data->op = kI2cOpScan;

  I2c* i2c = I2c::GetInstance();
  i2c->Scan(req_wrap);

  handler.Return(JVal::Null());
}


JHANDLER_FUNCTION(Open) {
  JHANDLER_CHECK(handler.GetArgLength() == 2);
  JHANDLER_CHECK(handler.GetArg(0)->IsString());
  JHANDLER_CHECK(handler.GetArg(1)->IsFunction());

  I2cReqWrap* req_wrap = new I2cReqWrap(*handler.GetArg(1));
  I2cReqData* req_data = req_wrap->req();

  req_data->op = kI2cOpOpen;

  String device = handler.GetArg(0)->GetString();
  req_data->device.Append(device.data(), device.size());

  I2c* i2c = I2c::GetInstance();
  i2c->Open(req_wrap);

  handler.Return(JVal::Null());
}


JHANDLER_FUNCTION(Close) {
  JHANDLER_CHECK(handler.GetArgLength() == 0);

  I2c* i2c = I2c::GetInstance();
  i2c->Close();

  handler.Return(JVal::Null());
}


JHANDLER_FUNCTION(Write) {
  JHANDLER_CHECK(handler.GetArgLength() == 2);
  JHANDLER_CHECK(handler.GetArg(0)->IsObject());
  JHANDLER_CHECK(handler.GetArg(1)->IsFunction());

  JObject* jbuffer = handler.GetArg(0);
  BufferWrap* buffer = BufferWrap::FromJBuffer(*jbuffer);
  char* buf_data = buffer->buffer();
  int buf_len = buffer->length();
  JHANDLER_CHECK(buf_data != NULL);
  JHANDLER_CHECK(buf_len >= 0);

  I2cReqWrap* req_wrap = new I2cReqWrap(*handler.GetArg(1));
  I2cReqData* req_data = req_wrap->req();

  req_data->op = kI2cOpWrite;

  req_data->buf_data = buf_data;
  req_data->buf_len = buf_len;

  I2c* i2c = I2c::GetInstance();
  i2c->Write(req_wrap);

  handler.Return(JVal::Null());
}


JHANDLER_FUNCTION(WriteByte) {
  JHANDLER_CHECK(handler.GetArgLength() == 2);
  JHANDLER_CHECK(handler.GetArg(0)->IsNumber());
  JHANDLER_CHECK(handler.GetArg(1)->IsFunction());

  int8_t byte = handler.GetArg(0)->GetNumber();

  I2cReqWrap* req_wrap = new I2cReqWrap(*handler.GetArg(1));
  I2cReqData* req_data = req_wrap->req();

  req_data->op = kI2cOpWriteByte;

  req_data->byte = byte;

  I2c* i2c = I2c::GetInstance();
  i2c->WriteByte(req_wrap);

  handler.Return(JVal::Null());
}


JHANDLER_FUNCTION(WriteBlock) {
  JHANDLER_CHECK(handler.GetArgLength() == 3);
  JHANDLER_CHECK(handler.GetArg(0)->IsNumber());
  JHANDLER_CHECK(handler.GetArg(1)->IsObject());
  JHANDLER_CHECK(handler.GetArg(2)->IsFunction());

  int8_t cmd = handler.GetArg(0)->GetNumber();

  JObject* jbuffer = handler.GetArg(1);
  BufferWrap* buffer = BufferWrap::FromJBuffer(*jbuffer);
  char* buf_data = buffer->buffer();
  int buf_len = buffer->length();
  JHANDLER_CHECK(buf_data != NULL);
  JHANDLER_CHECK(buf_len >= 0);

  I2cReqWrap* req_wrap = new I2cReqWrap(*handler.GetArg(2));
  I2cReqData* req_data = req_wrap->req();

  req_data->op = kI2cOpWriteBlock;

  req_data->cmd = cmd;
  req_data->buf_data = buf_data;
  req_data->buf_len = buf_len;

  I2c* i2c = I2c::GetInstance();
  i2c->WriteBlock(req_wrap);

  handler.Return(JVal::Null());
}


JHANDLER_FUNCTION(Read) {
  JHANDLER_CHECK(handler.GetArgLength() == 2);
  JHANDLER_CHECK(handler.GetArg(0)->IsNumber());
  JHANDLER_CHECK(handler.GetArg(1)->IsFunction());

  int8_t buf_len = handler.GetArg(0)->GetNumber();

  I2cReqWrap* req_wrap = new I2cReqWrap(*handler.GetArg(1));
  I2cReqData* req_data = req_wrap->req();

  req_data->op = kI2cOpRead;

  req_data->buf_len = buf_len;

  I2c* i2c = I2c::GetInstance();
  i2c->Read(req_wrap);

  handler.Return(JVal::Null());
}


JHANDLER_FUNCTION(ReadByte) {
  JHANDLER_CHECK(handler.GetArgLength() == 1);
  JHANDLER_CHECK(handler.GetArg(0)->IsFunction());

  I2cReqWrap* req_wrap = new I2cReqWrap(*handler.GetArg(0));
  I2cReqData* req_data = req_wrap->req();

  req_data->op = kI2cOpReadByte;

  I2c* i2c = I2c::GetInstance();
  i2c->ReadByte(req_wrap);

  handler.Return(JVal::Null());
}


JHANDLER_FUNCTION(ReadBlock) {
  JHANDLER_CHECK(handler.GetArgLength() == 4);
  JHANDLER_CHECK(handler.GetArg(0)->IsNumber());
  JHANDLER_CHECK(handler.GetArg(1)->IsNumber());
  JHANDLER_CHECK(handler.GetArg(2)->IsNumber());
  JHANDLER_CHECK(handler.GetArg(3)->IsFunction());

  uint8_t cmd = handler.GetArg(0)->GetNumber();
  uint8_t buf_len = handler.GetArg(1)->GetNumber();
  uint8_t delay = handler.GetArg(2)->GetNumber();

  I2cReqWrap* req_wrap = new I2cReqWrap(*handler.GetArg(3));
  I2cReqData* req_data = req_wrap->req();

  req_data->op = kI2cOpReadBlock;

  req_data->cmd = cmd;
  req_data->buf_len = buf_len;
  req_data->delay = delay;

  I2c* i2c = I2c::GetInstance();
  i2c->ReadBlock(req_wrap);

  handler.Return(JVal::Null());
}


JObject* InitI2c() {
  Module* module = GetBuiltinModule(MODULE_I2C);
  JObject* ji2c = module->module;

  if (ji2c == NULL) {
    ji2c = new JObject();

    ji2c->SetMethod("setAddress", SetAddress);
    ji2c->SetMethod("scan", Scan);
    ji2c->SetMethod("open", Open);
    ji2c->SetMethod("close", Close);
    ji2c->SetMethod("write", Write);
    ji2c->SetMethod("writeByte", WriteByte);
    ji2c->SetMethod("writeBlock", WriteBlock);
    ji2c->SetMethod("read", Read);
    ji2c->SetMethod("readByte", ReadByte);
    ji2c->SetMethod("readBlock", ReadBlock);

    I2c* i2c = I2c::Create(*ji2c);
    IOTJS_ASSERT(i2c == reinterpret_cast<I2c*>(ji2c->GetNative()));

    module->module = ji2c;
  }

  return ji2c;
}


} // namespace iotjs
