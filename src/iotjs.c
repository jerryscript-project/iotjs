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
#include "iotjs_js.h"
#include "iotjs_string_ext.h"
#include "jerryscript-ext/debugger.h"
#if ENABLE_MODULE_NAPI
#include "internal/node_api_internal.h"
#endif
#if !defined(__NUTTX__) && !defined(__TIZENRT__)
#include "jerryscript-port-default.h"
#endif
#include "jerryscript-port.h"
#include "jerryscript.h"

#include "iotjs_uv_handle.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static bool jerry_initialize(iotjs_environment_t* env) {
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

#ifdef JERRY_DEBUGGER
  if (iotjs_environment_config(env)->debugger != NULL) {
    bool protocol_created = false;
    char* debug_protocol = iotjs_environment_config(env)->debugger->protocol;
    char* debug_channel = iotjs_environment_config(env)->debugger->channel;

    if (!strcmp(debug_protocol, "tcp")) {
      uint16_t port = iotjs_environment_config(env)->debugger->port;
      protocol_created = jerryx_debugger_tcp_create(port);
    } else {
      IOTJS_ASSERT(!strcmp(debug_protocol, "serial"));
      char* config = iotjs_environment_config(env)->debugger->serial_config;
      protocol_created = jerryx_debugger_serial_create(config);
    }

    if (!strcmp(debug_channel, "rawpacket")) {
      jerryx_debugger_after_connect(protocol_created &&
                                    jerryx_debugger_rp_create());
    } else {
      IOTJS_ASSERT(!strcmp(debug_channel, "websocket"));
      jerryx_debugger_after_connect(protocol_created &&
                                    jerryx_debugger_ws_create());
    }

    if (!jerry_debugger_is_connected()) {
      DLOG("jerry debugger connection failed");
      return false;
    }

    jerry_debugger_continue();
  }
#endif

  // Set magic strings.
  iotjs_register_jerry_magic_string();

  // Register VM execution stop callback.
  jerry_set_vm_exec_stop_callback(vm_exec_stop_callback, &env->state, 2);

  // Do parse and run to generate initial javascript environment.
  jerry_value_t parsed_code =
      jerry_parse(NULL, 0, (jerry_char_t*)"", 0, JERRY_PARSE_NO_OPTS);
  if (jerry_value_is_error(parsed_code)) {
    DLOG("jerry_parse() failed");
    jerry_release_value(parsed_code);
    return false;
  }

  jerry_value_t ret_val = jerry_run(parsed_code);
  if (jerry_value_is_error(ret_val)) {
    DLOG("jerry_run() failed");
    jerry_release_value(parsed_code);
    jerry_release_value(ret_val);
    return false;
  }

  jerry_release_value(parsed_code);
  jerry_release_value(ret_val);
  return true;
}


bool iotjs_initialize(iotjs_environment_t* env) {
  // Initialize JerryScript
  if (!jerry_initialize(env)) {
    DLOG("iotjs_jerry_init failed");
    return false;
  }

  // Set event loop.
  if (!uv_default_loop()) {
    DLOG("iotjs uvloop init failed");
    return false;
  }
  iotjs_environment_set_loop(env, uv_default_loop());

  // Bind environment to global object.
  const jerry_value_t global = jerry_get_global_object();
  jerry_set_object_native_pointer(global, env, NULL);

  // Initialize builtin process module.
  const jerry_value_t process = iotjs_module_get("process");
  iotjs_jval_set_property_jval(global, "process", process);

  // Release the global object
  jerry_release_value(global);

  return true;
}


#ifdef JERRY_DEBUGGER
void iotjs_restart(iotjs_environment_t* env, jerry_value_t jmain) {
  jerry_value_t abort_value = jerry_get_value_from_error(jmain, false);
  if (jerry_value_is_string(abort_value)) {
    /* TODO: When there is an api function to check for reset,
    this needs an update. */
    static const char restart_str[] = "r353t";

    jerry_size_t str_size = jerry_get_string_size(abort_value);

    if (str_size == sizeof(restart_str) - 1) {
      jerry_char_t str_buf[5];
      jerry_string_to_char_buffer(abort_value, str_buf, str_size);
      if (memcmp(restart_str, (char*)(str_buf), str_size) == 0) {
        iotjs_environment_config(env)->debugger->context_reset = true;
      }
    }
  }
  jerry_release_value(abort_value);
}
#endif


void iotjs_run(iotjs_environment_t* env) {
// Evaluating 'iotjs.js' returns a function.
#ifndef ENABLE_SNAPSHOT
  jerry_value_t jmain = iotjs_jhelper_eval("iotjs.js", strlen("iotjs.js"),
                                           iotjs_s, iotjs_l, false);
#else
  jerry_value_t jmain =
      jerry_exec_snapshot((const uint32_t*)iotjs_js_modules_s,
                          iotjs_js_modules_l, module_iotjs_idx,
                          JERRY_SNAPSHOT_EXEC_ALLOW_STATIC);
#endif

  if (jerry_value_is_error(jmain)) {
    jerry_value_t errval = jerry_get_value_from_error(jmain, false);
#ifdef JERRY_DEBUGGER
    if (jerry_value_is_abort(jmain) && jerry_value_is_string(errval)) {
      static const char restart_str[] = "r353t";
      jerry_size_t str_size = jerry_get_string_size(errval);

      if (str_size == sizeof(restart_str) - 1) {
        jerry_char_t str_buf[5];
        jerry_string_to_char_buffer(errval, str_buf, str_size);
        if (memcmp(restart_str, (char*)(str_buf), str_size) == 0) {
          iotjs_environment_config(env)->debugger->context_reset = true;
        }
      }
    }
#endif

    bool throw_exception = !iotjs_environment_is_exiting(env);
#ifdef JERRY_DEBUGGER
    throw_exception = throw_exception &&
                      iotjs_environment_config(env)->debugger &&
                      !iotjs_environment_config(env)->debugger->context_reset;
#endif
    if (throw_exception) {
      iotjs_uncaught_exception(errval);
    }

    jerry_release_value(errval);
  }

  jerry_release_value(jmain);
}


static int iotjs_start(iotjs_environment_t* env) {
  iotjs_environment_set_state(env, kRunningMain);
#if ENABLE_MODULE_NAPI
  iotjs_setup_napi();
#endif
  // Load and call iotjs.js.
  iotjs_run(env);

  int exit_code = 0;
  if (!iotjs_environment_is_exiting(env)) {
    // Run event loop.
    iotjs_environment_set_state(env, kRunningLoop);

    bool more;
    do {
      more = uv_run(iotjs_environment_loop(env), UV_RUN_ONCE);
      more |= iotjs_process_next_tick();

      jerry_value_t ret_val = jerry_run_all_enqueued_jobs();
      if (jerry_value_is_error(ret_val)) {
        ret_val = jerry_get_value_from_error(ret_val, true);
        iotjs_uncaught_exception(ret_val);
        jerry_release_value(ret_val);
      }

      if (more == false) {
        more = uv_loop_alive(iotjs_environment_loop(env));
      }
    } while (more && !iotjs_environment_is_exiting(env));

    exit_code = iotjs_process_exitcode();

    if (!iotjs_environment_is_exiting(env)) {
      // Emit 'exit' event.
      iotjs_process_emit_exit(exit_code);

      iotjs_environment_set_state(env, kExiting);
    }
  }

  exit_code = iotjs_process_exitcode();

  return exit_code;
}


void iotjs_end(iotjs_environment_t* env) {
  uv_loop_t* loop = iotjs_environment_loop(env);
  // Close uv loop.
  uv_walk(loop, (uv_walk_cb)iotjs_uv_handle_close, NULL);
  uv_run(loop, UV_RUN_DEFAULT);

  int res = uv_loop_close(loop);
  IOTJS_ASSERT(res == 0);
}


void iotjs_terminate(iotjs_environment_t* env) {
  // Release builtin modules.
  iotjs_module_list_cleanup();
#if ENABLE_MODULE_NAPI
  iotjs_cleanup_napi();
#endif
  // Release JerryScript engine.
  jerry_cleanup();
}


void iotjs_conf_console_out(int (*out)(int lv, const char* fmt, ...)) {
  iotjs_set_console_out(out);
}

int iotjs_entry(int argc, char** argv) {
  int ret_code = 0;

  // Initialize debug log and environments
  iotjs_debuglog_init();
  srand((unsigned)jerry_port_get_current_time());

  iotjs_environment_t* env = iotjs_environment_get();
  if (!iotjs_environment_parse_command_line_arguments(env, (uint32_t)argc,
                                                      argv)) {
    ret_code = 1;
    goto exit;
  }

  // Initialize IoT.js
  if (!iotjs_initialize(env)) {
    DLOG("iotjs_initialize failed");
    ret_code = 1;
    goto terminate;
  }

  // Start IoT.js
  ret_code = iotjs_start(env);

  // Ends IoT.js
  iotjs_end(env);

terminate:
  iotjs_terminate(env);

exit:
#ifdef JERRY_DEBUGGER
  if (iotjs_environment_config(env)->debugger &&
      iotjs_environment_config(env)->debugger->context_reset) {
    iotjs_environment_release();
    iotjs_debuglog_release();

    return iotjs_entry(argc, argv);
  }
#endif

  iotjs_environment_release();
  iotjs_debuglog_release();
  return ret_code;
}
