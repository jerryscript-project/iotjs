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

#ifndef IOTJS_MODULE_BLE_HCI_SOCKET_LINUX_GENERAL_INL_H
#define IOTJS_MODULE_BLE_HCI_SOCKET_LINUX_GENERAL_INL_H


#include "iotjs_def.h"
#include "modules/iotjs_module_blehcisocket.h"

#define THIS iotjs_blehcisocket_t* blehcisocket


void iotjs_blehcisocket_initialize(THIS) {
  IOTJS_ASSERT(!"Not implemented");
}


void iotjs_blehcisocket_close(THIS) {
  IOTJS_ASSERT(!"Not implemented");
}


void iotjs_blehcisocket_start(THIS) {
  IOTJS_ASSERT(!"Not implemented");
}


int iotjs_blehcisocket_bindRaw(THIS, int* devId) {
  IOTJS_ASSERT(!"Not implemented");
  return 0;
}


int iotjs_blehcisocket_bindUser(THIS, int* devId) {
  IOTJS_ASSERT(!"Not implemented");
  return 0;
}


void iotjs_blehcisocket_bindControl(THIS) {
  IOTJS_ASSERT(!"Not implemented");
}


bool iotjs_blehcisocket_isDevUp(THIS) {
  IOTJS_ASSERT(!"Not implemented");
  return false;
}


void iotjs_blehcisocket_setFilter(THIS, char* data, size_t length) {
  IOTJS_ASSERT(!"Not implemented");
}


void iotjs_blehcisocket_poll(THIS) {
  IOTJS_ASSERT(!"Not implemented");
}


void iotjs_blehcisocket_stop(THIS) {
  IOTJS_ASSERT(!"Not implemented");
}


void iotjs_blehcisocket_write(THIS, char* data, size_t length) {
  IOTJS_ASSERT(!"Not implemented");
}


void iotjs_blehcisocket_emitErrnoError(THIS) {
  IOTJS_ASSERT(!"Not implemented");
}


int iotjs_blehcisocket_devIdFor(THIS, int* pDevId, bool isUp) {
  IOTJS_ASSERT(!"Not implemented");
  return 0;
}


int iotjs_blehcisocket_kernelDisconnectWorkArounds(THIS, int length,
                                                   char* data) {
  IOTJS_ASSERT(!"Not implemented");
  return 0;
}


#undef THIS

#endif /* IOTJS_MODULE_BLE_HCI_SOCKET_LINUX_GENERAL_INL_H */
