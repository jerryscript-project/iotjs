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

#include "iotjs_binding.h"
#include "iotjs_module.h"
#include "iotjs_module_process.h"
#include "iotjs_js.h"


namespace iotjs {


// Calls next tick callbacks registered via `process.nextTick()`.
void OnNextTick() {
  Module* module = GetBuiltinModule(MODULE_PROCESS);
  assert(module != NULL);

  JObject* process = module->module;
  assert(process != NULL);
  assert(process->IsObject());

  JObject jon_next_tick = process->GetProperty("_onNextTick");
  assert(jon_next_tick.IsFunction());

  jon_next_tick.Call(JObject::Null(), JArgList::Empty());
}


// Make a callback for the given `function` with `this_` binding and `args`
// arguments. The next tick callbacks registered via `process.nextTick()`
// will be called after the callback function `function` returned.
JObject MakeCallback(JObject& function, JObject& this_, JArgList& args) {
  // Calls back the function.
  JObject res = function.Call(this_, args);

  // Calls the next tick callbacks.
  OnNextTick();

  // Return value.
  return res;
}


JHANDLER_FUNCTION(Binding, handler) {
  assert(handler.GetArgLength() == 1);
  assert(handler.GetArg(0)->IsNumber());

  int module_kind = handler.GetArg(0)->GetInt32();

  Module* module = GetBuiltinModule(static_cast<ModuleKind>(module_kind));

  if (module->module == NULL) {
    module->module = module->fn_register();
  }

  handler.Return(*module->module);

  return true;
}

JHANDLER_FUNCTION(Compile, handler){
  assert(handler.GetArgLength() == 1);
  assert(handler.GetArg(0)->IsString());

  char* code = handler.GetArg(0)->GetCString();
  JRawValueType ret_val;
  jerry_api_eval(code,sizeof(code),true,false,&ret_val);
  JObject::ReleaseCString(code);

  JObject ret(&ret_val);
  handler.Return(ret);

  return true;
}

JHANDLER_FUNCTION(ReadSource, handler){
  assert(handler.GetArgLength() == 1);
  assert(handler.GetArg(0)->IsString());

  char* code = ReadFile(handler.GetArg(0)->GetCString());
  JObject ret(code);
  handler.Return(ret);

  JObject::ReleaseCString(code);
  return true;
}

void SetNativeSources(JObject* native_sources) {
  for (int i = 0; natives[i].name; i++) {
    JObject native_source(natives[i].source);
    native_sources->SetProperty(natives[i].name, native_source);
  }
}

JObject* InitProcess() {
  Module* module = GetBuiltinModule(MODULE_PROCESS);
  JObject* process = module->module;

  if (process == NULL) {
    process = new JObject();
    process->SetMethod("binding", Binding);
    process->SetMethod("compile", Compile);
    process->SetMethod("readSource", ReadSource);

    // process.native_sources
    JObject native_sources;
    SetNativeSources(&native_sources);
    process->SetProperty("native_sources", native_sources);

    // Binding module id.
    JObject jbinding = process->GetProperty("binding");

#define ENUMDEF_MODULE_LIST(upper, Camel, lower) \
    jbinding.SetProperty(# lower, JVal::Int(MODULE_ ## upper));

    MAP_MODULE_LIST(ENUMDEF_MODULE_LIST)

#undef ENUMDEF_MODULE_LIST

    module->module = process;
  }

  return process;
}


} // namespace iotjs
