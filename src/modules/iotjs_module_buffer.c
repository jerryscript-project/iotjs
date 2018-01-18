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

#include <math.h>
#include <stdlib.h>
#include <string.h>


IOTJS_DEFINE_NATIVE_HANDLE_INFO_THIS_MODULE(bufferwrap);


iotjs_bufferwrap_t* iotjs_bufferwrap_create(const jerry_value_t jbuiltin,
                                            size_t length) {
  iotjs_bufferwrap_t* bufferwrap = IOTJS_ALLOC(iotjs_bufferwrap_t);

  bufferwrap->jobject = jbuiltin;
  jerry_set_object_native_pointer(jbuiltin, bufferwrap,
                                  &this_module_native_info);

  if (length > 0) {
    bufferwrap->length = length;
    bufferwrap->buffer = iotjs_buffer_allocate(length);
    IOTJS_ASSERT(bufferwrap->buffer != NULL);
  } else {
    bufferwrap->length = 0;
    bufferwrap->buffer = NULL;
  }

  IOTJS_ASSERT(
      bufferwrap ==
      (iotjs_bufferwrap_t*)(iotjs_jval_get_object_native_handle(jbuiltin)));

  return bufferwrap;
}


static void iotjs_bufferwrap_destroy(iotjs_bufferwrap_t* bufferwrap) {
  if (bufferwrap->buffer != NULL) {
    iotjs_buffer_release(bufferwrap->buffer);
  }
  IOTJS_RELEASE(bufferwrap);
}


iotjs_bufferwrap_t* iotjs_bufferwrap_from_jbuiltin(
    const jerry_value_t jbuiltin) {
  IOTJS_ASSERT(jerry_value_is_object(jbuiltin));
  iotjs_bufferwrap_t* buffer =
      (iotjs_bufferwrap_t*)iotjs_jval_get_object_native_handle(jbuiltin);
  IOTJS_ASSERT(buffer != NULL);
  return buffer;
}


iotjs_bufferwrap_t* iotjs_bufferwrap_from_jbuffer(const jerry_value_t jbuffer) {
  IOTJS_ASSERT(jerry_value_is_object(jbuffer));
  jerry_value_t jbuiltin =
      iotjs_jval_get_property(jbuffer, IOTJS_MAGIC_STRING__BUILTIN);
  iotjs_bufferwrap_t* buffer = iotjs_bufferwrap_from_jbuiltin(jbuiltin);
  jerry_release_value(jbuiltin);
  return buffer;
}


size_t iotjs_bufferwrap_length(iotjs_bufferwrap_t* bufferwrap) {
#ifndef NDEBUG
  jerry_value_t jbuf =
      iotjs_jval_get_property(bufferwrap->jobject, IOTJS_MAGIC_STRING__BUFFER);
  jerry_value_t jlength =
      iotjs_jval_get_property(jbuf, IOTJS_MAGIC_STRING_LENGTH);
  size_t length = iotjs_jval_as_number(jlength);
  IOTJS_ASSERT(length == bufferwrap->length);
  jerry_release_value(jbuf);
  jerry_release_value(jlength);
#endif
  return bufferwrap->length;
}


static size_t bound_range(size_t index, size_t low, size_t upper) {
  if (index == SIZE_MAX) {
    return low;
  }
  if (index > upper) {
    return upper;
  }
  return index;
}


static int8_t hex2bin(char c) {
  if (c >= '0' && c <= '9')
    return (int8_t)(c - '0');
  if (c >= 'A' && c <= 'F')
    return (int8_t)(10 + (c - 'A'));
  if (c >= 'a' && c <= 'f')
    return (int8_t)(10 + (c - 'a'));

  return (int8_t)(-1);
}


static size_t hex_decode(char* buf, size_t len, const char* src,
                         const size_t srcLen) {
  size_t i;

  for (i = 0; i < len && i * 2 + 1 < srcLen; ++i) {
    int8_t a = hex2bin(src[i * 2 + 0]);
    int8_t b = hex2bin(src[i * 2 + 1]);
    if (a == -1 || b == -1)
      return i;
    buf[i] = (a << 4) | b;
  }

  return i;
}


static size_t iotjs_convert_double_to_sizet(double value) {
  size_t new_value;

  if (value < 0 || isnan(value) || isinf(value)) {
    new_value = SIZE_MAX;
  } else {
    new_value = (size_t)value;
  }

  return new_value;
}


int iotjs_bufferwrap_compare(const iotjs_bufferwrap_t* bufferwrap,
                             const iotjs_bufferwrap_t* other) {
  const char* other_buffer = other->buffer;
  size_t other_length = other->length;

  size_t i = 0;
  size_t j = 0;
  while (i < bufferwrap->length && j < other_length) {
    if (bufferwrap->buffer[i] < other_buffer[j]) {
      return -1;
    } else if (bufferwrap->buffer[i] > other_buffer[j]) {
      return 1;
    }
    ++i;
    ++j;
  }
  if (j < other_length) {
    return -1;
  } else if (i < bufferwrap->length) {
    return 1;
  }
  return 0;
}

size_t iotjs_bufferwrap_copy_internal(iotjs_bufferwrap_t* bufferwrap,
                                      const char* src, size_t src_from,
                                      size_t src_to, size_t dst_from) {
  size_t copied = 0;
  size_t dst_length = bufferwrap->length;
  for (size_t i = src_from, j = dst_from; i < src_to && j < dst_length;
       ++i, ++j) {
    *(bufferwrap->buffer + j) = *(src + i);
    ++copied;
  }
  return copied;
}


size_t iotjs_bufferwrap_copy(iotjs_bufferwrap_t* bufferwrap, const char* src,
                             size_t len) {
  return iotjs_bufferwrap_copy_internal(bufferwrap, src, 0, len, 0);
}


jerry_value_t iotjs_bufferwrap_create_buffer(size_t len) {
  jerry_value_t jglobal = jerry_get_global_object();

  jerry_value_t jbuffer =
      iotjs_jval_get_property(jglobal, IOTJS_MAGIC_STRING_BUFFER);
  jerry_release_value(jglobal);
  IOTJS_ASSERT(jerry_value_is_function(jbuffer));

  iotjs_jargs_t jargs = iotjs_jargs_create(1);
  iotjs_jargs_append_number(&jargs, len);

  jerry_value_t jres =
      iotjs_jhelper_call_ok(jbuffer, jerry_create_undefined(), &jargs);
  IOTJS_ASSERT(jerry_value_is_object(jres));

  iotjs_jargs_destroy(&jargs);
  jerry_release_value(jbuffer);

  return jres;
}


JS_FUNCTION(Buffer) {
  DJS_CHECK_THIS();
  DJS_CHECK_ARGS(2, object, number);

  const jerry_value_t jbuiltin = JS_GET_THIS();
  const jerry_value_t jbuffer = JS_GET_ARG(0, object);
  size_t length = JS_GET_ARG(1, number);

  iotjs_jval_set_property_jval(jbuiltin, IOTJS_MAGIC_STRING__BUFFER, jbuffer);

  iotjs_bufferwrap_create(jbuiltin, length);
  return jerry_create_undefined();
}


JS_FUNCTION(Compare) {
  JS_DECLARE_THIS_PTR(bufferwrap, src_buffer_wrap);
  JS_DECLARE_OBJECT_PTR(0, bufferwrap, dst_buffer_wrap);

  int compare = iotjs_bufferwrap_compare(src_buffer_wrap, dst_buffer_wrap);
  return jerry_create_number(compare);
}


JS_FUNCTION(Copy) {
  JS_DECLARE_THIS_PTR(bufferwrap, src_buffer_wrap);
  DJS_CHECK_ARGS(4, object, number, number, number);

  const jerry_value_t jdst_buffer = JS_GET_ARG(0, object);
  iotjs_bufferwrap_t* dst_buffer_wrap =
      iotjs_bufferwrap_from_jbuffer(jdst_buffer);

  size_t dst_length = iotjs_bufferwrap_length(dst_buffer_wrap);
  size_t src_length = iotjs_bufferwrap_length(src_buffer_wrap);

  size_t dst_start = iotjs_convert_double_to_sizet(JS_GET_ARG(1, number));
  dst_start = bound_range(dst_start, 0, dst_length);

  size_t src_start = iotjs_convert_double_to_sizet(JS_GET_ARG(2, number));
  src_start = bound_range(src_start, 0, src_length);

  size_t src_end = iotjs_convert_double_to_sizet(JS_GET_ARG(3, number));
  src_end = bound_range(src_end, 0, src_length);

  if (src_end < src_start) {
    src_end = src_start;
  }

  const char* src_data = src_buffer_wrap->buffer;
  size_t copied = iotjs_bufferwrap_copy_internal(dst_buffer_wrap, src_data,
                                                 src_start, src_end, dst_start);

  return jerry_create_number(copied);
}


JS_FUNCTION(Write) {
  JS_DECLARE_THIS_PTR(bufferwrap, buffer_wrap);
  DJS_CHECK_ARGS(3, string, number, number);

  iotjs_string_t src = JS_GET_ARG(0, string);

  size_t buffer_length = iotjs_bufferwrap_length(buffer_wrap);
  size_t offset = iotjs_convert_double_to_sizet(JS_GET_ARG(1, number));
  offset = bound_range(offset, 0, buffer_length);

  size_t length = iotjs_convert_double_to_sizet(JS_GET_ARG(2, number));
  length = bound_range(length, 0, buffer_length - offset);
  length = bound_range(length, 0, iotjs_string_size(&src));

  const char* src_data = iotjs_string_data(&src);
  size_t copied =
      iotjs_bufferwrap_copy_internal(buffer_wrap, src_data, 0, length, offset);

  iotjs_string_destroy(&src);

  return jerry_create_number(copied);
}


JS_FUNCTION(WriteUInt8) {
  JS_DECLARE_THIS_PTR(bufferwrap, buffer_wrap);
  DJS_CHECK_ARGS(2, number, number);

  const char src[] = { (char)JS_GET_ARG(0, number) };
  size_t length = 1;

  size_t buffer_length = iotjs_bufferwrap_length(buffer_wrap);
  size_t offset = iotjs_convert_double_to_sizet(JS_GET_ARG(1, number));
  offset = bound_range(offset, 0, buffer_length);
  length = bound_range(length, 0, buffer_length - offset);
  length = bound_range(length, 0, 1);

  size_t copied =
      iotjs_bufferwrap_copy_internal(buffer_wrap, src, 0, length, offset);

  return jerry_create_number(copied);
}


JS_FUNCTION(HexWrite) {
  JS_DECLARE_THIS_PTR(bufferwrap, buffer_wrap);
  DJS_CHECK_ARGS(3, string, number, number);

  iotjs_string_t src = JS_GET_ARG(0, string);

  size_t buffer_length = iotjs_bufferwrap_length(buffer_wrap);
  size_t offset = iotjs_convert_double_to_sizet(JS_GET_ARG(1, number));
  offset = bound_range(offset, 0, buffer_length);

  size_t length = iotjs_convert_double_to_sizet(JS_GET_ARG(2, number));
  length = bound_range(length, 0, buffer_length - offset);

  const char* src_data = iotjs_string_data(&src);
  unsigned src_length = iotjs_string_size(&src);
  char* src_buf = iotjs_buffer_allocate(length);

  size_t nbytes = hex_decode(src_buf, length, src_data, src_length);

  size_t copied =
      iotjs_bufferwrap_copy_internal(buffer_wrap, src_buf, 0, nbytes, offset);

  iotjs_buffer_release(src_buf);
  iotjs_string_destroy(&src);

  return jerry_create_number(copied);
}


JS_FUNCTION(ReadUInt8) {
  JS_DECLARE_THIS_PTR(bufferwrap, buffer_wrap);
  DJS_CHECK_ARGS(1, number);

  size_t buffer_length = iotjs_bufferwrap_length(buffer_wrap);
  size_t offset = iotjs_convert_double_to_sizet(JS_GET_ARG(0, number));
  offset = bound_range(offset, 0, buffer_length - 1);

  char* buffer = buffer_wrap->buffer;
  uint8_t result = 0;

  if (buffer != NULL) {
    result = (uint8_t)buffer[offset];
  }

  return jerry_create_number(result);
}


JS_FUNCTION(Slice) {
  JS_DECLARE_THIS_PTR(bufferwrap, buffer_wrap);
  DJS_CHECK_ARGS(2, number, number);

  int64_t start = JS_GET_ARG(0, number);
  int64_t end = JS_GET_ARG(1, number);
  size_t start_idx, end_idx;

  if (start < 0) {
    size_t len = iotjs_bufferwrap_length(buffer_wrap);
    if ((size_t)(-start) > len) {
      start_idx = SIZE_MAX;
    } else {
      start_idx = (size_t)start + len;
    }
  } else {
    start_idx = (size_t)start;
  }
  start_idx = bound_range(start_idx, 0, iotjs_bufferwrap_length(buffer_wrap));

  if (end < 0) {
    size_t len = iotjs_bufferwrap_length(buffer_wrap);
    if ((size_t)(-end) > len) {
      end_idx = SIZE_MAX;
    } else {
      end_idx = (size_t)end + len;
    }
  } else {
    end_idx = (size_t)end;
  }
  end_idx = bound_range(end_idx, 0, iotjs_bufferwrap_length(buffer_wrap));

  if (end_idx < start_idx) {
    end_idx = start_idx;
  }

  size_t length = (size_t)(end_idx - start_idx);

  jerry_value_t jnew_buffer = iotjs_bufferwrap_create_buffer(length);
  iotjs_bufferwrap_t* new_buffer_wrap =
      iotjs_bufferwrap_from_jbuffer(jnew_buffer);
  iotjs_bufferwrap_copy_internal(new_buffer_wrap, buffer_wrap->buffer,
                                 start_idx, end_idx, 0);

  return jnew_buffer;
}


JS_FUNCTION(ToString) {
  JS_DECLARE_THIS_PTR(bufferwrap, buffer_wrap);
  DJS_CHECK_ARGS(2, number, number);

  size_t start = iotjs_convert_double_to_sizet(JS_GET_ARG(0, number));
  start = bound_range(start, 0, iotjs_bufferwrap_length(buffer_wrap));

  size_t end = iotjs_convert_double_to_sizet(JS_GET_ARG(1, number));
  end = bound_range(end, 0, iotjs_bufferwrap_length(buffer_wrap));

  if (end < start) {
    end = start;
  }

  size_t length = end - start;

  const char* data = buffer_wrap->buffer + start;
  length = strnlen(data, length);

  if (!jerry_is_valid_utf8_string((const jerry_char_t*)data, length)) {
    return JS_CREATE_ERROR(TYPE, "Invalid UTF-8 string");
  }

  return jerry_create_string_sz_from_utf8((const jerry_char_t*)data, length);
}


JS_FUNCTION(ToHexString) {
  JS_DECLARE_THIS_PTR(bufferwrap, buffer_wrap);

  size_t length = iotjs_bufferwrap_length(buffer_wrap);
  const char* data = buffer_wrap->buffer;
  JS_CHECK(data != NULL);

  char* buffer = iotjs_buffer_allocate(length * 2);
  iotjs_string_t str = iotjs_string_create_with_buffer(buffer, length * 2);

  for (size_t i = 0; i < length; i++) {
    memcpy(buffer, &"0123456789abcdef"[data[i] >> 4 & 0xF], 1);
    buffer++;
    memcpy(buffer, &"0123456789abcdef"[data[i] >> 0 & 0xF], 1);
    buffer++;
  }

  jerry_value_t ret_value = iotjs_jval_create_string(&str);
  iotjs_string_destroy(&str);

  return ret_value;
}


JS_FUNCTION(ByteLength) {
  DJS_CHECK_THIS();
  DJS_CHECK_ARGS(1, string);

  iotjs_string_t str = JS_GET_ARG(0, string);
  jerry_value_t size = iotjs_jval_get_string_size(&str);

  iotjs_string_destroy(&str);
  return size;
}


jerry_value_t InitBuffer() {
  jerry_value_t buffer = jerry_create_external_function(Buffer);

  jerry_value_t prototype = jerry_create_object();
  iotjs_jval_set_property_jval(buffer, IOTJS_MAGIC_STRING_PROTOTYPE, prototype);
  iotjs_jval_set_method(buffer, IOTJS_MAGIC_STRING_BYTELENGTH, ByteLength);

  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_COMPARE, Compare);
  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_COPY, Copy);
  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_WRITE, Write);
  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_HEXWRITE, HexWrite);
  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_WRITEUINT8, WriteUInt8);
  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_READUINT8, ReadUInt8);
  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_SLICE, Slice);
  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_TOSTRING, ToString);
  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_TOHEXSTRING, ToHexString);

  jerry_release_value(prototype);

  return buffer;
}
