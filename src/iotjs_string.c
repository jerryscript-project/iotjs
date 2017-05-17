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
#include "iotjs_string.h"
#include "iotjs_util.h"

#include <string.h>


iotjs_string_t iotjs_string_create() {
  iotjs_string_t str;
  IOTJS_VALIDATED_STRUCT_CONSTRUCTOR(iotjs_string_t, &str);

  _this->size = 0;
  _this->data = NULL;

  return str;
}


iotjs_string_t iotjs_string_create_with_size(const char* data, size_t size) {
  iotjs_string_t str;
  IOTJS_VALIDATED_STRUCT_CONSTRUCTOR(iotjs_string_t, &str);

  _this->size = size;

  if (size > 0) {
    IOTJS_ASSERT(data != NULL);
    _this->data = iotjs_buffer_allocate(size);
    memcpy(_this->data, data, size);
  } else {
    _this->data = NULL;
  }

  return str;
}


iotjs_string_t iotjs_string_create_with_buffer(char* buffer, size_t size) {
  iotjs_string_t str;
  IOTJS_VALIDATED_STRUCT_CONSTRUCTOR(iotjs_string_t, &str);

  _this->size = size;

  if (size > 0) {
    IOTJS_ASSERT(buffer != NULL);
    _this->data = buffer;
  } else {
    _this->data = NULL;
  }

  return str;
}


void iotjs_string_destroy(iotjs_string_t* str) {
  IOTJS_VALIDATED_STRUCT_DESTRUCTOR(iotjs_string_t, str);

  if (_this->data != NULL) {
    iotjs_buffer_release(_this->data);
    _this->size = 0;
  }
}


bool iotjs_string_is_empty(const iotjs_string_t* str) {
  const IOTJS_VALIDATED_STRUCT_METHOD(iotjs_string_t, str);

  return _this->size == 0;
}


void iotjs_string_make_empty(iotjs_string_t* str) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_string_t, str);

  if (_this->data != NULL) {
    iotjs_buffer_release(_this->data);
    _this->size = 0;
    _this->data = NULL;
  }
}


void iotjs_string_append(iotjs_string_t* str, const char* data, size_t size) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_string_t, str);

  IOTJS_ASSERT(data != NULL);

  if (size == 0) {
    return;
  }

  if (_this->data != NULL) {
    _this->data = iotjs_buffer_reallocate(_this->data, _this->size + size);
  } else {
    IOTJS_ASSERT(_this->size == 0);
    _this->data = iotjs_buffer_allocate(size);
  }

  memcpy(_this->data + _this->size, data, size);
  _this->size += size;
}


const char* iotjs_string_data(const iotjs_string_t* str) {
  const IOTJS_VALIDATED_STRUCT_METHOD(iotjs_string_t, str);
  if (_this->data == NULL) {
    return "";
  }

  return _this->data;
}


unsigned iotjs_string_size(const iotjs_string_t* str) {
  const IOTJS_VALIDATED_STRUCT_METHOD(iotjs_string_t, str);
  return _this->size;
}
