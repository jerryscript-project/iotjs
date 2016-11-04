/* Copyright 2015-2016 Samsung Electronics Co., Ltd.
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


BufferWrap::BufferWrap(const iotjs_jval_t* jbuiltin, size_t length)
    : JObjectWrap(jbuiltin)
    , _buffer(NULL)
    , _length(length) {
  if (length > 0) {
    _buffer = iotjs_buffer_allocate(length);
    IOTJS_ASSERT(_buffer != NULL);
  }
}


BufferWrap::~BufferWrap() {
  if (_buffer != NULL) {
    iotjs_buffer_release(_buffer);
  }
}


BufferWrap* BufferWrap::FromJBufferBuiltin(const iotjs_jval_t* jbuiltin) {
  IOTJS_ASSERT(iotjs_jval_is_object(jbuiltin));
  BufferWrap* buffer = reinterpret_cast<BufferWrap*>(
          iotjs_jval_get_object_native_handle(jbuiltin));
  IOTJS_ASSERT(buffer != NULL);
  return buffer;
}


BufferWrap* BufferWrap::FromJBuffer(const iotjs_jval_t* jbuffer) {
  IOTJS_ASSERT(iotjs_jval_is_object(jbuffer));
  iotjs_jval_t jbuiltin = iotjs_jval_get_property(jbuffer, "_builtin");
  BufferWrap* buffer = FromJBufferBuiltin(&jbuiltin);
  iotjs_jval_destroy(&jbuiltin);
  return buffer;
}



iotjs_jval_t* BufferWrap::jbuiltin() {
  return jobject();
}


iotjs_jval_t BufferWrap::jbuffer() {
  return iotjs_jval_get_property(jbuiltin(), "_buffer");
}


char* BufferWrap::buffer() {
  return _buffer;
}


size_t BufferWrap::length() {
#ifndef NDEBUG
  iotjs_jval_t jbuf = jbuffer();
  iotjs_jval_t jlength = iotjs_jval_get_property(&jbuf, "length");
  int length = iotjs_jval_as_number(&jlength);
  IOTJS_ASSERT(static_cast<size_t>(length) == _length);
  iotjs_jval_destroy(&jbuf);
  iotjs_jval_destroy(&jlength);
#endif
  return _length;
}


static size_t BoundRange(int index, size_t low, size_t upper) {
  if (index < static_cast<int>(low)) {
    return low;
  }
  if (index > static_cast<int>(upper)) {
    return upper;
  }
  return index;
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


size_t BufferWrap::Copy(const char* src, size_t len) {
  return Copy(src, 0, len, 0);
}


size_t BufferWrap::Copy(const char* src,
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


iotjs_jval_t CreateBuffer(size_t len) {
  iotjs_jval_t* jglobal = iotjs_jval_get_global_object();

  iotjs_jval_t jBuffer = iotjs_jval_get_property(jglobal, "Buffer");
  IOTJS_ASSERT(iotjs_jval_is_function(&jBuffer));

  iotjs_jargs_t jargs = iotjs_jargs_create(1);
  iotjs_jargs_append_number(&jargs, len);

  iotjs_jval_t jres = iotjs_jhelper_call_ok(&jBuffer,
                                            iotjs_jval_get_undefined(), &jargs);
  IOTJS_ASSERT(iotjs_jval_is_object(&jres));

  iotjs_jargs_destroy(&jargs);
  iotjs_jval_destroy(&jBuffer);

  return jres;
}



JHANDLER_FUNCTION(Buffer) {
  JHANDLER_CHECK_THIS(object);
  JHANDLER_CHECK_ARGS(2, object, number);

  const iotjs_jval_t* jbuiltin = JHANDLER_GET_THIS(object);
  const iotjs_jval_t* jbuffer = JHANDLER_GET_ARG(0, object);
  int length = JHANDLER_GET_ARG(1, number);

  iotjs_jval_set_property_jval(jbuiltin, "_buffer", jbuffer);

  BufferWrap* buffer_wrap = new BufferWrap(jbuiltin, length);
  IOTJS_ASSERT(buffer_wrap ==
          (BufferWrap*)(iotjs_jval_get_object_native_handle(jbuiltin)));
  IOTJS_ASSERT(length == 0 || buffer_wrap->buffer() != NULL);
}


JHANDLER_FUNCTION(Compare) {
  JHANDLER_CHECK_THIS(object);
  JHANDLER_CHECK_ARGS(1, object);

  const iotjs_jval_t* jsrc_builtin = JHANDLER_GET_THIS(object);
  BufferWrap* src_buffer_wrap = BufferWrap::FromJBufferBuiltin(jsrc_builtin);

  const iotjs_jval_t* jdst_buffer = JHANDLER_GET_ARG(0, object);
  BufferWrap* dst_buffer_wrap = BufferWrap::FromJBuffer(jdst_buffer);

  int compare = src_buffer_wrap->Compare(*dst_buffer_wrap);
  iotjs_jhandler_return_number(jhandler, compare);
}


JHANDLER_FUNCTION(Copy) {
  JHANDLER_CHECK_THIS(object);
  JHANDLER_CHECK_ARGS(4, object, number, number, number);

  const iotjs_jval_t* jsrc_builtin = JHANDLER_GET_THIS(object);
  BufferWrap* src_buffer_wrap = BufferWrap::FromJBufferBuiltin(jsrc_builtin);

  const iotjs_jval_t* jdst_buffer = JHANDLER_GET_ARG(0, object);
  BufferWrap* dst_buffer_wrap = BufferWrap::FromJBuffer(jdst_buffer);

  int dst_start = JHANDLER_GET_ARG(1, number);
  int src_start = JHANDLER_GET_ARG(2, number);
  int src_end = JHANDLER_GET_ARG(3, number);

  dst_start = BoundRange(dst_start, 0, dst_buffer_wrap->length());
  src_start = BoundRange(src_start, 0, src_buffer_wrap->length());
  src_end = BoundRange(src_end, 0, src_buffer_wrap->length());

  if (src_end < src_start) {
    src_end = src_start;
  }

  int copied = dst_buffer_wrap->Copy(src_buffer_wrap->buffer(),
                                     src_start,
                                     src_end,
                                     dst_start);

  iotjs_jhandler_return_number(jhandler, copied);
}


JHANDLER_FUNCTION(Write) {
  JHANDLER_CHECK_ARGS(3, string, number, number);

  iotjs_string_t src = JHANDLER_GET_ARG(0, string);
  int offset = JHANDLER_GET_ARG(1, number);
  int length = JHANDLER_GET_ARG(2, number);

  const iotjs_jval_t* jbuiltin = JHANDLER_GET_THIS(object);

  BufferWrap* buffer_wrap = BufferWrap::FromJBufferBuiltin(jbuiltin);

  offset = BoundRange(offset, 0, buffer_wrap->length());
  length = BoundRange(length, 0, buffer_wrap->length() - offset);
  length = BoundRange(length, 0, iotjs_string_size(&src));

  size_t copied = buffer_wrap->Copy(iotjs_string_data(&src), 0, length, offset);

  iotjs_jhandler_return_number(jhandler, copied);

  iotjs_string_destroy(&src);
}


JHANDLER_FUNCTION(Slice) {
  JHANDLER_CHECK_ARGS(2, number, number);

  const iotjs_jval_t* jbuiltin = JHANDLER_GET_THIS(object);
  BufferWrap* buffer_wrap = BufferWrap::FromJBufferBuiltin(jbuiltin);

  int start = JHANDLER_GET_ARG(0, number);
  int end = JHANDLER_GET_ARG(1, number);

  if (start < 0) {
    start += buffer_wrap->length();
  }
  start = BoundRange(start, 0, buffer_wrap->length());

  if (end < 0) {
    end += buffer_wrap->length();
  }
  end = BoundRange(end, 0, buffer_wrap->length());

  if (end < start) {
    end = start;
  }

  int length = end - start;
  IOTJS_ASSERT(length >= 0);

  iotjs_jval_t jnew_buffer = CreateBuffer(length);
  BufferWrap* new_buffer_wrap = BufferWrap::FromJBuffer(&jnew_buffer);
  new_buffer_wrap->Copy(buffer_wrap->buffer(), start, end, 0);

  iotjs_jhandler_return_jval(jhandler, &jnew_buffer);
  iotjs_jval_destroy(&jnew_buffer);
}


JHANDLER_FUNCTION(ToString) {
  JHANDLER_CHECK_THIS(object);
  JHANDLER_CHECK_ARGS(2, number, number);

  const iotjs_jval_t* jbuiltin = JHANDLER_GET_THIS(object);
  BufferWrap* buffer_wrap = BufferWrap::FromJBufferBuiltin(jbuiltin);

  int start = JHANDLER_GET_ARG(0, number);
  int end = JHANDLER_GET_ARG(1, number);

  start = BoundRange(start, 0, buffer_wrap->length());
  end = BoundRange(end, 0, buffer_wrap->length());

  if (end < start) {
    end = start;
  }

  int length = end - start;
  IOTJS_ASSERT(length >= 0);

  const char* data = buffer_wrap->buffer() + start;
  length = strnlen(data, length);
  iotjs_string_t str = iotjs_string_create_with_size(data, length);

  iotjs_jhandler_return_string(jhandler, &str);

  iotjs_string_destroy(&str);
}


iotjs_jval_t InitBuffer() {

  iotjs_jval_t buffer = iotjs_jval_create_function(Buffer);

  iotjs_jval_t prototype = iotjs_jval_create_object();

  iotjs_jval_set_property_jval(&buffer, "prototype", &prototype);

  iotjs_jval_set_method(&prototype, "compare", Compare);
  iotjs_jval_set_method(&prototype, "copy", Copy);
  iotjs_jval_set_method(&prototype, "write", Write);
  iotjs_jval_set_method(&prototype, "slice", Slice);
  iotjs_jval_set_method(&prototype, "toString", ToString);

  iotjs_jval_destroy(&prototype);

  return buffer;
}


} // namespace iotjs


extern "C" {

iotjs_jval_t InitBuffer() {
  return iotjs::InitBuffer();
}

} // extern "C"
