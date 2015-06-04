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

#include "iotjs_def.h"
#include "iotjs_module_buffer.h"

#include <stdlib.h>
#include <string.h>


namespace iotjs {


JHANDLER_FUNCTION(Write, handler) {
  IOTJS_ASSERT(handler.GetArgLength() == 3);
  IOTJS_ASSERT(handler.GetArg(0)->IsString());
  IOTJS_ASSERT(handler.GetArg(1)->IsNumber());
  IOTJS_ASSERT(handler.GetArg(2)->IsNumber());

  LocalString src(handler.GetArg(0)->GetCString());

  int offset = handler.GetArg(1)->GetInt32();
  int length = handler.GetArg(2)->GetInt32();

  JObject* jbuffer = handler.GetThis();
  Buffer* buffer = Buffer::FromJBuffer(*jbuffer);
  char* buffer_p = buffer->buffer();
  int buffer_length = buffer->length();
  IOTJS_ASSERT(buffer_length >= offset + length);

  for (int i = 0; i < length; ++i) {
    *(buffer_p + offset + i) = *(src + i);
  }

  handler.Return(JVal::Int(length));

  return true;
}


JHANDLER_FUNCTION(ToString, handler) {
  IOTJS_ASSERT(handler.GetArgLength() == 0);

  JObject* jbuffer = handler.GetThis();
  Buffer* buffer = Buffer::FromJBuffer(*jbuffer);
  int length = buffer->length();

  LocalString str(length + 1);

  strncpy(str, buffer->buffer(), length);
  str[length] = 0;

  JObject ret(str);
  handler.Return(ret);

  return true;
}


JHANDLER_FUNCTION(Copy, handler) {
  uint16_t args = handler.GetArgLength();
  IOTJS_ASSERT(args >= 1);
  IOTJS_ASSERT(handler.GetArg(0)->IsObject());
  IOTJS_ASSERT(args <= 1 || handler.GetArg(1)->IsNumber());
  IOTJS_ASSERT(args <= 2 || handler.GetArg(2)->IsNumber());
  IOTJS_ASSERT(args <= 3 || handler.GetArg(3)->IsNumber());

  JObject* jsrc_buffer = handler.GetThis();
  Buffer* src_buffer = Buffer::FromJBuffer(*jsrc_buffer);

  JObject* jdst_buffer = handler.GetArg(0);
  Buffer* dst_buffer = Buffer::FromJBuffer(*jdst_buffer);

  int dst_start = 0;
  int src_start = 0;
  int src_length = src_buffer->length();
  int src_end = src_length;

  if (args >= 2) {
    dst_start = handler.GetArg(1)->GetInt32();
  }
  if (args >= 3) {
    src_start = handler.GetArg(2)->GetInt32();
  }
  if (args >= 4) {
    src_end = handler.GetArg(3)->GetInt32();
    if (src_end > src_length) {
      src_end = src_length;
    }
  }

  int copied = dst_buffer->Copy(src_buffer->buffer(),
                                src_end - src_start,
                                src_start,
                                dst_start);

  handler.Return(JVal::Int(copied));

  return true;
}


JHANDLER_FUNCTION(SetupBufferJs, handler) {
  IOTJS_ASSERT(handler.GetArgLength() == 1);
  IOTJS_ASSERT(handler.GetArg(0)->IsFunction());

  JObject* jbuffer = handler.GetArg(0);
  JObject prototype(jbuffer->GetProperty("prototype"));
  prototype.SetMethod("_write", Write);
  prototype.SetMethod("_toString", ToString);
  prototype.SetMethod("copy", Copy);

  return true;
}


JHANDLER_FUNCTION(Alloc, handler) {
  IOTJS_ASSERT(handler.GetArgLength() == 2);
  IOTJS_ASSERT(handler.GetArg(0)->IsObject());
  IOTJS_ASSERT(handler.GetArg(1)->IsNumber());

  JObject* jbuffer = handler.GetArg(0);
  int length = handler.GetArg(1)->GetInt32();
  Buffer* buffer = new Buffer(*jbuffer, length);
  IOTJS_ASSERT(buffer == reinterpret_cast<Buffer*>(jbuffer->GetNative()));
  IOTJS_ASSERT(buffer->buffer() != NULL);

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


JObject CreateBuffer(size_t len) {
  JObject jglobal(JObject::Global());
  IOTJS_ASSERT(jglobal.IsObject());

  JObject jBuffer = jglobal.GetProperty("Buffer");
  IOTJS_ASSERT(jBuffer.IsFunction());

  JArgList jargs(1);
  jargs.Add(JVal::Int(len));

  JObject jbuffer = jBuffer.Call(JObject::Null(), jargs);
  IOTJS_ASSERT(jbuffer.IsObject());

  return jbuffer;
}


Buffer::Buffer(JObject& jbuffer, size_t length)
    : JObjectWrap(jbuffer)
    , _buffer(NULL)
    , _length(length) {
  _buffer = AllocBuffer(length);
  IOTJS_ASSERT(_buffer != NULL);
}


Buffer::~Buffer() {
  if (_buffer != NULL) {
    ReleaseBuffer(_buffer);
  }
}


Buffer* Buffer::FromJBuffer(JObject& jbuffer) {
  Buffer* buffer = reinterpret_cast<Buffer*>(jbuffer.GetNative());
  IOTJS_ASSERT(buffer != NULL);
  return buffer;
}


JObject& Buffer::jbuffer() {
  return jobject();
}


char* Buffer::buffer() {
  return _buffer;
}


size_t Buffer::length() {
#ifndef NDEBUG
  int length = jbuffer().GetProperty("length").GetInt32();
  IOTJS_ASSERT(static_cast<size_t>(length) == _length);
#endif
  return _length;
}


size_t Buffer::Copy(char* src, size_t len) {
  return Copy(src, len, 0, 0);
}


size_t Buffer::Copy(char* src, size_t len, size_t src_from, size_t dst_from) {
  size_t copied = 0;

  size_t src_end = src_from + len;
  size_t dst_length = _length;

  for (size_t i = src_from, j = dst_from;
       i < src_end && j < dst_length;
       ++i, ++j) {
    *(_buffer + j) = *(src + i);
    ++copied;
  }

  return copied;
}

} // namespace iotjs
