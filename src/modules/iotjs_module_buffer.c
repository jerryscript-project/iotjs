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

typedef enum {
  BUFFER_HEX_ENC = 0,
  BUFFER_BASE64_ENC = 1,
} buffer_encoding_type_t;


IOTJS_DEFINE_NATIVE_HANDLE_INFO_THIS_MODULE(bufferwrap);


iotjs_bufferwrap_t* iotjs_bufferwrap_create(const jerry_value_t jobject,
                                            size_t length) {
  iotjs_bufferwrap_t* bufferwrap = (iotjs_bufferwrap_t*)iotjs_buffer_allocate(
      sizeof(iotjs_bufferwrap_t) + length);

  bufferwrap->jobject = jobject;
  jerry_set_object_native_pointer(jobject, bufferwrap,
                                  &this_module_native_info);

  bufferwrap->length = length;

  IOTJS_ASSERT(
      bufferwrap ==
      (iotjs_bufferwrap_t*)(iotjs_jval_get_object_native_handle(jobject)));

  return bufferwrap;
}


static void iotjs_bufferwrap_destroy(iotjs_bufferwrap_t* bufferwrap) {
  IOTJS_RELEASE(bufferwrap);
}


iotjs_bufferwrap_t* iotjs_bufferwrap_from_jbuffer(const jerry_value_t jbuffer) {
  IOTJS_ASSERT(jerry_value_is_object(jbuffer));
  iotjs_bufferwrap_t* buffer =
      (iotjs_bufferwrap_t*)iotjs_jval_get_object_native_handle(jbuffer);
  IOTJS_ASSERT(buffer != NULL);
  return buffer;
}


size_t iotjs_bufferwrap_length(iotjs_bufferwrap_t* bufferwrap) {
  if (bufferwrap == NULL) {
    IOTJS_ASSERT(0);
    return 0;
  }
#ifndef NDEBUG
  jerry_value_t jlength =
      iotjs_jval_get_property(bufferwrap->jobject, IOTJS_MAGIC_STRING_LENGTH);
  size_t length = iotjs_jval_as_number(jlength);
  IOTJS_ASSERT(length == bufferwrap->length);
  jerry_release_value(jlength);
#endif
  return bufferwrap->length;
}


iotjs_bufferwrap_t* iotjs_jbuffer_get_bufferwrap_ptr(
    const jerry_value_t jbuffer) {
  if (!jerry_value_is_object(jbuffer)) {
    return NULL;
  }

  void* native_p;
  const jerry_object_native_info_t* type_p;
  bool has_native_pointer =
      jerry_get_object_native_pointer(jbuffer, &native_p, &type_p);

  if (has_native_pointer && type_p == &this_module_native_info) {
    return (iotjs_bufferwrap_t*)native_p;
  }

  return NULL;
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


static int8_t hex_to_bin(char c) {
  if (c >= '0' && c <= '9') {
    return (int8_t)(c - '0');
  }
  if (c >= 'A' && c <= 'F') {
    return (int8_t)(10 + (c - 'A'));
  }
  if (c >= 'a' && c <= 'f') {
    return (int8_t)(10 + (c - 'a'));
  }

  return (int8_t)(-1);
}


static size_t hex_decode(char* buf, size_t len, const char* src,
                         const size_t srcLen) {
  const char* bufStart = buf;
  const char* bufEnd = buf + len;
  const char* srcEnd = src + srcLen;

  if ((srcLen & 0x1) != 0) {
    return 0;
  }

  while (src < srcEnd) {
    int8_t a = hex_to_bin(src[0]);
    int8_t b = hex_to_bin(src[1]);

    if (a == -1 || b == -1) {
      return 0;
    }

    if (buf < bufEnd) {
      *buf++ = (a << 4) | b;
    }

    src += 2;
  }

  return (size_t)((buf - bufStart) + 1);
}


static int32_t base64_to_bin(char c) {
  if (c >= 'A' && c <= 'Z') {
    return (int32_t)(c - 'A');
  }
  if (c >= 'a' && c <= 'z') {
    return (int32_t)(26 + (c - 'a'));
  }
  if (c >= '0' && c <= '9') {
    return (int32_t)(52 + (c - '0'));
  }
  if (c == '+') {
    return 62;
  }
  if (c == '/') {
    return 63;
  }

  return (int32_t)(-1);
}


static size_t base64_decode(char* buf, size_t len, const char* src,
                            const size_t srcLen) {
  if (srcLen == 0) {
    return 0 + 1;
  }

  if ((srcLen & 0x3) != 0) {
    return 0;
  }

  const char* bufStart = buf;
  const char* bufEnd = buf + len;
  const char* srcEnd = src + srcLen;

  if (srcEnd[-1] == '=') {
    srcEnd--;
    if (srcEnd[-1] == '=') {
      srcEnd--;
    }
  }

  int32_t current_bits = 0;
  int32_t shift = 8;

  while (src < srcEnd) {
    int32_t value = base64_to_bin(*src++);

    if (value == -1) {
      return 0;
    }

    current_bits = (current_bits << 6) | value;
    shift -= 2;

    if (shift == 6) {
      continue;
    }

    int32_t byte = (current_bits >> shift);
    current_bits &= (1 << shift) - 1;

    if (shift == 0) {
      shift = 8;
    }

    if (buf < bufEnd) {
      *buf++ = (char)byte;
    }
  }

  return (size_t)((buf - bufStart) + 1);
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

static size_t index_normalizer(int64_t index, size_t max_length) {
  size_t idx;
  if (index < 0) {
    if ((size_t)(-index) > max_length) {
      idx = SIZE_MAX;
    } else {
      idx = (size_t)index + max_length;
    }
  } else {
    idx = (size_t)index;
  }

  return bound_range(idx, 0, max_length);
}

jerry_value_t iotjs_bufferwrap_create_buffer(size_t len) {
  jerry_value_t jres_buffer = jerry_create_object();

  iotjs_bufferwrap_create(jres_buffer, len);

  iotjs_jval_set_property_number(jres_buffer, IOTJS_MAGIC_STRING_LENGTH, len);

  // Support for 'instanceof' operator
  jerry_value_t jglobal = jerry_get_global_object();
  jerry_value_t jbuffer =
      iotjs_jval_get_property(jglobal, IOTJS_MAGIC_STRING_BUFFER);
  jerry_release_value(jglobal);

  if (!jerry_value_is_error(jbuffer) && jerry_value_is_object(jbuffer)) {
    jerry_value_t jbuffer_proto =
        iotjs_jval_get_property(jbuffer, IOTJS_MAGIC_STRING_PROTOTYPE);

    if (!jerry_value_is_error(jbuffer_proto) &&
        jerry_value_is_object(jbuffer_proto)) {
      jerry_set_prototype(jres_buffer, jbuffer_proto);
    }

    jerry_release_value(jbuffer_proto);
  }
  jerry_release_value(jbuffer);

  return jres_buffer;
}


JS_FUNCTION(Buffer) {
  DJS_CHECK_ARGS(2, object, number);

  const jerry_value_t jobject = JS_GET_ARG(0, object);
  size_t length = JS_GET_ARG(1, number);

  iotjs_bufferwrap_create(jobject, length);
  return jerry_create_undefined();
}

JS_FUNCTION(Compare) {
  JS_DECLARE_OBJECT_PTR(0, bufferwrap, src_buffer_wrap);
  JS_DECLARE_OBJECT_PTR(1, bufferwrap, dst_buffer_wrap);

  int compare = iotjs_bufferwrap_compare(src_buffer_wrap, dst_buffer_wrap);
  return jerry_create_number(compare);
}


JS_FUNCTION(Copy) {
  DJS_CHECK_ARGS(5, object, object, number, number, number);
  JS_DECLARE_OBJECT_PTR(0, bufferwrap, src_buffer_wrap);

  const jerry_value_t jdst_buffer = JS_GET_ARG(1, object);
  iotjs_bufferwrap_t* dst_buffer_wrap =
      iotjs_bufferwrap_from_jbuffer(jdst_buffer);

  size_t dst_length = iotjs_bufferwrap_length(dst_buffer_wrap);
  size_t src_length = iotjs_bufferwrap_length(src_buffer_wrap);

  size_t dst_start = iotjs_convert_double_to_sizet(JS_GET_ARG(2, number));
  dst_start = bound_range(dst_start, 0, dst_length);

  size_t src_start = iotjs_convert_double_to_sizet(JS_GET_ARG(3, number));
  src_start = bound_range(src_start, 0, src_length);

  size_t src_end = iotjs_convert_double_to_sizet(JS_GET_ARG(4, number));
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
  DJS_CHECK_ARGS(4, object, string, number, number);
  JS_DECLARE_OBJECT_PTR(0, bufferwrap, buffer_wrap);

  iotjs_string_t src = JS_GET_ARG(1, string);

  size_t buffer_length = iotjs_bufferwrap_length(buffer_wrap);
  size_t offset = iotjs_convert_double_to_sizet(JS_GET_ARG(2, number));
  offset = bound_range(offset, 0, buffer_length);

  size_t length = iotjs_convert_double_to_sizet(JS_GET_ARG(3, number));
  length = bound_range(length, 0, buffer_length - offset);
  length = bound_range(length, 0, iotjs_string_size(&src));

  const char* src_data = iotjs_string_data(&src);
  size_t copied =
      iotjs_bufferwrap_copy_internal(buffer_wrap, src_data, 0, length, offset);

  iotjs_string_destroy(&src);

  return jerry_create_number(copied);
}


JS_FUNCTION(WriteUInt8) {
  DJS_CHECK_ARGS(3, object, number, number);
  JS_DECLARE_OBJECT_PTR(0, bufferwrap, buffer_wrap);

  const char src[] = { (char)JS_GET_ARG(1, number) };
  size_t length = 1;

  size_t buffer_length = iotjs_bufferwrap_length(buffer_wrap);
  size_t offset = iotjs_convert_double_to_sizet(JS_GET_ARG(2, number));
  offset = bound_range(offset, 0, buffer_length);
  length = bound_range(length, 0, buffer_length - offset);
  length = bound_range(length, 0, 1);

  size_t copied =
      iotjs_bufferwrap_copy_internal(buffer_wrap, src, 0, length, offset);

  return jerry_create_number(copied);
}


JS_FUNCTION(WriteDecode) {
  DJS_CHECK_ARGS(5, object, number, string, number, number);
  JS_DECLARE_OBJECT_PTR(0, bufferwrap, buffer_wrap);

  double type = JS_GET_ARG(1, number);
  iotjs_string_t src = JS_GET_ARG(2, string);

  size_t buffer_length = iotjs_bufferwrap_length(buffer_wrap);
  size_t offset = iotjs_convert_double_to_sizet(JS_GET_ARG(3, number));
  offset = bound_range(offset, 0, buffer_length);

  size_t length = iotjs_convert_double_to_sizet(JS_GET_ARG(4, number));
  length = bound_range(length, 0, buffer_length - offset);

  const char* src_data = iotjs_string_data(&src);
  unsigned src_length = iotjs_string_size(&src);

  size_t nbytes;
  char* dst_data = buffer_wrap->buffer + offset;
  const char* error_msg;

  if (type == BUFFER_HEX_ENC) {
    nbytes = hex_decode(dst_data, length, src_data, src_length);
    error_msg = "Invalid hex string";
  } else {
    nbytes = base64_decode(dst_data, length, src_data, src_length);
    error_msg = "Invalid base64 string";
  }

  iotjs_string_destroy(&src);

  if (nbytes == 0)
    return jerry_create_error(JERRY_ERROR_TYPE, (const jerry_char_t*)error_msg);

  return jerry_create_number(nbytes - 1);
}


JS_FUNCTION(ReadUInt8) {
  DJS_CHECK_ARGS(2, object, number);
  JS_DECLARE_OBJECT_PTR(0, bufferwrap, buffer_wrap);

  size_t buffer_length = iotjs_bufferwrap_length(buffer_wrap);

  if (buffer_length == 0) {
    return jerry_create_number(0);
  }

  size_t offset = iotjs_convert_double_to_sizet(JS_GET_ARG(1, number));
  offset = bound_range(offset, 0, buffer_length - 1);

  char* buffer = buffer_wrap->buffer;
  return jerry_create_number((uint8_t)buffer[offset]);
}


JS_FUNCTION(Slice) {
  DJS_CHECK_ARGS(3, object, number, number);
  JS_DECLARE_OBJECT_PTR(0, bufferwrap, buffer_wrap);

  int64_t start = JS_GET_ARG(1, number);
  int64_t end = JS_GET_ARG(2, number);
  size_t len = iotjs_bufferwrap_length(buffer_wrap);
  size_t start_idx = index_normalizer(start, len);
  size_t end_idx = index_normalizer(end, len);

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


static char to_hex_char(uint8_t digit) {
  return (char)((digit < 10) ? (digit + '0') : (digit + 'a' - 10));
}


static jerry_value_t to_hex_string(const uint8_t* data, size_t length) {
  if (length == 0) {
    return jerry_create_string_sz(NULL, 0);
  }

  const uint8_t* end = data + length;

  size_t buffer_length = length * 2;
  char* buffer = iotjs_buffer_allocate(buffer_length);
  const jerry_char_t* str = (const jerry_char_t*)buffer;

  while (data < end) {
    *buffer++ = to_hex_char(*data >> 4);
    *buffer++ = to_hex_char(*data & 0xf);
    data++;
  }

  jerry_value_t ret_value = jerry_create_string_sz(str, buffer_length);
  IOTJS_RELEASE(str);

  return ret_value;
}

static char to_base64_char(uint8_t digit) {
  if (digit <= 25) {
    return (char)digit + 'A';
  }
  if (digit <= 51) {
    return (char)digit + 'a' - 26;
  }
  if (digit <= 61) {
    return (char)digit + '0' - 52;
  }

  return (digit == 62) ? '+' : '/';
}

static jerry_value_t to_base64_string(const uint8_t* data, size_t length) {
  if (length == 0) {
    return jerry_create_string_sz(NULL, 0);
  }

  const uint8_t* end = data + length;

  size_t buffer_length = ((length + 2) / 3) * 4;
  char* buffer = iotjs_buffer_allocate(buffer_length);
  const jerry_char_t* str = (const jerry_char_t*)buffer;

  uint32_t current_bits = 0;
  int32_t shift = 2;

  while (data < end) {
    current_bits = (current_bits << 8) | *data++;

    *buffer++ = to_base64_char(current_bits >> shift);
    current_bits &= (uint32_t)((1 << shift) - 1);

    shift += 2;

    if (shift == 8) {
      *buffer++ = to_base64_char(current_bits);
      current_bits = 0;
      shift = 2;
    }
  }

  char* buffer_end = (char*)str + buffer_length;
  if (buffer < buffer_end) {
    buffer[0] = to_base64_char(current_bits << (8 - shift));
    buffer[1] = '=';

    if (buffer + 2 < buffer_end)
      buffer[2] = '=';
  }

  jerry_value_t ret_value = jerry_create_string_sz(str, buffer_length);
  IOTJS_RELEASE(str);

  return ret_value;
}


JS_FUNCTION(ToString) {
  DJS_CHECK_ARGS(4, object, number, number, number);
  JS_DECLARE_OBJECT_PTR(0, bufferwrap, buffer_wrap);

  double type = JS_GET_ARG(1, number);

  size_t start = iotjs_convert_double_to_sizet(JS_GET_ARG(2, number));
  start = bound_range(start, 0, iotjs_bufferwrap_length(buffer_wrap));

  size_t end = iotjs_convert_double_to_sizet(JS_GET_ARG(3, number));
  end = bound_range(end, 0, iotjs_bufferwrap_length(buffer_wrap));

  if (end < start) {
    end = start;
  }

  if (start > buffer_wrap->length) {
    start = buffer_wrap->length;
  }

  if (end > buffer_wrap->length) {
    end = buffer_wrap->length;
  }

  size_t length = end - start;

  const char* data = buffer_wrap->buffer + start;

  if (type == BUFFER_HEX_ENC) {
    return to_hex_string((const uint8_t*)data, length);
  }

  if (type == BUFFER_BASE64_ENC) {
    return to_base64_string((const uint8_t*)data, length);
  }

  /* Stops at first zero. */
  length = strnlen(data, length);

  if (!jerry_is_valid_utf8_string((const jerry_char_t*)data, length)) {
    return JS_CREATE_ERROR(TYPE, "Invalid UTF-8 string");
  }

  return jerry_create_string_sz_from_utf8((const jerry_char_t*)data, length);
}


JS_FUNCTION(ByteLength) {
  DJS_CHECK_ARGS(1, string);

  jerry_size_t size = jerry_get_string_size(jargv[0]);
  return jerry_create_number(size);
}


jerry_value_t InitBuffer() {
  jerry_value_t buffer = jerry_create_external_function(Buffer);
  iotjs_jval_set_method(buffer, IOTJS_MAGIC_STRING_BYTELENGTH, ByteLength);
  iotjs_jval_set_method(buffer, IOTJS_MAGIC_STRING_COMPARE, Compare);
  iotjs_jval_set_method(buffer, IOTJS_MAGIC_STRING_COPY, Copy);
  iotjs_jval_set_method(buffer, IOTJS_MAGIC_STRING_WRITE, Write);
  iotjs_jval_set_method(buffer, IOTJS_MAGIC_STRING_WRITEDECODE, WriteDecode);
  iotjs_jval_set_method(buffer, IOTJS_MAGIC_STRING_WRITEUINT8, WriteUInt8);
  iotjs_jval_set_method(buffer, IOTJS_MAGIC_STRING_READUINT8, ReadUInt8);
  iotjs_jval_set_method(buffer, IOTJS_MAGIC_STRING_SLICE, Slice);
  iotjs_jval_set_method(buffer, IOTJS_MAGIC_STRING_TOSTRING, ToString);

  return buffer;
}
