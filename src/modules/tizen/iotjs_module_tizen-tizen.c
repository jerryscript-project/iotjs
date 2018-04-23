/* Copyright 2018-present Samsung Electronics Co., Ltd. and other contributors
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
#include "modules/iotjs_module_bridge.h"

#include <app_common.h>

void iotjs_tizen_func(const char* command, const char* message, void* handle) {
  if (strncmp(command, "getResPath", strlen("getResPath")) == 0) {
    char* app_res_path = app_get_resource_path();
    iotjs_bridge_set_msg(handle, app_res_path);
  } else {
    iotjs_bridge_set_err(handle, "Can't find command");
  }
}
