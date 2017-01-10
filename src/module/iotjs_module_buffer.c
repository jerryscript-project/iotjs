/* Copyright 2015-present Samsung Electronics Co., Ltd. and other contributors
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


static void iotjs_bufferwrap_destroy(iotjs_bufferwrap_t* bufferwrap);

iotjs_bufferwrap_t* iotjs_bufferwrap_create(const iotjs_jval_t* jbuiltin,
                                            size_t length) {
  iotjs_bufferwrap_t* bufferwrap = IOTJS_ALLOC(iotjs_bufferwrap_t);
  IOTJS_VALIDATED_STRUCT_CONSTRUCTOR(iotjs_bufferwrap_t, bufferwrap);

  iotjs_jobjectwrap_initialize(&_this->jobjectwrap, jbuiltin,
                               (JFreeHandlerType)iotjs_bufferwrap_destroy);
  if (length > 0) {
    _this->length = length;
    _this->buffer = iotjs_buffer_allocate(length);
    IOTJS_ASSERT(_this->buffer != NULL);
  } else {
    _this->length = 0;
    _this->buffer = NULL;
  }

  IOTJS_ASSERT(
      bufferwrap ==
      (iotjs_bufferwrap_t*)(iotjs_jval_get_object_native_handle(jbuiltin)));

  return bufferwrap;
}


static void iotjs_bufferwrap_destroy(iotjs_bufferwrap_t* bufferwrap) {
  IOTJS_VALIDATED_STRUCT_DESTRUCTOR(iotjs_bufferwrap_t, bufferwrap);
  if (_this->buffer != NULL) {
    iotjs_buffer_release(_this->buffer);
  }
  iotjs_jobjectwrap_destroy(&_this->jobjectwrap);
  IOTJS_RELEASE(bufferwrap);
}


iotjs_bufferwrap_t* iotjs_bufferwrap_from_jbuiltin(
    const iotjs_jval_t* jbuiltin) {
  IOTJS_ASSERT(iotjs_jval_is_object(jbuiltin));
  iotjs_bufferwrap_t* buffer =
      (iotjs_bufferwrap_t*)iotjs_jval_get_object_native_handle(jbuiltin);
  IOTJS_ASSERT(buffer != NULL);
  return buffer;
}


iotjs_bufferwrap_t* iotjs_bufferwrap_from_jbuffer(const iotjs_jval_t* jbuffer) {
  IOTJS_ASSERT(iotjs_jval_is_object(jbuffer));
  iotjs_jval_t jbuiltin = iotjs_jval_get_property(jbuffer, "_builtin");
  iotjs_bufferwrap_t* buffer = iotjs_bufferwrap_from_jbuiltin(&jbuiltin);
  iotjs_jval_destroy(&jbuiltin);
  return buffer;
}


iotjs_jval_t* iotjs_bufferwrap_jbuiltin(iotjs_bufferwrap_t* bufferwrap) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_bufferwrap_t, bufferwrap);
  return iotjs_jobjectwrap_jobject(&_this->jobjectwrap);
}


iotjs_jval_t iotjs_bufferwrap_jbuffer(iotjs_bufferwrap_t* bufferwrap) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_bufferwrap_t, bufferwrap);
  iotjs_jval_t* jbuiltin = iotjs_bufferwrap_jbuiltin(bufferwrap);
  return iotjs_jval_get_property(jbuiltin, "_buffer");
}


char* iotjs_bufferwrap_buffer(iotjs_bufferwrap_t* bufferwrap) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_bufferwrap_t, bufferwrap);
  return _this->buffer;
}


size_t iotjs_bufferwrap_length(iotjs_bufferwrap_t* bufferwrap) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_bufferwrap_t, bufferwrap);
#ifndef NDEBUG
  iotjs_jval_t jbuf = iotjs_bufferwrap_jbuffer(bufferwrap);
  iotjs_jval_t jlength = iotjs_jval_get_property(&jbuf, "length");
  size_t length = iotjs_jval_as_number(&jlength);
  IOTJS_ASSERT(length == _this->length);
  iotjs_jval_destroy(&jbuf);
  iotjs_jval_destroy(&jlength);
#endif
  return _this->length;
}


static size_t bound_range(int index, size_t low, size_t upper) {
  if (index < (int)low) {
    return low;
  }
  if (index > (int)upper) {
    return upper;
  }
  return index;
}


int iotjs_bufferwrap_compare(const iotjs_bufferwrap_t* bufferwrap,
                             const iotjs_bufferwrap_t* other) {
  const IOTJS_VALIDATED_STRUCT_METHOD(iotjs_bufferwrap_t, bufferwrap);

  const char* other_buffer = other->unsafe.buffer;
  size_t other_length = other->unsafe.length;

  size_t i = 0;
  size_t j = 0;
  while (i < _this->length && j < other_length) {
    if (_this->buffer[i] < other_buffer[j]) {
      return -1;
    } else if (_this->buffer[i] > other_buffer[j]) {
      return 1;
    }
    ++i;
    ++j;
  }
  if (j < other_length) {
    return -1;
  } else if (i < _this->length) {
    return 1;
  }
  return 0;
}

size_t iotjs_bufferwrap_copy_internal(iotjs_bufferwrap_t* bufferwrap,
                                      const char* src, size_t src_from,
                                      size_t src_to, size_t dst_from) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_bufferwrap_t, bufferwrap);
  size_t copied = 0;
  size_t dst_length = _this->length;
  for (size_t i = src_from, j = dst_from; i < src_to && j < dst_length;
       ++i, ++j) {
    *(_this->buffer + j) = *(src + i);
    ++copied;
  }
  return copied;
}


size_t iotjs_bufferwrap_copy(iotjs_bufferwrap_t* bufferwrap, const char* src,
                             size_t len) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_bufferwrap_t, bufferwrap);
  return iotjs_bufferwrap_copy_internal(bufferwrap, src, 0, len, 0);
}


iotjs_jval_t iotjs_bufferwrap_create_buffer(size_t len) {
  iotjs_jval_t* jglobal = iotjs_jval_get_global_object();

  iotjs_jval_t jbuffer = iotjs_jval_get_property(jglobal, "Buffer");
  IOTJS_ASSERT(iotjs_jval_is_function(&jbuffer));

  iotjs_jargs_t jargs = iotjs_jargs_create(1);
  iotjs_jargs_append_number(&jargs, len);

  iotjs_jval_t jres =
      iotjs_jhelper_call_ok(&jbuffer, iotjs_jval_get_undefined(), &jargs);
  IOTJS_ASSERT(iotjs_jval_is_object(&jres));

  iotjs_jargs_destroy(&jargs);
  iotjs_jval_destroy(&jbuffer);

  return jres;
}


JHANDLER_FUNCTION(Buffer) {
  JHANDLER_CHECK_THIS(object);
  JHANDLER_CHECK_ARGS(2, object, number);

  const iotjs_jval_t* jbuiltin = JHANDLER_GET_THIS(object);
  const iotjs_jval_t* jbuffer = JHANDLER_GET_ARG(0, object);
  int length = JHANDLER_GET_ARG(1, number);

  iotjs_jval_set_property_jval(jbuiltin, "_buffer", jbuffer);

  iotjs_bufferwrap_t* buffer_wrap = iotjs_bufferwrap_create(jbuiltin, length);
}


JHANDLER_FUNCTION(Compare) {
  JHANDLER_CHECK_THIS(object);
  JHANDLER_CHECK_ARGS(1, object);

  const iotjs_jval_t* jsrc_builtin = JHANDLER_GET_THIS(object);
  iotjs_bufferwrap_t* src_buffer_wrap =
      iotjs_bufferwrap_from_jbuiltin(jsrc_builtin);

  const iotjs_jval_t* jdst_buffer = JHANDLER_GET_ARG(0, object);
  iotjs_bufferwrap_t* dst_buffer_wrap =
      iotjs_bufferwrap_from_jbuffer(jdst_buffer);

  int compare = iotjs_bufferwrap_compare(src_buffer_wrap, dst_buffer_wrap);
  iotjs_jhandler_return_number(jhandler, compare);
}


JHANDLER_FUNCTION(Copy) {
  JHANDLER_CHECK_THIS(object);
  JHANDLER_CHECK_ARGS(4, object, number, number, number);

  const iotjs_jval_t* jsrc_builtin = JHANDLER_GET_THIS(object);
  iotjs_bufferwrap_t* src_buffer_wrap =
      iotjs_bufferwrap_from_jbuiltin(jsrc_builtin);

  const iotjs_jval_t* jdst_buffer = JHANDLER_GET_ARG(0, object);
  iotjs_bufferwrap_t* dst_buffer_wrap =
      iotjs_bufferwrap_from_jbuffer(jdst_buffer);

  int dst_start = JHANDLER_GET_ARG(1, number);
  int src_start = JHANDLER_GET_ARG(2, number);
  int src_end = JHANDLER_GET_ARG(3, number);

  size_t dst_length = iotjs_bufferwrap_length(dst_buffer_wrap);
  size_t src_length = iotjs_bufferwrap_length(src_buffer_wrap);

  dst_start = bound_range(dst_start, 0, dst_length);
  src_start = bound_range(src_start, 0, src_length);
  src_end = bound_range(src_end, 0, src_length);

  if (src_end < src_start) {
    src_end = src_start;
  }

  const char* src_data = iotjs_bufferwrap_buffer(src_buffer_wrap);
  size_t copied = iotjs_bufferwrap_copy_internal(dst_buffer_wrap, src_data,
                                                 src_start, src_end, dst_start);

  iotjs_jhandler_return_number(jhandler, copied);
}


JHANDLER_FUNCTION(Write) {
  JHANDLER_CHECK_ARGS(3, string, number, number);

  iotjs_string_t src = JHANDLER_GET_ARG(0, string);
  int offset = JHANDLER_GET_ARG(1, number);
  int length = JHANDLER_GET_ARG(2, number);

  const iotjs_jval_t* jbuiltin = JHANDLER_GET_THIS(object);

  iotjs_bufferwrap_t* buffer_wrap = iotjs_bufferwrap_from_jbuiltin(jbuiltin);

  size_t buffer_length = iotjs_bufferwrap_length(buffer_wrap);
  offset = bound_range(offset, 0, buffer_length);
  length = bound_range(length, 0, buffer_length - offset);
  length = bound_range(length, 0, iotjs_string_size(&src));

  const char* src_data = iotjs_string_data(&src);
  size_t copied =
      iotjs_bufferwrap_copy_internal(buffer_wrap, src_data, 0, length, offset);

  iotjs_jhandler_return_number(jhandler, copied);

  iotjs_string_destroy(&src);
}


JHANDLER_FUNCTION(Slice) {
  JHANDLER_CHECK_ARGS(2, number, number);

  const iotjs_jval_t* jbuiltin = JHANDLER_GET_THIS(object);
  iotjs_bufferwrap_t* buffer_wrap = iotjs_bufferwrap_from_jbuiltin(jbuiltin);

  int start = JHANDLER_GET_ARG(0, number);
  int end = JHANDLER_GET_ARG(1, number);

  if (start < 0) {
    start += iotjs_bufferwrap_length(buffer_wrap);
  }
  start = bound_range(start, 0, iotjs_bufferwrap_length(buffer_wrap));

  if (end < 0) {
    end += iotjs_bufferwrap_length(buffer_wrap);
  }
  end = bound_range(end, 0, iotjs_bufferwrap_length(buffer_wrap));

  if (end < start) {
    end = start;
  }

  int length = end - start;
  IOTJS_ASSERT(length >= 0);

  iotjs_jval_t jnew_buffer = iotjs_bufferwrap_create_buffer(length);
  iotjs_bufferwrap_t* new_buffer_wrap =
      iotjs_bufferwrap_from_jbuffer(&jnew_buffer);
  iotjs_bufferwrap_copy_internal(new_buffer_wrap,
                                 iotjs_bufferwrap_buffer(buffer_wrap), start,
                                 end, 0);

  iotjs_jhandler_return_jval(jhandler, &jnew_buffer);
  iotjs_jval_destroy(&jnew_buffer);
}


JHANDLER_FUNCTION(ToString) {
  JHANDLER_CHECK_THIS(object);
  JHANDLER_CHECK_ARGS(2, number, number);

  const iotjs_jval_t* jbuiltin = JHANDLER_GET_THIS(object);
  iotjs_bufferwrap_t* buffer_wrap = iotjs_bufferwrap_from_jbuiltin(jbuiltin);

  int start = JHANDLER_GET_ARG(0, number);
  int end = JHANDLER_GET_ARG(1, number);

  start = bound_range(start, 0, iotjs_bufferwrap_length(buffer_wrap));
  end = bound_range(end, 0, iotjs_bufferwrap_length(buffer_wrap));

  if (end < start) {
    end = start;
  }

  int length = end - start;
  IOTJS_ASSERT(length >= 0);

  const char* data = iotjs_bufferwrap_buffer(buffer_wrap) + start;
  length = strnlen(data, length);
  iotjs_string_t str = iotjs_string_create_with_size(data, length);

  iotjs_jhandler_return_string(jhandler, &str);

  iotjs_string_destroy(&str);
}


JHANDLER_FUNCTION(ByteLength) {
  JHANDLER_CHECK_THIS(object);
  JHANDLER_CHECK_ARGS(1, string);

  iotjs_string_t str = JHANDLER_GET_ARG(0, string);
  iotjs_jval_t size = iotjs_jval_get_string_size(&str);

  iotjs_jhandler_return_jval(jhandler, &size);
  iotjs_string_destroy(&str);
  iotjs_jval_destroy(&size);
}


iotjs_jval_t InitBuffer() {
  iotjs_jval_t buffer = iotjs_jval_create_function(Buffer);

  iotjs_jval_t prototype = iotjs_jval_create_object();
  iotjs_jval_t byte_length = iotjs_jval_create_function(ByteLength);

  iotjs_jval_set_property_jval(&buffer, "prototype", &prototype);
  iotjs_jval_set_property_jval(&buffer, "byteLength", &byte_length);

  iotjs_jval_set_method(&prototype, "compare", Compare);
  iotjs_jval_set_method(&prototype, "copy", Copy);
  iotjs_jval_set_method(&prototype, "write", Write);
  iotjs_jval_set_method(&prototype, "slice", Slice);
  iotjs_jval_set_method(&prototype, "toString", ToString);

  iotjs_jval_destroy(&prototype);
  iotjs_jval_destroy(&byte_length);

  return buffer;
}
