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


BufferWrap::BufferWrap(JObject& jbuiltin,
                       size_t length)
    : JObjectWrap(jbuiltin)
    , _buffer(NULL)
    , _length(length) {
  if (length > 0) {
    _buffer = AllocBuffer(length);
    IOTJS_ASSERT(_buffer != NULL);
  }
}


BufferWrap::~BufferWrap() {
  if (_buffer != NULL) {
    ReleaseBuffer(_buffer);
  }
}


BufferWrap* BufferWrap::FromJBufferBuiltin(JObject& jbuiltin) {
  IOTJS_ASSERT(jbuiltin.IsObject());
  BufferWrap* buffer = reinterpret_cast<BufferWrap*>(jbuiltin.GetNative());
  IOTJS_ASSERT(buffer != NULL);
  return buffer;
}


BufferWrap* BufferWrap::FromJBuffer(JObject& jbuffer) {
  IOTJS_ASSERT(jbuffer.IsObject());
  JObject jbuiltin(jbuffer.GetProperty("_builtin"));
  return FromJBufferBuiltin(jbuiltin);
}



JObject BufferWrap::jbuiltin() {
  return jobject();
}


JObject BufferWrap::jbuffer() {
  return jbuiltin().GetProperty("_buffer");
}


char* BufferWrap::buffer() {
  return _buffer;
}


size_t BufferWrap::length() {
#ifndef NDEBUG
  int length = jbuffer().GetProperty("length").GetInt32();
  IOTJS_ASSERT(static_cast<size_t>(length) == _length);
#endif
  return _length;
}


int BufferWrap::Compare(const BufferWrap& other) const {
  size_t i = 0;
  size_t j = 0;
  while (i < _length && j < other._length) {
    if (_buffer[i] < other._buffer[j]) {
      return -1;
    } else if (_buffer[i] > other._buffer[j]) {
      return 1;
    }
    ++i;
    ++j;
  }
  if (j < other._length) {
    return -1;
  } else if (i < _length) {
    return 1;
  }
  return 0;
}


size_t BufferWrap::Copy(char* src, size_t len) {
  return Copy(src, 0, len, 0);
}


size_t BufferWrap::Copy(char* src,
                        size_t src_from,
                        size_t src_to,
                        size_t dst_from) {
  size_t copied = 0;
  size_t dst_length = _length;
  for (size_t i = src_from, j = dst_from;
       i < src_to && j < dst_length;
       ++i, ++j) {
    *(_buffer + j) = *(src + i);
    ++copied;
  }
  return copied;
}


JObject CreateBuffer(size_t len) {
  JObject jglobal(JObject::Global());
  IOTJS_ASSERT(jglobal.IsObject());

  JObject jBuffer(jglobal.GetProperty("Buffer"));
  IOTJS_ASSERT(jBuffer.IsFunction());

  JArgList jargs(1);
  jargs.Add(JVal::Number((int)len));

  JResult jres(jBuffer.Call(JObject::Null(), jargs));
  IOTJS_ASSERT(jres.IsOk());
  IOTJS_ASSERT(jres.value().IsObject());

  return jres.value();
}



JHANDLER_FUNCTION(Buffer) {
  JHANDLER_CHECK(handler.GetThis()->IsObject());
  JHANDLER_CHECK(handler.GetArgLength() == 2);
  JHANDLER_CHECK(handler.GetArg(0)->IsObject());
  JHANDLER_CHECK(handler.GetArg(1)->IsNumber());

  int length = handler.GetArg(1)->GetInt32();
  JObject* jbuffer = handler.GetArg(0);
  JObject* jbuiltin = handler.GetThis();

  jbuiltin->SetProperty("_buffer", *jbuffer);

  BufferWrap* buffer_wrap = new BufferWrap(*jbuiltin, length);
  IOTJS_ASSERT(buffer_wrap == (BufferWrap*)(jbuiltin->GetNative()));
  IOTJS_ASSERT(length == 0 || buffer_wrap->buffer() != NULL);

  return true;
}


JHANDLER_FUNCTION(Compare) {
  JHANDLER_CHECK(handler.GetThis()->IsObject());
  JHANDLER_CHECK(handler.GetArgLength() == 1);
  JHANDLER_CHECK(handler.GetArg(0)->IsObject());

  JObject* jsrc_builtin = handler.GetThis();
  BufferWrap* src_buffer_wrap = BufferWrap::FromJBufferBuiltin(*jsrc_builtin);

  JObject* jdst_buffer = handler.GetArg(0);
  BufferWrap* dst_buffer_wrap = BufferWrap::FromJBuffer(*jdst_buffer);

  handler.Return(JVal::Number(src_buffer_wrap->Compare(*dst_buffer_wrap)));

  return true;
}


JHANDLER_FUNCTION(Copy) {
  JHANDLER_CHECK(handler.GetThis()->IsObject());
  JHANDLER_CHECK(handler.GetArgLength() == 4);
  JHANDLER_CHECK(handler.GetArg(0)->IsObject());
  JHANDLER_CHECK(handler.GetArg(1)->IsNumber());
  JHANDLER_CHECK(handler.GetArg(2)->IsNumber());
  JHANDLER_CHECK(handler.GetArg(3)->IsNumber());

  JObject* jsrc_builtin = handler.GetThis();
  BufferWrap* src_buffer_wrap = BufferWrap::FromJBufferBuiltin(*jsrc_builtin);

  JObject* jdst_buffer = handler.GetArg(0);
  BufferWrap* dst_buffer_wrap = BufferWrap::FromJBuffer(*jdst_buffer);

  int dst_start = handler.GetArg(1)->GetInt32();
  int src_start = handler.GetArg(2)->GetInt32();
  int src_end = handler.GetArg(3)->GetInt32();

  int copied = dst_buffer_wrap->Copy(src_buffer_wrap->buffer(),
                                     src_start,
                                     src_end,
                                     dst_start);

  handler.Return(JVal::Number(copied));

  return true;
}


JHANDLER_FUNCTION(Write) {
  JHANDLER_CHECK(handler.GetArgLength() == 3);
  JHANDLER_CHECK(handler.GetArg(0)->IsString());
  JHANDLER_CHECK(handler.GetArg(1)->IsNumber());
  JHANDLER_CHECK(handler.GetArg(2)->IsNumber());

  String src = handler.GetArg(0)->GetString();
  int offset = handler.GetArg(1)->GetInt32();
  int length = handler.GetArg(2)->GetInt32();

  JObject* jbuiltin = handler.GetThis();

  BufferWrap* buffer_wrap = BufferWrap::FromJBufferBuiltin(*jbuiltin);

  size_t copied = buffer_wrap->Copy(src.data(), 0, length, offset);

  handler.Return(JVal::Number((int)copied));

  return true;
}


JHANDLER_FUNCTION(Slice) {
  JHANDLER_CHECK(handler.GetArgLength() == 2);
  JHANDLER_CHECK(handler.GetArg(0)->IsNumber());
  JHANDLER_CHECK(handler.GetArg(1)->IsNumber());

  JObject* jbuiltin = handler.GetThis();
  BufferWrap* buffer_wrap = BufferWrap::FromJBufferBuiltin(*jbuiltin);

  int start = handler.GetArg(0)->GetInt32();
  int end = handler.GetArg(1)->GetInt32();
  int length = end - start;
  JHANDLER_CHECK(length >= 0);

  JObject jnew_buffer = CreateBuffer(length);
  BufferWrap* new_buffer_wrap = BufferWrap::FromJBuffer(jnew_buffer);
  new_buffer_wrap->Copy(buffer_wrap->buffer(), start, end, 0);

  handler.Return(jnew_buffer);

  return true;
}


JHANDLER_FUNCTION(ToString) {
  JHANDLER_CHECK(handler.GetThis()->IsObject());
  JHANDLER_CHECK(handler.GetArgLength() == 2);
  JHANDLER_CHECK(handler.GetArg(0)->IsNumber());
  JHANDLER_CHECK(handler.GetArg(1)->IsNumber());

  JObject* jbuiltin = handler.GetThis();
  BufferWrap* buffer_wrap = BufferWrap::FromJBufferBuiltin(*jbuiltin);

  int start = handler.GetArg(0)->GetInt32();
  int end = handler.GetArg(1)->GetInt32();
  int length = end - start;
  JHANDLER_CHECK(length >= 0);

  String str("", length + 1);

  strncpy(str.data(), buffer_wrap->buffer() + start, length);

  JObject ret(str);
  handler.Return(ret);

  return true;
}


JObject* InitBuffer() {
  Module* module = GetBuiltinModule(MODULE_BUFFER);
  JObject* buffer = module->module;

  if (buffer == NULL) {
    buffer = new JObject(Buffer);

    JObject prototype;
    buffer->SetProperty("prototype", prototype);

    prototype.SetMethod("compare", Compare);
    prototype.SetMethod("copy", Copy);
    prototype.SetMethod("write", Write);
    prototype.SetMethod("slice", Slice);
    prototype.SetMethod("toString", ToString);

    module->module = buffer;
  }

  return buffer;
}


} // namespace iotjs
