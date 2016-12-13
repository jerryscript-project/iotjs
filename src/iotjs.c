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

#include "iotjs.h"
#include "iotjs_handlewrap.h"
#include "iotjs_js.h"
#include "iotjs_string_ext.h"

#include "jerry-api.h"
#include "jerry-port-default.h"
#include "jerry-port.h"

#include <stdio.h>
#include <string.h>


/**
 * Initialize JerryScript.
 */
static bool iotjs_jerry_initialize(const iotjs_environment_t* env) {
  // Set jerry run flags.
  uint32_t jerry_flag = JERRY_INIT_EMPTY;

  if (iotjs_environment_config(env)->memstat) {
    jerry_flag |= JERRY_INIT_MEM_STATS;
    jerry_port_default_set_log_level(JERRY_LOG_LEVEL_DEBUG);
  }

  if (iotjs_environment_config(env)->show_opcode) {
    jerry_flag |= JERRY_INIT_SHOW_OPCODES;
    jerry_port_default_set_log_level(JERRY_LOG_LEVEL_DEBUG);
  }

  // Initialize jerry.
  jerry_init((jerry_init_flag_t)jerry_flag);

  // Set magic strings.
  iotjs_register_jerry_magic_string();

  // Do parse and run to generate initial javascript environment.
  jerry_value_t parsed_code = jerry_parse((jerry_char_t*)"", 0, false);
  if (jerry_value_has_error_flag(parsed_code)) {
    DLOG("jerry_parse() failed");
    jerry_release_value(parsed_code);
    return false;
  }

  jerry_value_t ret_val = jerry_run(parsed_code);
  if (jerry_value_has_error_flag(ret_val)) {
    DLOG("jerry_run() failed");
    jerry_release_value(parsed_code);
    jerry_release_value(ret_val);
    return false;
  }

  jerry_release_value(parsed_code);
  jerry_release_value(ret_val);
  return true;
}


static void iotjs_jerry_release() {
  jerry_cleanup();
}


static bool iotjs_run(const iotjs_jval_t* process) {
  // Evaluating 'iotjs.js' returns a function.
  bool throws;
#ifndef ENABLE_SNAPSHOT
  iotjs_jval_t jmain = iotjs_jhelper_eval(iotjs_s, iotjs_l, false, &throws);
#else
  iotjs_jval_t jmain = iotjs_jhelper_exec_snapshot(iotjs_s, iotjs_l, &throws);
#endif
  IOTJS_ASSERT(!throws);

  // Run the entry function passing process builtin.
  // The entry function will continue initializing process module, global, and
  // other native modules, and finally load and run application.
  iotjs_jargs_t args = iotjs_jargs_create(1);
  iotjs_jargs_append_jval(&args, process);

  const iotjs_jval_t* global = iotjs_jval_get_global_object();
  iotjs_jval_t jmain_res = iotjs_jhelper_call(&jmain, global, &args, &throws);

  iotjs_jargs_destroy(&args);
  iotjs_jval_destroy(&jmain);

  if (throws) {
    iotjs_uncaught_exception(&jmain_res);
  }
  iotjs_jval_destroy(&jmain_res);

  return !throws;
}


static bool iotjs_start(iotjs_environment_t* env) {
  // Initialize commonly used jerry values
  iotjs_binding_initialize();

  // Bind environment to global object.
  const iotjs_jval_t* global = iotjs_jval_get_global_object();
  iotjs_jval_set_object_native_handle(global, (uintptr_t)(env), NULL);

  // Initialize builtin modules.
  iotjs_module_list_init();

  // Initialize builtin process module.
  const iotjs_jval_t* process =
      iotjs_module_initialize_if_necessary(MODULE_PROCESS);

  // Call the entry.
  // load and call iotjs.js
  iotjs_environment_go_state_running_main(env);

  iotjs_run(process);

  // Run event loop.
  iotjs_environment_go_state_running_loop(env);

  bool more;
  do {
    more = uv_run(iotjs_environment_loop(env), UV_RUN_ONCE);
    more |= iotjs_process_next_tick();
    if (more == false) {
      more = uv_loop_alive(iotjs_environment_loop(env));
    }
  } while (more);

  iotjs_environment_go_state_exiting(env);

  // Emit 'exit' event.
  iotjs_process_emit_exit(0);

  // Release builtin modules.
  iotjs_module_list_cleanup();

  // Release commonly used jerry values.
  iotjs_binding_finalize();

  return true;
}


static void iotjs_uv_walk_to_close_callback(uv_handle_t* handle, void* arg) {
  iotjs_handlewrap_t* handle_wrap = iotjs_handlewrap_from_handle(handle);
  IOTJS_ASSERT(handle_wrap != NULL);

  iotjs_handlewrap_close(handle_wrap, NULL);
}


int iotjs_entry(int argc, char** argv) {
  // Initialize debug print.
  init_debug_settings();

  // Create environment.
  iotjs_environment_t* env = (iotjs_environment_t*)iotjs_environment_get();

  // Parse command line arguments.
  if (!iotjs_environment_parse_command_line_arguments(env, argc, argv)) {
    DLOG("iotjs_environment_parse_command_line_arguments failed");
    return 1;
  }

  // Set event loop.
  iotjs_environment_set_loop(env, uv_default_loop());

  // Initialize JerryScript engine.
  if (!iotjs_jerry_initialize(env)) {
    DLOG("iotjs_jerry_initialize failed");
    return 1;
  }

  // Start IoT.js
  if (!iotjs_start(env)) {
    DLOG("iotjs_start failed");
    return 1;
  }

  // close uv loop.
  // uv_stop(iotjs_environment_loop(env));
  uv_walk(iotjs_environment_loop(env), iotjs_uv_walk_to_close_callback, NULL);
  uv_run(iotjs_environment_loop(env), UV_RUN_DEFAULT);

  int res = uv_loop_close(iotjs_environment_loop(env));
  IOTJS_ASSERT(res == 0);

  // Release JerryScript engine.
  iotjs_jerry_release();

  // Release environment.
  iotjs_environment_release();

  // Release debug print setting.
  release_debug_settings();

  return 0;
}
