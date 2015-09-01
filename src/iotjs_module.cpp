/* Copyright 2015 Samsung Electronics Co., Ltd.
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

#include "iotjs_module_buffer.h"
#include "iotjs_module_console.h"
#include "iotjs_module_constants.h"
#include "iotjs_module_fs.h"
#include "iotjs_module_process.h"
#include "iotjs_module_tcp.h"
#include "iotjs_module_timer.h"
#include "iotjs_module_gpio.h"
#include "iotjs_module_httpparser.h"
#include "iotjs_module_dns.h"


namespace iotjs {


static Module _modules[MODULE_COUNT];


#define INIT_MODULE_LIST(upper, Camel, lower) \
  _modules[MODULE_ ## upper].kind = MODULE_ ## upper; \
  _modules[MODULE_ ## upper].module = NULL; \
  _modules[MODULE_ ## upper].fn_register = Init ## Camel;

void InitModuleList() {
  MAP_MODULE_LIST(INIT_MODULE_LIST)
}

#undef INIT_MODULE_LIST


#define CLENUP_MODULE_LIST(upper, Camel, lower) \
  if (_modules[MODULE_ ## upper].module) \
    delete _modules[MODULE_ ## upper].module; \
  _modules[MODULE_ ## upper].module = NULL;

void CleanupModuleList() {
  MAP_MODULE_LIST(CLENUP_MODULE_LIST)
}

#undef CLENUP_MODULE_LIST


Module* GetBuiltinModule(ModuleKind kind) {
  IOTJS_ASSERT(kind < MODULE_COUNT);
  return &_modules[kind];
}


} // namespace iotjs
