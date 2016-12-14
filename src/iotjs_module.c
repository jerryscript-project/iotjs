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


typedef struct {
  ModuleKind kind;
  iotjs_jval_t jmodule;
  register_func fn_register;
} iotjs_module_t;


static iotjs_module_t modules[MODULE_COUNT];


#define DECLARE_MODULE_INITIALIZER(upper, Camel, lower) \
  iotjs_jval_t Init##Camel();

MAP_MODULE_LIST(DECLARE_MODULE_INITIALIZER)

#undef DECLARE_MODULE_INITIALIZER


#define INIT_MODULE_LIST(upper, Camel, lower)                    \
  modules[MODULE_##upper].kind = MODULE_##upper;                 \
  modules[MODULE_##upper].jmodule = *iotjs_jval_get_undefined(); \
  modules[MODULE_##upper].fn_register = Init##Camel;

void iotjs_module_list_init() {
  MAP_MODULE_LIST(INIT_MODULE_LIST)
}

#undef INIT_MODULE_LIST


#define CLENUP_MODULE_LIST(upper, Camel, lower)                   \
  if (!iotjs_jval_is_undefined(&modules[MODULE_##upper].jmodule)) \
    iotjs_jval_destroy(&modules[MODULE_##upper].jmodule);

void iotjs_module_list_cleanup() {
  MAP_MODULE_LIST(CLENUP_MODULE_LIST)
}

#undef CLENUP_MODULE_LIST


const iotjs_jval_t* iotjs_module_initialize_if_necessary(ModuleKind kind) {
  IOTJS_ASSERT(kind < MODULE_COUNT);
  IOTJS_ASSERT(&modules[kind].fn_register != NULL);

  if (iotjs_jval_is_undefined(&modules[kind].jmodule)) {
    modules[kind].jmodule = modules[kind].fn_register();
  }

  return iotjs_module_get(kind);
}


const iotjs_jval_t* iotjs_module_get(ModuleKind kind) {
  IOTJS_ASSERT(kind < MODULE_COUNT);
  IOTJS_ASSERT(!iotjs_jval_is_undefined(&modules[kind].jmodule));
  return &modules[kind].jmodule;
}
