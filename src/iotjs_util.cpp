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
  size_t len = ftell(file);
  fseek(file, 0, SEEK_SET);

  char* buff = AllocBuffer(len + 1);

  size_t read = fread(buff, 1, len, file);
  IOTJS_ASSERT(read == len);

  *(buff+len) = 0;

  fclose(file);

  return String(buff);
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


String::String(const char* data, int size) {
  if (size < 0) {
    _size = strlen(data);
  } else {
    _size = size;
  }

  if (_size > 0) {
    IOTJS_ASSERT(data != NULL);
    _data = AllocBuffer(_size + 1);
    strncpy(_data, data, _size);
  } else {
    _data = NULL;
  }
}


String::~String() {
  IOTJS_ASSERT(_size == 0 || _data != NULL);

  if (_data != NULL) {
    ReleaseBuffer(_data);
  }
}


bool String::IsEmpty() const {
  IOTJS_ASSERT(_size >= 0);
  return _size == 0;
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
