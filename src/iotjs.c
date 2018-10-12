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
#include "iotjs_context.h"

#include "iotjs.h"
#include "iotjs_js.h"
#include "iotjs_string_ext.h"

#include "jerryscript-ext/debugger.h"
#ifndef __NUTTX__
#include "jerryscript-port-default.h"
#endif
#include "jerryscript-port.h"
#include "jerryscript.h"

#include "iotjs_uv_handle.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

iotjs_context_t *iotjs_create_context(
    uint32_t heap_size,          /**< the size of heap */
    iotjs_context_alloc_t alloc, /**< the alloc function */
    void *cb_data_p)             /**< the cb_data for alloc function */
{
  size_t total_size = sizeof(iotjs_context_t) + (1u << 3);


  heap_size = IOTJS_ALIGNUP(heap_size, (1u << 3));

  /* Minimum heap size is 1Kbyte. */
  if (heap_size < 1024) {
    return NULL;
  }

  total_size += heap_size;

  total_size = IOTJS_ALIGNUP(total_size, (1u << 3));

  iotjs_context_t *context_p = (iotjs_context_t *)alloc(total_size, cb_data_p);

  if (context_p == NULL) {
    return NULL;
  }

  memset(context_p, 0, total_size);

  uintptr_t context_ptr = ((uintptr_t)context_p) + sizeof(iotjs_context_t);
  context_ptr = IOTJS_ALIGNUP(context_ptr, (uintptr_t)(1u << 3));

  return context_p;

} /* iotjs_create_context */


static bool jerry_initialize() {
  // Set jerry run flags.
  jerry_init_flag_t jerry_flags = JERRY_INIT_EMPTY;

  if (IOTJS_CONTEXT(current_env)->config.memstat) {
    jerry_flags |= JERRY_INIT_MEM_STATS;
#if !defined(__NUTTX__) && !defined(__TIZENRT__)
    jerry_port_default_set_log_level(JERRY_LOG_LEVEL_DEBUG);
#endif
  }

  if (IOTJS_CONTEXT(current_env)->config.show_opcode) {
    jerry_flags |= JERRY_INIT_SHOW_OPCODES;
#if !defined(__NUTTX__) && !defined(__TIZENRT__)
    jerry_port_default_set_log_level(JERRY_LOG_LEVEL_DEBUG);
#endif
  }
  // Initialize jerry.
  jerry_init(jerry_flags);

  if (IOTJS_CONTEXT(current_env)->config.debugger != NULL) {
    uint16_t port = IOTJS_CONTEXT(current_env)->config.debugger->port;
    jerryx_debugger_after_connect(jerryx_debugger_tcp_create(port) &&
                                  jerryx_debugger_ws_create());

    if (!jerry_debugger_is_connected()) {
      DLOG("jerry debugger connection failed");
      return false;
    }

    jerry_debugger_continue();
  }

  // Set magic strings.
  iotjs_register_jerry_magic_string();

  // Register VM execution stop callback.
  jerry_set_vm_exec_stop_callback(vm_exec_stop_callback,
                                  &IOTJS_CONTEXT(current_env)->state, 2);

  // Do parse and run to generate initial javascript environment.
  jerry_value_t parsed_code =
      jerry_parse(NULL, 0, (jerry_char_t *)"", 0, JERRY_PARSE_NO_OPTS);
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


bool iotjs_initialize() {
  // Initialize JerryScript
  if (!jerry_initialize()) {
    DLOG("iotjs_jerry_init failed");
    return false;
  }

  // Set event loop.
  if (!uv_default_loop()) {
    DLOG("iotjs uvloop init failed");
    return false;
  }
  IOTJS_CONTEXT(current_env)->loop = uv_default_loop();
  // Bind environment to global object.
  const jerry_value_t global = jerry_get_global_object();
  jerry_set_object_native_pointer(global, IOTJS_CONTEXT(current_env), NULL);

  // Initialize builtin process module.
  const jerry_value_t process = iotjs_module_get("process");
  iotjs_jval_set_property_jval(global, "process", process);

  // Release the global object
  jerry_release_value(global);
  return true;
}

void iotjs_restart(jerry_value_t jmain) {
  jerry_value_t abort_value = jerry_get_value_from_error(jmain, false);
  if (jerry_value_is_string(abort_value)) {
    /* TODO: When there is an api function to check for reset,
    this needs an update. */
    static const char restart_str[] = "r353t";

    jerry_size_t str_size = jerry_get_string_size(abort_value);

    if (str_size == sizeof(restart_str) - 1) {
      jerry_char_t str_buf[5];
      jerry_string_to_char_buffer(abort_value, str_buf, str_size);
      if (memcmp(restart_str, (char *)(str_buf), str_size) == 0) {
        IOTJS_CONTEXT(current_env)->config.debugger->context_reset = true;
      }
    }
  }
  jerry_release_value(abort_value);
}

void iotjs_run() {
// Evaluating 'iotjs.js' returns a function.
#ifndef ENABLE_SNAPSHOT
  jerry_value_t jmain = iotjs_jhelper_eval("iotjs.js", strlen("iotjs.js"),
                                           iotjs_s, iotjs_l, false);
#else
  jerry_value_t jmain =
      jerry_exec_snapshot((const uint32_t *)iotjs_js_modules_s,
                          iotjs_js_modules_l, module_iotjs_idx,
                          JERRY_SNAPSHOT_EXEC_ALLOW_STATIC);
#endif

  if (jerry_value_is_abort(jmain)) {
    iotjs_restart(jmain);
  } else if (jerry_value_is_error(jmain) &&
             !(IOTJS_CONTEXT(current_env)->state == kExiting)) {
    jerry_value_t errval = jerry_get_value_from_error(jmain, false);
    iotjs_uncaught_exception(errval);
    jerry_release_value(errval);
  }

  jerry_release_value(jmain);
}


static int iotjs_start() {
  iotjs_environment_set_state(kRunningMain);
  // Load and call iotjs.js.
  iotjs_run();

  int exit_code = 0;
  if (!(IOTJS_CONTEXT(current_env)->state == kExiting)) {
    // Run event loop.
    iotjs_environment_set_state(kRunningLoop);

    bool more;
    do {
      more = uv_run(IOTJS_CONTEXT(current_env)->loop, UV_RUN_ONCE);
      more |= iotjs_process_next_tick();

      jerry_value_t ret_val = jerry_run_all_enqueued_jobs();
      if (jerry_value_is_error(ret_val)) {
        DLOG("jerry_run_all_enqueued_jobs() failed");
      }

      if (more == false) {
        more = uv_loop_alive(IOTJS_CONTEXT(current_env)->loop);
      }
    } while (more && !(IOTJS_CONTEXT(current_env)->state == kExiting));

    exit_code = iotjs_process_exitcode();

    if (!(IOTJS_CONTEXT(current_env)->state == kExiting)) {
      // Emit 'exit' event.
      iotjs_process_emit_exit(exit_code);

      iotjs_environment_set_state(kExiting);
    }
  }

  exit_code = iotjs_process_exitcode();

  return exit_code;
}


void iotjs_end() {
  // Close uv loop.
  uv_walk(IOTJS_CONTEXT(current_env)->loop, (uv_walk_cb)iotjs_uv_handle_close,
          NULL);
  uv_run(IOTJS_CONTEXT(current_env)->loop, UV_RUN_DEFAULT);

  int res = uv_loop_close(IOTJS_CONTEXT(current_env)->loop);
  IOTJS_ASSERT(res == 0);
}


void iotjs_terminate() {
  // Release builtin modules.
  iotjs_module_list_cleanup();

  // Release JerryScript engine.
  jerry_cleanup();
}


void iotjs_conf_console_out(int (*out)(int lv, const char *fmt, ...)) {
  iotjs_set_console_out(out);
}
#ifdef IOTJS_ENABLE_EXTERNAL_CONTEXT

/**
 * The alloc function passed to iotjs_create_context
 */
static void *context_alloc(size_t size, void *cb_data_p) {
  (void)cb_data_p; /* unused */
  return malloc(size);
} /* context_alloc */

#endif /* IOTJS_ENABLE_EXTERNAL_CONTEXT */

int iotjs_entry(int argc, char **argv) {
  int ret_code = 0;
  // Initialize debug log and environments
  iotjs_debuglog_init();
  srand((unsigned)jerry_port_get_current_time());

#ifdef IOTJS_ENABLE_EXTERNAL_CONTEXT
  iotjs_context_t *context_p =
      iotjs_create_context(512 * 1024, context_alloc, NULL);
  iotjs_port_default_set_context(context_p);

#endif /* IOTJS_ENABLE_EXTERNAL_CONTEXT */
  IOTJS_CONTEXT(initialized) = false;
  iotjs_environment_initialize();

  if (!iotjs_environment_parse_command_line_arguments(IOTJS_CONTEXT(
                                                          current_env),
                                                      (uint32_t)argc, argv)) {
    ret_code = 1;
    goto exit;
  }

  // Initialize IoT.js
  if (!iotjs_initialize()) {
    DLOG("iotjs_initialize failed");
    ret_code = 1;
    goto terminate;
  }

  // Start IoT.js
  ret_code = iotjs_start();

  // Ends IoT.js
  iotjs_end();

terminate:
  iotjs_terminate();

exit:
  if (IOTJS_CONTEXT(current_env)->config.debugger &&
      IOTJS_CONTEXT(current_env)->config.debugger->context_reset) {
    iotjs_environment_release();
    iotjs_debuglog_release();

    return iotjs_entry(argc, argv);
  }

  iotjs_environment_release();
  iotjs_debuglog_release();
#ifdef IOTJS_ENABLE_EXTERNAL_CONTEXT
  IOTJS_RELEASE(context_p);
#endif /* IOTJS_ENABLE_EXTERNAL_CONTEXT */
  return ret_code;
}
