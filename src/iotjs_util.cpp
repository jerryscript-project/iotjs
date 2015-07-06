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


int jstrlen(const jschar* jstr) {
  return strlen(reinterpret_cast<const char*>(jstr));
}


jschar* jstrcpy(jschar* dst, const jschar* src) {
  return reinterpret_cast<jschar*>(strcpy(reinterpret_cast<char*>(dst),
                                          reinterpret_cast<const char*>(src)));
}


jschar* jstrcat(jschar* dst, const jschar* src) {
  return reinterpret_cast<jschar*>(strcat(reinterpret_cast<char*>(dst),
                                          reinterpret_cast<const char*>(src)));
}


jschar* jstrncpy(jschar* dst, const jschar* src, size_t num) {
  return reinterpret_cast<jschar*>(strncpy(reinterpret_cast<char*>(dst),
                                           reinterpret_cast<const char*>(src),
                                           num));
}


jschar* ReadFile(const jschar* path) {
  FILE* file = fopen((const char*)path, "rb");
  IOTJS_ASSERT(file != NULL);

  fseek(file, 0, SEEK_END);
  size_t len = ftell(file);
  fseek(file, 0, SEEK_SET);

  jschar* buff = static_cast<jschar*>(AllocBuffer(len + 1));

  size_t read = fread(buff, 1, len, file);
  IOTJS_ASSERT(read == len);

  *(buff+len) = 0;

  fclose(file);

  return buff;
}


octet* AllocBuffer(size_t size) {
  octet* buff = static_cast<octet*>(malloc(size));
  memset(buff, 0, size);
  return buff;
}


octet* ReallocBuffer(octet* buffer, size_t size) {
  return static_cast<octet*>(realloc(buffer, size));
}


void ReleaseBuffer(octet* buffer) {
  free(buffer);
}


LocalString::LocalString(size_t len)
    : _strp(static_cast<jschar*>(AllocBuffer(len))) {
  IOTJS_ASSERT(_strp != NULL);
}

LocalString::LocalString(jschar* strp)
    : _strp(strp) {
  IOTJS_ASSERT(_strp != NULL);
}


LocalString::~LocalString() {
  IOTJS_ASSERT(_strp != NULL);
  ReleaseBuffer(const_cast<jschar*>(_strp));
}


LocalString::operator jschar* () const {
  return _strp;
}


} // namespace iotjs
