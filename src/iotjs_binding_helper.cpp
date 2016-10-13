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


void UncaughtException(JObject& jexception) {
  JObject* process = GetProcess();

  JObject jonuncaughtexception(process->GetProperty("_onUncaughtExcecption"));
  IOTJS_ASSERT(jonuncaughtexception.IsFunction());

  JRawValueType jexception_val = jerry_acquire_value(jexception.raw_value());
  jerry_value_clear_error_flag(&jexception_val);
  JObject jexception_obj(jexception_val);

  JArgList args(1);
  args.Add(jexception_obj);

  JResult jres = jonuncaughtexception.Call(*process, args);
  IOTJS_ASSERT(jres.IsOk());
}


void ProcessEmitExit(int code) {
  JObject* process = GetProcess();

  JObject jexit(process->GetProperty("emitExit"));
  IOTJS_ASSERT(jexit.IsFunction());

  JArgList args(1);
  args.Add(JVal::Number(code));

  JResult jres = jexit.Call(JObject::Undefined(), args);
  if (!jres.IsOk()) {
    exit(2);
  }
}


// Calls next tick callbacks registered via `process.nextTick()`.
bool ProcessNextTick() {
  JObject* process = GetProcess();

  JObject jon_next_tick(process->GetProperty("_onNextTick"));
  IOTJS_ASSERT(jon_next_tick.IsFunction());

  JResult jres = jon_next_tick.Call(JObject::Undefined(), JArgList::Empty());
  IOTJS_ASSERT(jres.IsOk());
  IOTJS_ASSERT(jres.value().IsBoolean());

  return jres.value().GetBoolean();
}


// Make a callback for the given `function` with `this_` binding and `args`
// arguments. The next tick callbacks registered via `process.nextTick()`
// will be called after the callback function `function` returns.
JObject MakeCallback(JObject& function, JObject& this_, JArgList& args) {
  // Calls back the function.
  JResult jres = function.Call(this_, args);
  if (jres.IsException()) {
    UncaughtException(jres.value());
  }

  // Calls the next tick callbacks.
  ProcessNextTick();

  // Return value.
  return jres.value();
}


JObject* InitProcessModule() {
  return InitProcess();
}


} // namespace iotjs
