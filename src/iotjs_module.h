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

#ifndef IOTJS_MODULE_H
#define IOTJS_MODULE_H

#include "iotjs_binding.h"


namespace iotjs {


typedef JObject* (*register_func)();


// List of builtin modules
#define MAP_MODULE_LIST(F) \
  F(BUFFER, Buffer, buffer) \
  F(CONSOLE, Console, console) \
  F(CONSTANTS, Constants, constants) \
  F(DNS, Dns, dns) \
  F(FS, Fs, fs) \
  F(GPIO, Gpio, gpio) \
  F(HTTPPARSER, Httpparser, httpparser) \
  F(PROCESS, Process, process) \
  F(TCP, Tcp, tcp) \
  F(TIMER, Timer, timer)

#define ENUMDEF_MODULE_LIST(upper, Camel, lower) \
  MODULE_ ## upper,

enum ModuleKind {
  MAP_MODULE_LIST(ENUMDEF_MODULE_LIST)
  MODULE_COUNT
};

#undef ENUMDEF_MODULE_LIST


struct Module {
  ModuleKind kind;
  JObject* module;
  register_func fn_register;
};


void InitModuleList();

void CleanupModuleList();

Module* GetBuiltinModule(ModuleKind kind);


} // namespace iotjs


#endif /* IOTJS_MODULE_H */
