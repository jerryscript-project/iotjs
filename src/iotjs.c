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

#include "jerryscript-debugger.h"
#ifndef __NUTTX__
#include "jerryscript-port-default.h"
#endif
#include "jerryscript-port.h"
#include "jerryscript.h"

#include <stdio.h>
#include <string.h>


/**
 * Initialize JerryScript.
 */
static bool iotjs_jerry_initialize(iotjs_environment_t* env) {
  // Set jerry run flags.
  jerry_init_flag_t jerry_flags = JERRY_INIT_EMPTY;

  if (iotjs_environment_config(env)->memstat) {
    jerry_flags |= JERRY_INIT_MEM_STATS;
#if !defined(__NUTTX__) && !defined(__TIZENRT__)
    jerry_port_default_set_log_level(JERRY_LOG_LEVEL_DEBUG);
#endif
  }

  if (iotjs_environment_config(env)->show_opcode) {
    jerry_flags |= JERRY_INIT_SHOW_OPCODES;
#if !defined(__NUTTX__) && !defined(__TIZENRT__)
    jerry_port_default_set_log_level(JERRY_LOG_LEVEL_DEBUG);
#endif
  }
  // Initialize jerry.
  jerry_init(jerry_flags);

  if (iotjs_environment_config(env)->debugger) {
    jerry_debugger_init(iotjs_environment_config(env)->debugger_port);
  }

  if (iotjs_environment_config(env)->debugger) {
    jerry_debugger_continue();
  }

  // Set magic strings.
  iotjs_register_jerry_magic_string();

  // Register VM execution stop callback.
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_environment_t, env);
  jerry_set_vm_exec_stop_callback(vm_exec_stop_callback, &(_this->state), 2);

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


static void iotjs_jerry_release(iotjs_environment_t* env) {
  jerry_cleanup();
}


static bool iotjs_run(iotjs_environment_t* env) {
  // Evaluating 'iotjs.js' returns a function.
  bool throws = false;
#ifndef ENABLE_SNAPSHOT
  iotjs_jval_t jmain = iotjs_jhelper_eval("iotjs.js", strlen("iotjs.js"),
                                          iotjs_s, iotjs_l, false, &throws);
#else
  iotjs_jval_t jmain = iotjs_jhelper_exec_snapshot(iotjs_s, iotjs_l, &throws);
#endif

  if (throws) {
    iotjs_uncaught_exception(&jmain);
  }

  iotjs_jval_destroy(&jmain);

  return !throws;
}


static int iotjs_start(iotjs_environment_t* env) {
  // Initialize commonly used jerry values.
  iotjs_binding_initialize();

  // Bind environment to global object.
  const iotjs_jval_t* global = iotjs_jval_get_global_object();
  iotjs_jval_set_object_native_handle(global, (uintptr_t)(env), NULL);

  // Initialize builtin modules.
  iotjs_module_list_init();

  // Initialize builtin process module.
  const iotjs_jval_t* process = iotjs_init_process_module();
  iotjs_jval_set_property_jval(global, "process", process);

  // Set running state.
  iotjs_environment_go_state_running_main(env);

  // Load and call iotjs.js.
  iotjs_run(env);

  int exit_code = 0;
  if (!iotjs_environment_is_exiting(env)) {
    // Run event loop.
    iotjs_environment_go_state_running_loop(env);

    bool more;
    do {
      more = uv_run(iotjs_environment_loop(env), UV_RUN_ONCE);
      more |= iotjs_process_next_tick();
      if (more == false) {
        more = uv_loop_alive(iotjs_environment_loop(env));
      }
      jerry_value_t ret_val = jerry_run_all_enqueued_jobs();
      if (jerry_value_has_error_flag(ret_val)) {
        DLOG("jerry_run_all_enqueued_jobs() failed");
      }
    } while (more && !iotjs_environment_is_exiting(env));

    exit_code = iotjs_process_exitcode();

    if (!iotjs_environment_is_exiting(env)) {
      // Emit 'exit' event.
      iotjs_process_emit_exit(exit_code);

      iotjs_environment_go_state_exiting(env);
    }
  }

  exit_code = iotjs_process_exitcode();

  // Release builtin modules.
  iotjs_module_list_cleanup();

  return exit_code;
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

  int ret_code = 0;

  // Parse command line arguments.
  if (!iotjs_environment_parse_command_line_arguments(env, (uint32_t)argc,
                                                      argv)) {
    DLOG("iotjs_environment_parse_command_line_arguments failed");
    ret_code = 1;
    goto terminate;
  }

  // Initialize JerryScript engine.
  if (!iotjs_jerry_initialize(env)) {
    DLOG("iotjs_jerry_initialize failed");
    ret_code = 1;
    goto terminate;
  }

  // Set event loop.
  iotjs_environment_set_loop(env, uv_default_loop());

  // Start iot.js.
  ret_code = iotjs_start(env);

  // Close uv loop.
  uv_walk(iotjs_environment_loop(env), iotjs_uv_walk_to_close_callback, NULL);
  uv_run(iotjs_environment_loop(env), UV_RUN_DEFAULT);

  int res = uv_loop_close(iotjs_environment_loop(env));
  IOTJS_ASSERT(res == 0);

  // Release commonly used jerry values.
  iotjs_binding_finalize();

  // Release JerryScript engine.
  iotjs_jerry_release(env);

terminate:
  // Release environment.
  iotjs_environment_release();

  // Release debug print setting.
  release_debug_settings();

  return ret_code;
}
