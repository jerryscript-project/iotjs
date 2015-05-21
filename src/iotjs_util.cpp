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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "iotjs_util.h"


namespace iotjs {


char* ReadFile(const char* path) {
  FILE* file = fopen(path, "rb");
  assert(file != NULL);

  fseek(file, 0, SEEK_END);
  size_t len = ftell(file);
  fseek(file, 0, SEEK_SET);

  char* buff = AllocCharBuffer(len + 1);

  size_t read = fread(buff, 1, len, file);
  assert(read == len);

  *(buff+len) = 0;

  fclose(file);

  return buff;
}


char* AllocCharBuffer(size_t size) {
  char* buff = static_cast<char*>(malloc(size));
  memset(buff, 0, size);
  return buff;
}


void ReleaseCharBuffer(char* buffer) {
  free(buffer);
}

} // namespace iotjs
