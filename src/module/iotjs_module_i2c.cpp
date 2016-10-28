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


I2c::I2c(const iotjs_jval_t* ji2c)
    : JObjectWrap(ji2c) {
}


I2c::~I2c() {
}


const iotjs_jval_t* I2c::GetJI2c() {
  Module* module = GetBuiltinModule(MODULE_I2C);
  const iotjs_jval_t* ji2c = &module->module;
  IOTJS_ASSERT(ji2c != NULL);

  return ji2c;
}


I2c* I2c::GetInstance() {
  I2c* i2c = reinterpret_cast<I2c*>(
          iotjs_jval_get_object_native_handle(I2c::GetJI2c()));
  IOTJS_ASSERT(i2c != NULL);

  return i2c;
}


static void GetI2cArray(const iotjs_jval_t* jarray, I2cReqData* req_data) {
  // FIXME
  // Need to implement a function to get array info from iotjs_jval_t Array.
  iotjs_jval_t jlength = iotjs_jval_get_property(jarray, "length");
  IOTJS_ASSERT(!iotjs_jval_is_undefined(&jlength));

  req_data->buf_len = iotjs_jval_as_number(&jlength);
  req_data->buf_data = iotjs_buffer_allocate(req_data->buf_len);

  for (int i = 0; i < req_data->buf_len; i++) {
    iotjs_jval_t jdata = iotjs_jval_get_property_by_index(jarray, i);
    req_data->buf_data[i] = iotjs_jval_as_number(&jdata);
    iotjs_jval_destroy(&jdata);
  }

  iotjs_jval_destroy(&jlength);
}


JHANDLER_FUNCTION(SetAddress) {
  JHANDLER_CHECK_ARGS(1, number);

  I2c* i2c = I2c::GetInstance();
  i2c->SetAddress(JHANDLER_GET_ARG(0, number));

  iotjs_jhandler_return_null(jhandler);
}


JHANDLER_FUNCTION(Scan) {
  JHANDLER_CHECK_ARGS(1, function);

  I2cReqWrap* req_wrap = new I2cReqWrap(JHANDLER_GET_ARG(0, function));
  I2cReqData* req_data = req_wrap->req();

  req_data->op = kI2cOpScan;

  I2c* i2c = I2c::GetInstance();
  i2c->Scan(req_wrap);

  iotjs_jhandler_return_null(jhandler);
}


JHANDLER_FUNCTION(Open) {
  JHANDLER_CHECK_ARGS(2, string, function);

  I2cReqWrap* req_wrap = new I2cReqWrap(JHANDLER_GET_ARG(1, function));
  I2cReqData* req_data = req_wrap->req();

  req_data->op = kI2cOpOpen;
  req_data->device = JHANDLER_GET_ARG(0, string);

  I2c* i2c = I2c::GetInstance();
  i2c->Open(req_wrap);

  iotjs_jhandler_return_null(jhandler);
}


JHANDLER_FUNCTION(Close) {
  JHANDLER_CHECK_ARGS(0);

  I2c* i2c = I2c::GetInstance();
  i2c->Close();

  iotjs_jhandler_return_null(jhandler);
}


JHANDLER_FUNCTION(Write) {
  JHANDLER_CHECK_ARGS(2, array, function);

  I2cReqWrap* req_wrap = new I2cReqWrap(JHANDLER_GET_ARG(1, function));
  I2cReqData* req_data = req_wrap->req();

  req_data->op = kI2cOpWrite;
  GetI2cArray(JHANDLER_GET_ARG(0, array), req_data);

  I2c* i2c = I2c::GetInstance();
  i2c->Write(req_wrap);

  iotjs_jhandler_return_null(jhandler);
}


JHANDLER_FUNCTION(WriteByte) {
  JHANDLER_CHECK_ARGS(2, number, function);

  uint8_t byte = JHANDLER_GET_ARG(0, number);

  I2cReqWrap* req_wrap = new I2cReqWrap(JHANDLER_GET_ARG(1, function));
  I2cReqData* req_data = req_wrap->req();

  req_data->op = kI2cOpWriteByte;
  req_data->byte = byte;

  I2c* i2c = I2c::GetInstance();
  i2c->WriteByte(req_wrap);

  iotjs_jhandler_return_null(jhandler);
}


JHANDLER_FUNCTION(WriteBlock) {
  JHANDLER_CHECK_ARGS(3, number, array, function);

  I2cReqWrap* req_wrap = new I2cReqWrap(JHANDLER_GET_ARG(2, function));
  I2cReqData* req_data = req_wrap->req();

  req_data->op = kI2cOpWriteBlock;
  req_data->cmd = JHANDLER_GET_ARG(0, number);
  GetI2cArray(JHANDLER_GET_ARG(1, array), req_data);

  I2c* i2c = I2c::GetInstance();
  i2c->WriteBlock(req_wrap);

  iotjs_jhandler_return_null(jhandler);
}


JHANDLER_FUNCTION(Read) {
  JHANDLER_CHECK_ARGS(2, number, function);

  I2cReqWrap* req_wrap = new I2cReqWrap(JHANDLER_GET_ARG(1, function));
  I2cReqData* req_data = req_wrap->req();

  req_data->op = kI2cOpRead;
  req_data->buf_len = JHANDLER_GET_ARG(0, number);
  req_data->delay = 0;

  I2c* i2c = I2c::GetInstance();
  i2c->Read(req_wrap);

  iotjs_jhandler_return_null(jhandler);
}


JHANDLER_FUNCTION(ReadByte) {
  JHANDLER_CHECK_ARGS(1, function);

  I2cReqWrap* req_wrap = new I2cReqWrap(JHANDLER_GET_ARG(0, function));
  I2cReqData* req_data = req_wrap->req();

  req_data->op = kI2cOpReadByte;

  I2c* i2c = I2c::GetInstance();
  i2c->ReadByte(req_wrap);

  iotjs_jhandler_return_null(jhandler);
}


JHANDLER_FUNCTION(ReadBlock) {
  JHANDLER_CHECK_ARGS(4, number, number, number, function);

  I2cReqWrap* req_wrap = new I2cReqWrap(JHANDLER_GET_ARG(0, function));
  I2cReqData* req_data = req_wrap->req();

  req_data->op = kI2cOpReadBlock;
  req_data->cmd = JHANDLER_GET_ARG(0, number);
  req_data->buf_len = JHANDLER_GET_ARG(1, number);
  req_data->delay = JHANDLER_GET_ARG(2, number);

  I2c* i2c = I2c::GetInstance();
  i2c->ReadBlock(req_wrap);

  iotjs_jhandler_return_null(jhandler);
}


iotjs_jval_t InitI2c() {

  iotjs_jval_t ji2c = iotjs_jval_create_object();

  iotjs_jval_set_method(&ji2c, "setAddress", SetAddress);
  iotjs_jval_set_method(&ji2c, "scan", Scan);
  iotjs_jval_set_method(&ji2c, "open", Open);
  iotjs_jval_set_method(&ji2c, "close", Close);
  iotjs_jval_set_method(&ji2c, "write", Write);
  iotjs_jval_set_method(&ji2c, "writeByte", WriteByte);
  iotjs_jval_set_method(&ji2c, "writeBlock", WriteBlock);
  iotjs_jval_set_method(&ji2c, "read", Read);
  iotjs_jval_set_method(&ji2c, "readByte", ReadByte);
  iotjs_jval_set_method(&ji2c, "readBlock", ReadBlock);

  I2c* i2c = I2c::Create(&ji2c);
  IOTJS_ASSERT(i2c == reinterpret_cast<I2c*>(
              iotjs_jval_get_object_native_handle(&ji2c)));

  return ji2c;
}


} // namespace iotjs
