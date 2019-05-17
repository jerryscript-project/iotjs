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
#include "iotjs_module_bridge.h"

extern void iotjs_tizen_func(const char* command, const char* message,
                             void* handle);

/**
 * Init method called by IoT.js
 */
jerry_value_t iotjs_init_tizen() {
  char* module_name = IOTJS_MAGIC_STRING_TIZEN;
  jerry_value_t mymodule = jerry_create_object();
  iotjs_jval_set_property_string_raw(mymodule, IOTJS_MAGIC_STRING_MODULE_NAME,
                                     module_name);

  iotjs_bridge_register(module_name, iotjs_tizen_func);
  return mymodule;
}
