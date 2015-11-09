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
#include "iotjs_util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


namespace iotjs {


String ReadFile(const char* path) {
  FILE* file = fopen(path, "rb");
  IOTJS_ASSERT(file != NULL);

  fseek(file, 0, SEEK_END);
  long len = ftell(file);
  IOTJS_ASSERT(len >= 0);
  fseek(file, 0, SEEK_SET);

  String contents(NULL, 0, len);

  char buff[128];
  size_t total = 0;

  while (total < len) {
    size_t read = fread(buff, 1, 128, file);
    IOTJS_ASSERT(read > 0);

    contents.Append(buff, read);
    total += read;
  }

  fclose(file);

  return contents;
}


char* AllocBuffer(size_t size) {
  char* buff = reinterpret_cast<char*>(malloc(size));
  memset(buff, 0, size);
  return buff;
}


char* ReallocBuffer(char* buffer, size_t size) {
  return reinterpret_cast<char*>(realloc(buffer, size));
}


void ReleaseBuffer(char* buffer) {
  free(buffer);
}


String::String() {
  _size = _cap = 0;
  _data = NULL;
}


String::String(const char* data, int size, int cap) {
  if (size < 0) {
    if (data != NULL) {
      _size = strlen(data);
    } else {
      _size = 0;
    }
  } else {
    _size = size;
  }

  if (cap < 0) {
    _cap = _size;
  } else {
    _cap = cap;
  }

  if (_cap < _size) {
    _cap = _size;
  }

  IOTJS_ASSERT(_size >= 0);
  IOTJS_ASSERT(_cap >= 0);

  if (_cap > 0) {
    _data = AllocBuffer(_cap + 1);
  } else {
    _data = NULL;
  }

  if (_data != NULL && data != NULL) {
    strncpy(_data, data, _size);
  }
}


String::~String() {
  IOTJS_ASSERT(_cap == 0 || _data != NULL);

  if (_data != NULL) {
    ReleaseBuffer(_data);
  }
}


bool String::IsEmpty() const {
  IOTJS_ASSERT(_size >= 0);
  return _size == 0;
}


void String::MakeEmpty() {
  if (_data != NULL) {
    ReleaseBuffer(_data);
    _cap = _size = 0;
    _data = NULL;
  }
}


void String::Append(const char* data, int size) {
  const int kCapacityIncreaseFactor = 2;

  IOTJS_ASSERT(_cap == 0 || _data != NULL);
  IOTJS_ASSERT(_cap >= _size);

  if (data == NULL) {
    return;
  }
  if (size < 0) {
    size = strlen(data);
  }
  if (size == 0) {
    return;
  }

  if (_cap == 0) {
    // No buffer was allocated.
    IOTJS_ASSERT(_size == 0);
    IOTJS_ASSERT(_data == NULL);
    _cap = _size = size;
    _data = AllocBuffer(_cap + 1);
    strncpy(_data, data, _size);
  } else if (_cap >= _size + size) {
    // Have enough capacity to append data.
    strncpy(_data + _size, data, size);
    _size += size;
  } else {
    // Lack of capacity, calculate next capacity.
    while (_cap < _size + size) {
      _cap *= kCapacityIncreaseFactor;
    }
    // Reallocate buffer and copy data.
    _data = ReallocBuffer(_data, _cap + 1);
    strncpy(_data + _size, data, size);
    _size += size;
  }
  _data[_size] = 0;
}


char* String::data() const {
  IOTJS_ASSERT(_size == 0 || _data != NULL);
  return _data;
}


int String::size() const {
  IOTJS_ASSERT(_size >= 0);
  return _size;
}


} // namespace iotjs
