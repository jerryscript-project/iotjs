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


#if defined(__APPLE__)

#include "module/iotjs_module_blehcisocket.h"
#include "iotjs_module_unimplemented.inl.h"

#define THIS iotjs_blehcisocket_t* iotjs_blehcisocket

void iotjs_blehcisocket_initialize(THIS) IOTJS_MODULE_UNIMPLEMENTED();
void iotjs_blehcisocket_close(THIS) IOTJS_MODULE_UNIMPLEMENTED();
void iotjs_blehcisocket_start(THIS) IOTJS_MODULE_UNIMPLEMENTED();
int iotjs_blehcisocket_bindRaw(THIS, int* devId) IOTJS_MODULE_UNIMPLEMENTED(0);
int iotjs_blehcisocket_bindUser(THIS, int* devId) IOTJS_MODULE_UNIMPLEMENTED(0);
void iotjs_blehcisocket_bindControl(THIS) IOTJS_MODULE_UNIMPLEMENTED();
bool iotjs_blehcisocket_isDevUp(THIS) IOTJS_MODULE_UNIMPLEMENTED(false);
void iotjs_blehcisocket_setFilter(THIS, char* data, int length)
    IOTJS_MODULE_UNIMPLEMENTED();
void iotjs_blehcisocket_stop(THIS) IOTJS_MODULE_UNIMPLEMENTED();
void iotjs_blehcisocket_write(THIS, char* data, int length)
    IOTJS_MODULE_UNIMPLEMENTED();

#undef THIS

#endif // __APPLE__
