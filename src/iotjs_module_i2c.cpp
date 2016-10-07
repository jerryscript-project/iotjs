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

JHANDLER_FUNCTION(SetAddress) {
  IOTJS_ASSERT(!"Not implemented");

  handler.Return(JVal::Null());
}


JHANDLER_FUNCTION(Scan) {
  IOTJS_ASSERT(!"Not implemented");

  handler.Return(JVal::Null());
}


JHANDLER_FUNCTION(Open) {
  IOTJS_ASSERT(!"Not implemented");

  handler.Return(JVal::Null());
}


JHANDLER_FUNCTION(Close) {
  IOTJS_ASSERT(!"Not implemented");

  handler.Return(JVal::Null());
}

JHANDLER_FUNCTION(Write) {
  IOTJS_ASSERT(!"Not implemented");

  handler.Return(JVal::Null());
}

JHANDLER_FUNCTION(WriteByte) {
  IOTJS_ASSERT(!"Not implemented");

  handler.Return(JVal::Null());
}

JHANDLER_FUNCTION(WriteBlock) {
  IOTJS_ASSERT(!"Not implemented");

  handler.Return(JVal::Null());
}

JHANDLER_FUNCTION(Read) {
  IOTJS_ASSERT(!"Not implemented");

  handler.Return(JVal::Null());
}

JHANDLER_FUNCTION(ReadByte) {
  IOTJS_ASSERT(!"Not implemented");

  handler.Return(JVal::Null());
}

JHANDLER_FUNCTION(ReadBlock) {
  IOTJS_ASSERT(!"Not implemented");

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

    // TODO: Need to implement Create for each platform.
    // I2c* i2c = I2c::Create(*ji2c);
    // IOTJS_ASSERT(i2c == reinterpret_cast<I2c*>(ji2c->GetNative()));

    module->module = ji2c;
  }

  return ji2c;
}


} // namespace iotjs
