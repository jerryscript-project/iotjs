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


#ifndef IOTJS_MODULE_BLE_HCI_SOCKET_H
#define IOTJS_MODULE_BLE_HCI_SOCKET_H

#include "iotjs_def.h"
#include "iotjs_objectwrap.h"
#include "iotjs_reqwrap.h"

typedef struct {
  iotjs_jobjectwrap_t jobjectwrap;

  int _mode;
  int _socket;
  int _devId;
  uv_poll_t _pollHandle;
  int _l2socket;
  uint8_t _address[6];
  uint8_t _addressType;
} IOTJS_VALIDATED_STRUCT(iotjs_blehcisocket_t);


#define THIS iotjs_blehcisocket_t* iotjs_blehcisocket


iotjs_blehcisocket_t* iotjs_blehcisocket_create(const iotjs_jval_t* jble);
iotjs_blehcisocket_t* iotjs_blehcisocket_instance_from_jval(
    const iotjs_jval_t* jble);


void iotjs_blehcisocket_initialize(THIS);
void iotjs_blehcisocket_start(THIS);
void iotjs_blehcisocket_bindRaw(THIS, int* devId);
void iotjs_blehcisocket_bindUser(THIS, int* devId);
void iotjs_blehcisocket_bindControl(THIS);
void iotjs_blehcisocket_isDevUp(THIS);
void iotjs_blehcisocket_setFilter(THIS, char* data, int length);
void iotjs_blehcisocket_poll(THIS);
void iotjs_blehcisocket_stop(THIS);
void iotjs_blehcisocket_write(THIS, char* data, int length);
void iotjs_blehcisocket_emitErrnoError(THIS);
void iotjs_blehcisocket_devIdFor(THIS);
void iotjs_blehcisocket_kernelDisconnectWorkArounds(THIS);

#undef THIS

#endif /* IOTJS_MODULE_BLE_HCI_SOCKET_H */
