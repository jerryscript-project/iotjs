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


void GetI2cArray(JObject* jarray, I2cReqData* req_data) {
  // FIXME: Need to implement a function to get array info from JObject Array.
  IOTJS_ASSERT(!(jarray->GetProperty("length").IsUndefined()));

  req_data->buf_len = jarray->GetProperty("length").GetNumber();
  req_data->buf_data = iotjs_buffer_allocate(req_data->buf_len);

  char index_str[3];
  for (int i = 0; i < req_data->buf_len; i++) {
    sprintf(index_str, "%d", i);
    req_data->buf_data[i] = jarray->GetProperty(index_str).GetNumber();
  }
}


JHANDLER_FUNCTION(SetAddress) {
  JHANDLER_CHECK(iotjs_jhandler_get_arg_length(jhandler) == 1);
  JHANDLER_CHECK(iotjs_jhandler_get_arg(jhandler, 0)->IsNumber());

  I2c* i2c = I2c::GetInstance();
  i2c->SetAddress(iotjs_jhandler_get_arg(jhandler, 0)->GetNumber());

  iotjs_jhandler_return_null(jhandler);
}


JHANDLER_FUNCTION(Scan) {
  JHANDLER_CHECK(iotjs_jhandler_get_arg_length(jhandler) == 1);
  JHANDLER_CHECK(iotjs_jhandler_get_arg(jhandler, 0)->IsFunction());

  I2cReqWrap* req_wrap = new I2cReqWrap(*iotjs_jhandler_get_arg(jhandler, 0));
  I2cReqData* req_data = req_wrap->req();

  req_data->op = kI2cOpScan;

  I2c* i2c = I2c::GetInstance();
  i2c->Scan(req_wrap);

  iotjs_jhandler_return_null(jhandler);
}


JHANDLER_FUNCTION(Open) {
  JHANDLER_CHECK(iotjs_jhandler_get_arg_length(jhandler) == 2);
  JHANDLER_CHECK(iotjs_jhandler_get_arg(jhandler, 0)->IsString());
  JHANDLER_CHECK(iotjs_jhandler_get_arg(jhandler, 1)->IsFunction());

  I2cReqWrap* req_wrap = new I2cReqWrap(*iotjs_jhandler_get_arg(jhandler, 1));
  I2cReqData* req_data = req_wrap->req();

  req_data->op = kI2cOpOpen;
  req_data->device = iotjs_jhandler_get_arg(jhandler, 0)->GetString();

  I2c* i2c = I2c::GetInstance();
  i2c->Open(req_wrap);

  iotjs_jhandler_return_null(jhandler);
}


JHANDLER_FUNCTION(Close) {
  JHANDLER_CHECK(iotjs_jhandler_get_arg_length(jhandler) == 0);

  I2c* i2c = I2c::GetInstance();
  i2c->Close();

  iotjs_jhandler_return_null(jhandler);
}


JHANDLER_FUNCTION(Write) {
  JHANDLER_CHECK(iotjs_jhandler_get_arg_length(jhandler) == 2);
  JHANDLER_CHECK(iotjs_jhandler_get_arg(jhandler, 0)->IsArray());
  JHANDLER_CHECK(iotjs_jhandler_get_arg(jhandler, 1)->IsFunction());

  I2cReqWrap* req_wrap = new I2cReqWrap(*iotjs_jhandler_get_arg(jhandler, 1));
  I2cReqData* req_data = req_wrap->req();

  req_data->op = kI2cOpWrite;
  GetI2cArray(iotjs_jhandler_get_arg(jhandler, 0), req_data);

  I2c* i2c = I2c::GetInstance();
  i2c->Write(req_wrap);

  iotjs_jhandler_return_null(jhandler);
}


JHANDLER_FUNCTION(WriteByte) {
  JHANDLER_CHECK(iotjs_jhandler_get_arg_length(jhandler) == 2);
  JHANDLER_CHECK(iotjs_jhandler_get_arg(jhandler, 0)->IsNumber());
  JHANDLER_CHECK(iotjs_jhandler_get_arg(jhandler, 1)->IsFunction());

  uint8_t byte = iotjs_jhandler_get_arg(jhandler, 0)->GetNumber();

  I2cReqWrap* req_wrap = new I2cReqWrap(*iotjs_jhandler_get_arg(jhandler, 1));
  I2cReqData* req_data = req_wrap->req();

  req_data->op = kI2cOpWriteByte;
  req_data->byte = byte;

  I2c* i2c = I2c::GetInstance();
  i2c->WriteByte(req_wrap);

  iotjs_jhandler_return_null(jhandler);
}


JHANDLER_FUNCTION(WriteBlock) {
  JHANDLER_CHECK(iotjs_jhandler_get_arg_length(jhandler) == 3);
  JHANDLER_CHECK(iotjs_jhandler_get_arg(jhandler, 0)->IsNumber());
  JHANDLER_CHECK(iotjs_jhandler_get_arg(jhandler, 1)->IsArray());
  JHANDLER_CHECK(iotjs_jhandler_get_arg(jhandler, 2)->IsFunction());

  I2cReqWrap* req_wrap = new I2cReqWrap(*iotjs_jhandler_get_arg(jhandler, 2));
  I2cReqData* req_data = req_wrap->req();

  req_data->op = kI2cOpWriteBlock;
  req_data->cmd = iotjs_jhandler_get_arg(jhandler, 0)->GetNumber();
  GetI2cArray(iotjs_jhandler_get_arg(jhandler, 1), req_data);

  I2c* i2c = I2c::GetInstance();
  i2c->WriteBlock(req_wrap);

  iotjs_jhandler_return_null(jhandler);
}


JHANDLER_FUNCTION(Read) {
  JHANDLER_CHECK(iotjs_jhandler_get_arg_length(jhandler) == 2);
  JHANDLER_CHECK(iotjs_jhandler_get_arg(jhandler, 0)->IsNumber());
  JHANDLER_CHECK(iotjs_jhandler_get_arg(jhandler, 1)->IsFunction());

  I2cReqWrap* req_wrap = new I2cReqWrap(*iotjs_jhandler_get_arg(jhandler, 1));
  I2cReqData* req_data = req_wrap->req();

  req_data->op = kI2cOpRead;
  req_data->buf_len = iotjs_jhandler_get_arg(jhandler, 0)->GetNumber();
  req_data->delay = 0;

  I2c* i2c = I2c::GetInstance();
  i2c->Read(req_wrap);

  iotjs_jhandler_return_null(jhandler);
}


JHANDLER_FUNCTION(ReadByte) {
  JHANDLER_CHECK(iotjs_jhandler_get_arg_length(jhandler) == 1);
  JHANDLER_CHECK(iotjs_jhandler_get_arg(jhandler, 0)->IsFunction());

  I2cReqWrap* req_wrap = new I2cReqWrap(*iotjs_jhandler_get_arg(jhandler, 0));
  I2cReqData* req_data = req_wrap->req();

  req_data->op = kI2cOpReadByte;

  I2c* i2c = I2c::GetInstance();
  i2c->ReadByte(req_wrap);

  iotjs_jhandler_return_null(jhandler);
}


JHANDLER_FUNCTION(ReadBlock) {
  JHANDLER_CHECK(iotjs_jhandler_get_arg_length(jhandler) == 4);
  JHANDLER_CHECK(iotjs_jhandler_get_arg(jhandler, 0)->IsNumber());
  JHANDLER_CHECK(iotjs_jhandler_get_arg(jhandler, 1)->IsNumber());
  JHANDLER_CHECK(iotjs_jhandler_get_arg(jhandler, 2)->IsNumber());
  JHANDLER_CHECK(iotjs_jhandler_get_arg(jhandler, 3)->IsFunction());

  I2cReqWrap* req_wrap = new I2cReqWrap(*iotjs_jhandler_get_arg(jhandler, 3));
  I2cReqData* req_data = req_wrap->req();

  req_data->op = kI2cOpReadBlock;
  req_data->cmd = iotjs_jhandler_get_arg(jhandler, 0)->GetNumber();
  req_data->buf_len = iotjs_jhandler_get_arg(jhandler, 1)->GetNumber();
  req_data->delay = iotjs_jhandler_get_arg(jhandler, 2)->GetNumber();

  I2c* i2c = I2c::GetInstance();
  i2c->ReadBlock(req_wrap);

  iotjs_jhandler_return_null(jhandler);
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
