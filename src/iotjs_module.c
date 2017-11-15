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
#include "iotjs_module.h"

typedef struct { iotjs_jval_t jmodule; } iotjs_module_objects_t;

#include "iotjs_module_inl.h"

/**
 * iotjs_module_inl.h provides:
 *  - iotjs_modules[]
 *  - iotjs_module_objects[]
 */

const unsigned iotjs_modules_count =
    sizeof(iotjs_modules) / sizeof(iotjs_module_t);

void iotjs_module_list_cleanup() {
  for (unsigned i = 0; i < iotjs_modules_count; i++) {
    if (iotjs_module_objects[i].jmodule != 0) {
      jerry_release_value(iotjs_module_objects[i].jmodule);
    }
  }
}

iotjs_jval_t iotjs_module_get(const char* name) {
  for (unsigned i = 0; i < iotjs_modules_count; i++) {
    if (!strcmp(name, iotjs_modules[i].name)) {
      if (iotjs_module_objects[i].jmodule == 0) {
        iotjs_module_objects[i].jmodule = iotjs_modules[i].fn_register();
      }

      return iotjs_module_objects[i].jmodule;
    }
  }

  return jerry_create_undefined();
}
