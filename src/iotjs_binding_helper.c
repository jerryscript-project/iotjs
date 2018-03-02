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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void iotjs_uncaught_exception(jerry_value_t jexception) {
  const jerry_value_t process = iotjs_module_get("process");

  jerry_value_t jonuncaughtexception =
      iotjs_jval_get_property(process, IOTJS_MAGIC_STRING__ONUNCAUGHTEXCEPTION);
  IOTJS_ASSERT(jerry_value_is_function(jonuncaughtexception));

  iotjs_jargs_t args = iotjs_jargs_create(1);
  iotjs_jargs_append_jval(&args, jexception);

  jerry_value_t jres = iotjs_jhelper_call(jonuncaughtexception, process, &args);

  iotjs_jargs_destroy(&args);
  jerry_release_value(jonuncaughtexception);

  if (jerry_value_has_error_flag(jres)) {
    iotjs_environment_t* env = iotjs_environment_get();

    if (!iotjs_environment_is_exiting(env)) {
      iotjs_set_process_exitcode(2);
      iotjs_environment_set_state(env, kExiting);
    }
  }

  jerry_release_value(jres);
}


void iotjs_process_emit_exit(int code) {
  const jerry_value_t process = iotjs_module_get("process");

  jerry_value_t jexit =
      iotjs_jval_get_property(process, IOTJS_MAGIC_STRING_EMITEXIT);

  if (jerry_value_is_function(jexit)) {
    iotjs_jargs_t jargs = iotjs_jargs_create(1);
    iotjs_jargs_append_number(&jargs, code);

    jerry_value_t jres = iotjs_jhelper_call(jexit, process, &jargs);

    if (jerry_value_has_error_flag(jres)) {
      iotjs_set_process_exitcode(2);
    }

    iotjs_jargs_destroy(&jargs);
    jerry_release_value(jres);
  }

  jerry_release_value(jexit);
}


// Calls next tick callbacks registered via `process.nextTick()`.
bool iotjs_process_next_tick() {
  iotjs_environment_t* env = iotjs_environment_get();

  if (iotjs_environment_is_exiting(env)) {
    return false;
  }

  const jerry_value_t process = iotjs_module_get("process");

  jerry_value_t jon_next_tick =
      iotjs_jval_get_property(process, IOTJS_MAGIC_STRING__ONNEXTTICK);
  IOTJS_ASSERT(jerry_value_is_function(jon_next_tick));

  jerry_value_t jres =
      iotjs_jhelper_call(jon_next_tick, jerry_create_undefined(),
                         iotjs_jargs_get_empty());

  bool ret = false;

  if (!jerry_value_has_error_flag(jres)) {
    ret = iotjs_jval_as_boolean(jres);
  }

  jerry_release_value(jres);
  jerry_release_value(jon_next_tick);

  return ret;
}


// Make a callback for the given `function` with `this_` binding and `args`
// arguments. The next tick callbacks registered via `process.nextTick()`
// will be called after the callback function `function` returns.
void iotjs_make_callback(jerry_value_t jfunction, jerry_value_t jthis,
                         const iotjs_jargs_t* jargs) {
  jerry_value_t result =
      iotjs_make_callback_with_result(jfunction, jthis, jargs);
  jerry_release_value(result);
}


jerry_value_t iotjs_make_callback_with_result(jerry_value_t jfunction,
                                              jerry_value_t jthis,
                                              const iotjs_jargs_t* jargs) {
  // If the environment is already exiting just return an undefined value.
  if (iotjs_environment_is_exiting(iotjs_environment_get())) {
    return jerry_create_undefined();
  }
  // Calls back the function.
  jerry_value_t jres = iotjs_jhelper_call(jfunction, jthis, jargs);
  if (jerry_value_has_error_flag(jres)) {
    jerry_value_t errval = jerry_get_value_without_error_flag(jres);
    iotjs_uncaught_exception(errval);
    jerry_release_value(errval);
  }

  // Calls the next tick callbacks.
  iotjs_process_next_tick();

  // Return value.
  return jres;
}


int iotjs_process_exitcode() {
  const jerry_value_t process = iotjs_module_get("process");

  jerry_value_t jexitcode =
      iotjs_jval_get_property(process, IOTJS_MAGIC_STRING_EXITCODE);
  uint8_t exitcode = 0;
  jerry_value_t num_val = jerry_value_to_number(jexitcode);
  if (jerry_value_has_error_flag(num_val)) {
    exitcode = 1;
    jerry_value_clear_error_flag(&num_val);
  } else {
    exitcode = (uint8_t)iotjs_jval_as_number(num_val);
  }
  jerry_release_value(num_val);
  jerry_release_value(jexitcode);
  return (int)exitcode;
}


void iotjs_set_process_exitcode(int code) {
  const jerry_value_t process = iotjs_module_get("process");
  iotjs_jval_set_property_number(process, IOTJS_MAGIC_STRING_EXITCODE, code);
}
