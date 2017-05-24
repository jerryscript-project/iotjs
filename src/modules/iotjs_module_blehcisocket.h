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

/* Copyright (C) 2015 Sandeep Mistry sandeep.mistry@gmail.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
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
  int _l2sockets[1024];
  int _l2socketCount;
  uint8_t _address[6];
  uint8_t _addressType;
} IOTJS_VALIDATED_STRUCT(iotjs_blehcisocket_t);


#define THIS iotjs_blehcisocket_t* iotjs_blehcisocket


iotjs_blehcisocket_t* iotjs_blehcisocket_create(const iotjs_jval_t* jble);
iotjs_blehcisocket_t* iotjs_blehcisocket_instance_from_jval(
    const iotjs_jval_t* jble);


void iotjs_blehcisocket_initialize(THIS);
void iotjs_blehcisocket_close(THIS);
void iotjs_blehcisocket_start(THIS);
int iotjs_blehcisocket_bindRaw(THIS, int* devId);
int iotjs_blehcisocket_bindUser(THIS, int* devId);
void iotjs_blehcisocket_bindControl(THIS);
bool iotjs_blehcisocket_isDevUp(THIS);
void iotjs_blehcisocket_setFilter(THIS, char* data, size_t length);
void iotjs_blehcisocket_poll(THIS);
void iotjs_blehcisocket_stop(THIS);
void iotjs_blehcisocket_write(THIS, char* data, size_t length);
void iotjs_blehcisocket_emitErrnoError(THIS);
int iotjs_blehcisocket_devIdFor(THIS, int* pDevId, bool isUp);
int iotjs_blehcisocket_kernelDisconnectWorkArounds(THIS, int length,
                                                   char* data);

#undef THIS

void iotjs_blehcisocket_poll_cb(uv_poll_t* handle, int status, int events);


#endif /* IOTJS_MODULE_BLE_HCI_SOCKET_H */
