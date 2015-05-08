/* Copyright 2015 Samsung Electronics Co., Ltd.
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

#include <stdlib.h>

#include "iotjs_binding.h"
#include "iotjs_module.h"
#include "iotjs_module_buffer.h"


namespace iotjs {

JHANDLER_FUNCTION(Write, handler) {
  assert(handler.GetArgLength() == 3);
  assert(handler.GetArg(0)->IsString());
  assert(handler.GetArg(1)->IsNumber());
  assert(handler.GetArg(2)->IsNumber());

  char* src = handler.GetArg(0)->GetCString();

  int offset = handler.GetArg(1)->GetInt32();
  int length = handler.GetArg(2)->GetInt32();

  JObject* jbuffer = handler.GetThis();
  char* buffer = reinterpret_cast<char*>(jbuffer->GetNative());
  assert(buffer != NULL);

  int buffer_length = jbuffer->GetProperty("length").GetInt32();
  assert(buffer_length >= offset + length);

  for (int i = 0; i < length; ++i) {
    *(buffer + offset + i) = *(src + i);
  }

  JObject::ReleaseCString(src);

  JObject ret(length);
  handler.Return(ret);

  return true;
}


JHANDLER_FUNCTION(ToString, handler) {
  assert(handler.GetArgLength() == 0);

  JObject* jbuffer = handler.GetThis();
  char* buffer = reinterpret_cast<char*>(jbuffer->GetNative());
  assert(buffer != NULL);

  JObject ret(buffer);
  handler.Return(ret);

  return true;
}


JHANDLER_FUNCTION(Copy, handler) {
  uint16_t args = handler.GetArgLength();
  assert(args >= 1);
  assert(handler.GetArg(0)->IsObject());
  assert(args <= 1 || handler.GetArg(1)->IsNumber());
  assert(args <= 2 || handler.GetArg(2)->IsNumber());
  assert(args <= 3 || handler.GetArg(3)->IsNumber());

  JObject* jsource_buffer = handler.GetThis();
  int source_length = jsource_buffer->GetProperty("length").GetInt32();
  char* source_buffer = reinterpret_cast<char*>(jsource_buffer->GetNative());
  assert(source_buffer != NULL);

  JObject* jtarget_buffer = handler.GetArg(0);
  int target_length = jtarget_buffer->GetProperty("length").GetInt32();
  char* target_buffer = reinterpret_cast<char*>(jtarget_buffer->GetNative());
  assert(target_buffer != NULL);

  int target_start = 0;
  int source_start = 0;
  int source_end = source_length;

  if (args >= 2) {
    target_start = handler.GetArg(1)->GetInt32();
  }
  if (args >= 3) {
    source_start = handler.GetArg(2)->GetInt32();
  }
  if (args >= 4) {
    source_end = handler.GetArg(3)->GetInt32();
    if (source_end > source_length) {
      source_end = source_length;
    }
  }

  int copied = 0;
  for (int i = source_start, j = target_start;
       i < source_end && j < target_length;
       ++i, ++j) {
    *(target_buffer + j) = *(source_buffer + i);
    ++copied;
  }

  JObject ret(copied);
  handler.Return(ret);

  return true;
}


JHANDLER_FUNCTION(SetupBufferJs, handler) {
  assert(handler.GetArgLength() == 1);
  assert(handler.GetArg(0)->IsFunction());

  JObject* jbuffer = handler.GetArg(0);
  JObject prototype(jbuffer->GetProperty("prototype"));
  prototype.SetMethod("_write", Write);
  prototype.SetMethod("_toString", ToString);
  prototype.SetMethod("copy", Copy);

  return true;
}


JHANDLER_FUNCTION(Alloc, handler) {
  assert(handler.GetArgLength() == 2);
  assert(handler.GetArg(0)->IsObject());
  assert(handler.GetArg(1)->IsNumber());

  int length = handler.GetArg(1)->GetInt32();
  char* buffer = AllocCharBuffer(length);

  JObject* buffer_obj = handler.GetArg(0);
  buffer_obj->SetNative(reinterpret_cast<uintptr_t>(buffer));

  JObject ret(length);
  handler.Return(ret);

  return true;
}


JObject* InitBuffer() {
  Module* module = GetBuiltinModule(MODULE_BUFFER);
  JObject* buffer = module->module;

  if (buffer == NULL) {
    buffer = new JObject();
    buffer->SetMethod("setupBufferJs", SetupBufferJs);
    buffer->SetMethod("alloc", Alloc);

    module->module = buffer;
  }

  return buffer;
}

} // namespace iotjs
