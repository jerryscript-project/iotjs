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

  jerry_value_t jres =
      jerry_call_function(jonuncaughtexception, process, &jexception, 1);

  jerry_release_value(jonuncaughtexception);

  if (jerry_value_is_error(jres)) {
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
    jerry_value_t jcode = jerry_create_number(code);
    jerry_value_t jres = jerry_call_function(jexit, process, &jcode, 1);

    if (jerry_value_is_error(jres)) {
      iotjs_set_process_exitcode(2);
    }

    jerry_release_value(jcode);
    jerry_release_value(jres);
  }

  jerry_release_value(jexit);
}


// Calls next tick callbacks registered via `process.nextTick()`.
bool iotjs_process_next_tick(void) {
  iotjs_environment_t* env = iotjs_environment_get();

  if (iotjs_environment_is_exiting(env)) {
    return false;
  }

  const jerry_value_t process = iotjs_module_get("process");

  jerry_value_t jon_next_tick =
      iotjs_jval_get_property(process, IOTJS_MAGIC_STRING__ONNEXTTICK);
  IOTJS_ASSERT(jerry_value_is_function(jon_next_tick));

  jerry_value_t jres =
      jerry_call_function(jon_next_tick, jerry_create_undefined(), NULL, 0);

  bool ret = false;

  if (!jerry_value_is_error(jres)) {
    ret = iotjs_jval_as_boolean(jres);
  }

  jerry_release_value(jres);
  jerry_release_value(jon_next_tick);

  return ret;
}


// Make a callback for the given `function` with `this_` binding and `args`
// arguments. The next tick callbacks registered via `process.nextTick()`
// will be called after the callback function `function` returns.
void iotjs_invoke_callback(jerry_value_t jfunc, jerry_value_t jthis,
                           const jerry_value_t* jargv, size_t jargc) {
  jerry_value_t result =
      iotjs_invoke_callback_with_result(jfunc, jthis, jargv, jargc);
  jerry_release_value(result);
}

jerry_value_t iotjs_invoke_callback_with_result(jerry_value_t jfunc,
                                                jerry_value_t jthis,
                                                const jerry_value_t* jargv,
                                                size_t jargc) {
  IOTJS_ASSERT(jerry_value_is_function(jfunc));

  // If the environment is already exiting just return an undefined value.
  if (iotjs_environment_is_exiting(iotjs_environment_get())) {
    return jerry_create_undefined();
  }
  // Calls back the function.
  jerry_value_t jres = jerry_call_function(jfunc, jthis, jargv, jargc);
  if (jerry_value_is_error(jres)) {
    jerry_value_t errval = jerry_get_value_from_error(jres, false);
    iotjs_uncaught_exception(errval);
    jerry_release_value(errval);
  }

  // Calls the next tick callbacks.
  iotjs_process_next_tick();

  // Return value.
  return jres;
}


int iotjs_process_exitcode(void) {
  const jerry_value_t process = iotjs_module_get("process");

  jerry_value_t jexitcode =
      iotjs_jval_get_property(process, IOTJS_MAGIC_STRING_EXITCODE);
  uint8_t exitcode = 0;
  jerry_value_t num_val = jerry_value_to_number(jexitcode);
  if (jerry_value_is_error(num_val)) {
    exitcode = 1;
  } else {
    exitcode = (uint8_t)iotjs_jval_as_number(num_val);
  }

  uint8_t native_exitcode = iotjs_environment_get()->exitcode;
  if (native_exitcode != exitcode && native_exitcode) {
    exitcode = native_exitcode;
  }
  jerry_release_value(num_val);
  jerry_release_value(jexitcode);
  return (int)exitcode;
}


void iotjs_set_process_exitcode(int code) {
  const jerry_value_t process = iotjs_module_get("process");
  jerry_value_t jstring =
      jerry_create_string((jerry_char_t*)IOTJS_MAGIC_STRING_EXITCODE);
  jerry_value_t jcode = jerry_create_number(code);
  jerry_value_t ret_val = jerry_set_property(process, jstring, jcode);
  if (jerry_value_is_error(ret_val)) {
    iotjs_environment_get()->exitcode = 1;
  }

  jerry_release_value(ret_val);
  jerry_release_value(jstring);
  jerry_release_value(jcode);
}
