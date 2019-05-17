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
      jerry_get_object_native_pointer(jobject, NULL, &this_module_native_info));
  return bufferwrap;
}


static void iotjs_bufferwrap_destroy(iotjs_bufferwrap_t* bufferwrap) {
  if (bufferwrap->external_info && bufferwrap->external_info->free_hint) {
    ((void (*)(void*))bufferwrap->external_info->free_hint)(
        bufferwrap->external_info->free_info);
  }

  IOTJS_RELEASE(bufferwrap->external_info);
  IOTJS_RELEASE(bufferwrap);
}


void iotjs_bufferwrap_set_external_callback(iotjs_bufferwrap_t* bufferwrap,
                                            void* free_hint, void* free_info) {
  bufferwrap->external_info = IOTJS_ALLOC(iotjs_bufferwrap_external_info_t);
  bufferwrap->external_info->free_hint = free_hint;
  bufferwrap->external_info->free_info = free_info;
}


iotjs_bufferwrap_t* iotjs_bufferwrap_from_jbuffer(const jerry_value_t jbuffer) {
  IOTJS_ASSERT(jerry_value_is_object(jbuffer));

  void* buffer = NULL;
  bool res = jerry_get_object_native_pointer(jbuffer, &buffer,
                                             &this_module_native_info);
  IOTJS_ASSERT(res && buffer != NULL);
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

  void* buffer = NULL;
  if (jerry_get_object_native_pointer(jbuffer, &buffer,
                                      &this_module_native_info)) {
    return (iotjs_bufferwrap_t*)buffer;
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
                         const size_t src_len) {
  const char* buf_start = buf;
  const char* buf_end = buf + len;
  const char* src_end = src + src_len;

  if ((src_len & 0x1) != 0) {
    return 0;
  }

  while (src < src_end) {
    int8_t a = hex_to_bin(src[0]);
    int8_t b = hex_to_bin(src[1]);

    if (a == -1 || b == -1) {
      return 0;
    }

    if (buf < buf_end) {
      *buf++ = (a << 4) | b;
    }

    src += 2;
  }

  return (size_t)((buf - buf_start) + 1);
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


static size_t base64_decode(char* dst, size_t len, const char* src,
                            const size_t srcLen) {
  if (srcLen == 0) {
    return 1;
  }

  char* decoded_base64 = NULL;
  size_t decoded_len = iotjs_base64_decode(&decoded_base64, src, srcLen);
  size_t ret_val = 0;
  if (decoded_len) {
    char* buf = dst;
    char* bufEnd = len > decoded_len ? dst + decoded_len : dst + len;

    char* pos = decoded_base64;
    while (buf < bufEnd) {
      *buf++ = *pos++;
    }
    ret_val = (size_t)(buf - dst) + 1;
  }

  IOTJS_RELEASE(decoded_base64);
  return ret_val;
}


size_t iotjs_base64_decode(char** out_buff, const char* src,
                           const size_t srcLen) {
  if ((srcLen & 0x3) != 0 || srcLen == 0) {
    return 0;
  }

  size_t len = (3 * (srcLen / 4));

  const char* src_end = src + srcLen;

  if (src_end[-1] == '=') {
    src_end--;
    len--;
    if (src_end[-1] == '=') {
      src_end--;
      len--;
    }
  }

  if (*out_buff == NULL) {
    *out_buff = IOTJS_CALLOC(len, char);
  }

  char* buf = *out_buff;

  const char* bufStart = buf;
  const char* bufEnd = buf + len;

  int32_t current_bits = 0;
  int32_t shift = 8;

  while (src < src_end) {
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

    if (buf <= bufEnd) {
      *buf++ = byte;
    }
  }

  return (size_t)((buf - bufStart));
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
  jerry_value_t native_buffer = iotjs_module_get("buffer");
  jerry_value_t jbuffer =
      iotjs_jval_get_property(native_buffer, IOTJS_MAGIC_STRING_BUFFER);

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


JS_FUNCTION(buffer_constructor) {
  DJS_CHECK_ARGS(2, object, number);

  const jerry_value_t jobject = JS_GET_ARG(0, object);
  size_t length = JS_GET_ARG(1, number);

  iotjs_bufferwrap_create(jobject, length);
  return jerry_create_undefined();
}

JS_FUNCTION(buffer_compare) {
  JS_DECLARE_OBJECT_PTR(0, bufferwrap, src_buffer_wrap);
  JS_DECLARE_OBJECT_PTR(1, bufferwrap, dst_buffer_wrap);

  int compare = iotjs_bufferwrap_compare(src_buffer_wrap, dst_buffer_wrap);
  return jerry_create_number(compare);
}


JS_FUNCTION(buffer_copy) {
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


JS_FUNCTION(buffer_write) {
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


JS_FUNCTION(buffer_write_uint8) {
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


JS_FUNCTION(buffer_write_decode) {
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


JS_FUNCTION(buffer_read_uint8) {
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


JS_FUNCTION(buffer_slice) {
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


static jerry_value_t to_base64_string(const uint8_t* data, size_t length) {
  unsigned char* buffer = NULL;
  size_t buffer_length = iotjs_base64_encode(&buffer, data, length);
  jerry_value_t ret_value = jerry_create_string_sz(buffer, buffer_length);
  IOTJS_RELEASE(buffer);

  return ret_value;
}

static const unsigned char base64_enc_map[65] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";


size_t iotjs_base64_encode(unsigned char** out_buff, const unsigned char* data,
                           size_t buff_len) {
  size_t i, n;
  int C1, C2, C3;
  unsigned char* p;

  if (buff_len == 0) {
    return 0;
  }

  n = buff_len / 3 + (buff_len % 3 != 0);

  if (n > ((size_t)-2) / 4) {
    return 0;
  }

  if (*out_buff == NULL) {
    *out_buff = IOTJS_CALLOC(n * 4 + 1, unsigned char);
  }

  n = (buff_len / 3) * 3;

  for (i = 0, p = *out_buff; i < n; i += 3) {
    C1 = *data++;
    C2 = *data++;
    C3 = *data++;

    *p++ = base64_enc_map[(C1 >> 2) & 0x3F];
    *p++ = base64_enc_map[(((C1 & 3) << 4) + (C2 >> 4)) & 0x3F];
    *p++ = base64_enc_map[(((C2 & 15) << 2) + (C3 >> 6)) & 0x3F];
    *p++ = base64_enc_map[C3 & 0x3F];
  }

  if (i < buff_len) {
    C1 = *data++;
    C2 = ((i + 1) < buff_len) ? *data++ : 0;

    *p++ = base64_enc_map[(C1 >> 2) & 0x3F];
    *p++ = base64_enc_map[(((C1 & 3) << 4) + (C2 >> 4)) & 0x3F];

    if ((i + 1) < buff_len) {
      *p++ = base64_enc_map[((C2 & 15) << 2) & 0x3F];
    } else {
      *p++ = '=';
    }

    *p++ = '=';
  }

  size_t ret_len = (size_t)(p - *out_buff);
  *p = 0;

  return ret_len;
}


JS_FUNCTION(buffer_to_string) {
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


JS_FUNCTION(buffer_byte_length) {
  DJS_CHECK_ARGS(1, string);

  jerry_size_t size = jerry_get_string_size(jargv[0]);
  return jerry_create_number(size);
}


JS_FUNCTION(buffer_from_array_buffer) {
  if (jargc < 1 || !jerry_value_is_arraybuffer(jargv[0])) {
    return jerry_create_undefined();
  }
  jerry_length_t offset = 0;
  jerry_length_t length = jerry_get_arraybuffer_byte_length(jargv[0]);

  if (jargc >= 2) {
    jerry_value_t offset_num = jerry_value_to_number(jargv[1]);

    if (jerry_value_is_error(offset_num)) {
      return offset_num;
    }

    double offset_value = jerry_get_number_value(offset_num);
    if (isnan(offset_value)) {
      offset_value = 0;
    }
    jerry_release_value(offset_num);

    if (offset_value < 0 || offset_value > length) {
      return JS_CREATE_ERROR(RANGE, "'offset' is out of bounds");
    }
    offset = (jerry_length_t)offset_value;
  }

  length -= offset;

  if (jargc >= 3) {
    if (jerry_value_is_error(jargv[2])) {
      return length;
    }

    if (jerry_value_is_number(jargv[2])) {
      double length_value = (double)length;
      length_value = jerry_get_number_value(jargv[2]);

      if (isnan(length_value) || length_value < 0) {
        length = 0;
      } else if (length_value < length) {
        length = (jerry_length_t)length_value;
      } else if (length_value > length) {
        return JS_CREATE_ERROR(RANGE, "'length' is out of bounds");
      }
    }
  }

  if (length < 1) {
    return iotjs_bufferwrap_create_buffer(0);
  }

  jerry_value_t jres_bufferwrap = iotjs_bufferwrap_create_buffer(length);
  iotjs_bufferwrap_t* jsres_buffer =
      iotjs_jbuffer_get_bufferwrap_ptr(jres_bufferwrap);
  jerry_arraybuffer_read(jargv[0], offset, (uint8_t*)jsres_buffer->buffer,
                         length);
  return jres_bufferwrap;
}


jerry_value_t iotjs_init_buffer(void) {
  jerry_value_t buffer = jerry_create_external_function(buffer_constructor);
  iotjs_jval_set_method(buffer, IOTJS_MAGIC_STRING_BYTELENGTH,
                        buffer_byte_length);
  iotjs_jval_set_method(buffer, IOTJS_MAGIC_STRING_COMPARE, buffer_compare);
  iotjs_jval_set_method(buffer, IOTJS_MAGIC_STRING_COPY, buffer_copy);
  iotjs_jval_set_method(buffer, IOTJS_MAGIC_STRING_WRITE, buffer_write);
  iotjs_jval_set_method(buffer, IOTJS_MAGIC_STRING_WRITEDECODE,
                        buffer_write_decode);
  iotjs_jval_set_method(buffer, IOTJS_MAGIC_STRING_WRITEUINT8,
                        buffer_write_uint8);
  iotjs_jval_set_method(buffer, IOTJS_MAGIC_STRING_READUINT8,
                        buffer_read_uint8);
  iotjs_jval_set_method(buffer, IOTJS_MAGIC_STRING_SLICE, buffer_slice);
  iotjs_jval_set_method(buffer, IOTJS_MAGIC_STRING_TOSTRING, buffer_to_string);
  iotjs_jval_set_method(buffer, IOTJS_MAGIC_STRING_FROM_ARRAYBUFFER,
                        buffer_from_array_buffer);

  return buffer;
}
