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


#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "iotjs_def.h"
#include "module/iotjs_module_blehcisocket.h"

#define THIS iotjs_blehcisocket_t* blehcisocket


void iotjs_blehcisocket_initialize(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_blehcisocket_t, blehcisocket);
}


void iotjs_blehcisocket_start(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_blehcisocket_t, blehcisocket);

  // _this->_devId;
}


void iotjs_blehcisocket_bindRaw(THIS, int* devId) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_blehcisocket_t, blehcisocket);
}


void iotjs_blehcisocket_bindUser(THIS, int* devId) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_blehcisocket_t, blehcisocket);
}


void iotjs_blehcisocket_bindControl(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_blehcisocket_t, blehcisocket);
}


void iotjs_blehcisocket_isDevUp(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_blehcisocket_t, blehcisocket);
}


void iotjs_blehcisocket_setFilter(THIS, char* data, int length) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_blehcisocket_t, blehcisocket);
}


void iotjs_blehcisocket_poll(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_blehcisocket_t, blehcisocket);

  //
  // emit('data') implementation
  //
  // iotjs_jval_t* jhcisocket = iotjs_jobjectwrap_jobject(&_this->jobjectwrap);
  // iotjs_jval_t jemit = iotjs_jval_get_property(jhcisocket, "emit");
  // IOTJS_ASSERT(iotjs_jval_is_function(&jemit));

  // iotjs_jargs_t jargs = iotjs_jargs_create(2);
  // iotjs_jval_t str = iotjs_jval_create_string_raw("data");
  // iotjs_jval_t jbuf = iotjs_bufferwrap_create_buffer(length);
  // iotjs_bufferwrap_t* buf_wrap = iotjs_bufferwrap_from_jbuffer(&jbuf);
  // iotjs_bufferwrap_copy(buf_wrap, data, length);
  // iotjs_jargs_append_jval(&jargs, &str);
  // iotjs_jargs_append_jval(&jargs, &jbuf);
  // iotjs_jhelper_call_ok(&jemit, jhcisocket, &jargs);

  // iotjs_jval_destroy(&str);
  // iotjs_jval_destroy(&jbuf);
  // iotjs_jargs_destroy(&jargs);
  // iotjs_jval_destroy(&jemit);
}


void iotjs_blehcisocket_stop(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_blehcisocket_t, blehcisocket);
}


void iotjs_blehcisocket_write(THIS, char* data, int length) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_blehcisocket_t, blehcisocket);
}


void iotjs_blehcisocket_emitErrnoError(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_blehcisocket_t, blehcisocket);
}


void iotjs_blehcisocket_devIdFor(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_blehcisocket_t, blehcisocket);
}


void iotjs_blehcisocket_kernelDisconnectWorkArounds(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_blehcisocket_t, blehcisocket);
}


#undef THIS

#endif /* IOTJS_MODULE_BLE_HCI_SOCKET_LINUX_GENERAL_INL_H */
