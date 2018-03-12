/* Copyright 2017-present Samsung Electronics Co., Ltd. and other contributors
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
#include "modules/iotjs_module_blehcisocket.h"


void iotjs_blehcisocket_initialize(iotjs_blehcisocket_t* blehcisocket) {
  IOTJS_ASSERT(!"Not implemented");
}


void iotjs_blehcisocket_close(iotjs_blehcisocket_t* blehcisocket) {
  IOTJS_ASSERT(!"Not implemented");
}


void iotjs_blehcisocket_start(iotjs_blehcisocket_t* blehcisocket) {
  IOTJS_ASSERT(!"Not implemented");
}


int iotjs_blehcisocket_bindRaw(iotjs_blehcisocket_t* blehcisocket, int* devId) {
  IOTJS_ASSERT(!"Not implemented");
  return 0;
}


int iotjs_blehcisocket_bindUser(iotjs_blehcisocket_t* blehcisocket,
                                int* devId) {
  IOTJS_ASSERT(!"Not implemented");
  return 0;
}


void iotjs_blehcisocket_bindControl(iotjs_blehcisocket_t* blehcisocket) {
  IOTJS_ASSERT(!"Not implemented");
}


bool iotjs_blehcisocket_isDevUp(iotjs_blehcisocket_t* blehcisocket) {
  IOTJS_ASSERT(!"Not implemented");
  return false;
}


void iotjs_blehcisocket_setFilter(iotjs_blehcisocket_t* blehcisocket,
                                  char* data, size_t length) {
  IOTJS_ASSERT(!"Not implemented");
}


void iotjs_blehcisocket_poll(iotjs_blehcisocket_t* blehcisocket) {
  IOTJS_ASSERT(!"Not implemented");
}


void iotjs_blehcisocket_stop(iotjs_blehcisocket_t* blehcisocket) {
  IOTJS_ASSERT(!"Not implemented");
}


void iotjs_blehcisocket_write(iotjs_blehcisocket_t* blehcisocket, char* data,
                              size_t length) {
  IOTJS_ASSERT(!"Not implemented");
}


void iotjs_blehcisocket_emitErrnoError(iotjs_blehcisocket_t* blehcisocket) {
  IOTJS_ASSERT(!"Not implemented");
}


int iotjs_blehcisocket_devIdFor(iotjs_blehcisocket_t* blehcisocket, int* pDevId,
                                bool isUp) {
  IOTJS_ASSERT(!"Not implemented");
  return 0;
}


int iotjs_blehcisocket_kernelDisconnectWorkArounds(
    iotjs_blehcisocket_t* blehcisocket, int length, char* data) {
  IOTJS_ASSERT(!"Not implemented");
  return 0;
}
