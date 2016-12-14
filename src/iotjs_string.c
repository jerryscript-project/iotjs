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


static char iotjs_empty_string[] = { '\0' };


iotjs_string_t iotjs_string_create(const char* data) {
  return iotjs_string_create_with_size(data, strlen(data));
}


iotjs_string_t iotjs_string_create_with_size(const char* data, unsigned size) {
  iotjs_string_t str;
  IOTJS_VALIDATED_STRUCT_CONSTRUCTOR(iotjs_string_t, &str);

  IOTJS_ASSERT(data != NULL);
  IOTJS_ASSERT(strnlen(data, size) == size); // forbidden due to ambiguity

  _this->size = _this->cap = size;

  if (size > 0) {
    _this->data = iotjs_buffer_allocate(size + 1);
    memcpy(_this->data, data, size);
    _this->data[size] = '\0';
  } else {
    _this->data = iotjs_empty_string;
  }

  return str;
}


iotjs_string_t iotjs_string_create_with_buffer(char* buffer, unsigned size) {
  iotjs_string_t str;
  IOTJS_VALIDATED_STRUCT_CONSTRUCTOR(iotjs_string_t, &str);

  IOTJS_ASSERT(buffer != NULL);

  _this->size = _this->cap = size;
  _this->data = buffer;

  return str;
}


void iotjs_string_destroy(iotjs_string_t* str) {
  IOTJS_VALIDATED_STRUCT_DESTRUCTOR(iotjs_string_t, str);

  if (_this->data != iotjs_empty_string) {
    iotjs_buffer_release(_this->data);
  }
}


void iotjs_string_reserve(iotjs_string_t* str, unsigned capacity) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_string_t, str);

  if (_this->cap >= capacity)
    return;

  if (_this->cap == 0) {
    IOTJS_ASSERT(_this->data == iotjs_empty_string);
    _this->data = iotjs_buffer_allocate(capacity + 1);
  } else {
    _this->data = iotjs_buffer_reallocate(_this->data, capacity + 1);
  }
  _this->cap = capacity;
}


bool iotjs_string_is_empty(const iotjs_string_t* str) {
  const IOTJS_VALIDATED_STRUCT_METHOD(iotjs_string_t, str);

  return _this->size == 0;
}


void iotjs_string_make_empty(iotjs_string_t* str) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_string_t, str);

  if (_this->data != iotjs_empty_string) {
    iotjs_buffer_release(_this->data);
    _this->cap = _this->size = 0;
    _this->data = iotjs_empty_string;
  }
}


void iotjs_string_append(iotjs_string_t* str, const char* data, int size) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_string_t, str);

  const int kCapacityIncreaseFactor = 2;

  IOTJS_ASSERT(_this->data != NULL);
  IOTJS_ASSERT(_this->cap >= _this->size);

  if (size < 0) {
    size = strlen(data);
  }
  if (size == 0) {
    return;
  }

  if (_this->cap == 0) {
    // No buffer was allocated.
    IOTJS_ASSERT(_this->size == 0);
    IOTJS_ASSERT(_this->data == iotjs_empty_string);
    _this->cap = _this->size = size;
    _this->data = iotjs_buffer_allocate(_this->cap + 1);
    strncpy(_this->data, data, _this->size);
  } else if (_this->cap >= _this->size + size) {
    // Have enough capacity to append data.
    strncpy(_this->data + _this->size, data, size);
    _this->size += size;
  } else {
    // Lack of capacity, calculate next capacity.
    while (_this->cap < _this->size + size) {
      _this->cap *= kCapacityIncreaseFactor;
    }
    // Reallocate buffer and copy data.
    _this->data = iotjs_buffer_reallocate(_this->data, _this->cap + 1);
    strncpy(_this->data + _this->size, data, size);
    _this->size += size;
  }
  _this->data[_this->size] = 0;
}


const char* iotjs_string_data(const iotjs_string_t* str) {
  const IOTJS_VALIDATED_STRUCT_METHOD(iotjs_string_t, str);
  IOTJS_ASSERT(_this->data != NULL);
  return _this->data;
}


unsigned iotjs_string_size(const iotjs_string_t* str) {
  const IOTJS_VALIDATED_STRUCT_METHOD(iotjs_string_t, str);
  return _this->size;
}
