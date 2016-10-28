/* Copyright 2015-2016 Samsung Electronics Co., Ltd.
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
#include "module/iotjs_module_process.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


namespace iotjs {


void UncaughtException(const iotjs_jval_t* jexception) {
  iotjs_jval_t* process = GetProcess();

  iotjs_jval_t jonuncaughtexception =
      iotjs_jval_get_property(process, "_onUncaughtExcecption");
  IOTJS_ASSERT(iotjs_jval_is_function(&jonuncaughtexception));

  iotjs_jargs_t args = iotjs_jargs_create(1);
  iotjs_jargs_append_jval(&args, jexception);

  iotjs_jval_t jres = iotjs_jhelper_call_ok(&jonuncaughtexception, process,
                                            &args);
  iotjs_jval_destroy(&jres);

  iotjs_jargs_destroy(&args);
  iotjs_jval_destroy(&jonuncaughtexception);
}


void ProcessEmitExit(int code) {
  iotjs_jval_t* process = GetProcess();

  iotjs_jval_t jexit = iotjs_jval_get_property(process, "emitExit");
  IOTJS_ASSERT(iotjs_jval_is_function(&jexit));

  iotjs_jargs_t jargs = iotjs_jargs_create(1);
  iotjs_jargs_append_number(&jargs, code);

  bool throws;
  iotjs_jval_t jres = iotjs_jhelper_call(&jexit, process, &jargs, &throws);

  iotjs_jargs_destroy(&jargs);
  iotjs_jval_destroy(&jres);
  iotjs_jval_destroy(&jexit);

  if (throws) {
    exit(2);
  }
}


// Calls next tick callbacks registered via `process.nextTick()`.
bool ProcessNextTick() {
  iotjs_jval_t* process = GetProcess();

  iotjs_jval_t jon_next_tick = iotjs_jval_get_property(process, "_onNextTick");
  IOTJS_ASSERT(iotjs_jval_is_function(&jon_next_tick));

  iotjs_jval_t jres = iotjs_jhelper_call_ok(&jon_next_tick,
                                            iotjs_jval_get_undefined(),
                                            iotjs_jargs_get_empty());

  IOTJS_ASSERT(iotjs_jval_is_boolean(&jres));

  bool ret = iotjs_jval_as_boolean(&jres);
  iotjs_jval_destroy(&jres);
  iotjs_jval_destroy(&jon_next_tick);

  return ret;
}


// Make a callback for the given `function` with `this_` binding and `args`
// arguments. The next tick callbacks registered via `process.nextTick()`
// will be called after the callback function `function` returns.
void MakeCallback(const iotjs_jval_t* jfunction,
                  const iotjs_jval_t* jthis,
                  const iotjs_jargs_t* jargs) {
  iotjs_jval_t result = MakeCallbackWithResult(jfunction, jthis, jargs);
  iotjs_jval_destroy(&result);
}


iotjs_jval_t MakeCallbackWithResult(const iotjs_jval_t* jfunction,
                                    const iotjs_jval_t* jthis,
                                    const iotjs_jargs_t* jargs) {
  // Calls back the function.
  bool throws;
  iotjs_jval_t jres = iotjs_jhelper_call(jfunction, jthis, jargs, &throws);
  if (throws) {
    UncaughtException(&jres);
  }

  // Calls the next tick callbacks.
  ProcessNextTick();

  // Return value.
  return jres;
}


iotjs_jval_t* InitProcessModule() {
  Module* module = GetBuiltinModule(MODULE_PROCESS);
  module->module = InitProcess();
  return &module->module;
}


} // namespace iotjs
