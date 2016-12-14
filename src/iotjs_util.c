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
#include "iotjs_util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


iotjs_string_t iotjs_file_read(const char* path) {
  FILE* file = fopen(path, "rb");
  IOTJS_ASSERT(file != NULL);

  fseek(file, 0, SEEK_END);
  long len = ftell(file);
  IOTJS_ASSERT(len >= 0);
  fseek(file, 0, SEEK_SET);

  char* buffer = iotjs_buffer_allocate(len + 1);

#if defined(__NUTTX__)
  char* ptr = buffer;
  unsigned nread = 0;
  unsigned read = 0;

  while ((nread = fread(ptr, 1, IOTJS_MAX_READ_BUFFER_SIZE, file)) > 0) {
    read += nread;
    ptr = buffer + read;
  }
#else
  size_t read = fread(buffer, 1, len, file);
#endif
  IOTJS_ASSERT(read == (size_t)len);

  *(buffer + len) = 0;

  fclose(file);

  iotjs_string_t contents = iotjs_string_create_with_buffer(buffer, len);

  return contents;
}


char* iotjs_buffer_allocate(unsigned size) {
  char* buffer = (char*)(calloc(size, sizeof(char)));
  IOTJS_ASSERT(buffer != NULL);
  return buffer;
}


char* iotjs_buffer_reallocate(char* buffer, unsigned size) {
  IOTJS_ASSERT(buffer != NULL);
  return (char*)(realloc(buffer, size));
}


void iotjs_buffer_release(char* buffer) {
  IOTJS_ASSERT(buffer != NULL);
  free(buffer);
}
