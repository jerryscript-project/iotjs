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


JObject* InitProcess() {
  Module* module = GetBuiltinModule(MODULE_PROCESS);
  JObject* process = module->module;

  if (process == NULL) {
    process = new JObject();
    process->SetMethod("binding", Binding);

    module->module = process;
  }

  return process;
}

} // namespace iotjs
