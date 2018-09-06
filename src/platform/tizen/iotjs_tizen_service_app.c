/* Copyright 2018-present Samsung Electronics Co., Ltd. and other contributors
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

#include <dlog.h>
#include <glib.h>
#include <service_app_internal.h>
#include <stdio.h>
#include <stdlib.h>

#include "iotjs_def.h"
#include "iotjs.h"

extern bool iotjs_initialize(iotjs_environment_t* env);
extern void iotjs_run(iotjs_environment_t* env);
extern void iotjs_end(iotjs_environment_t* env);
extern void iotjs_terminate(iotjs_environment_t* env);

static char js_absolute_path[128];
static GMainLoop* gmain_loop;
static bool is_env_initialized = false;

typedef struct {
  GSource source;
  iotjs_environment_t* env;
} iotjs_gmain_source_t;

static int console_log(int level, const char* format, ...) {
  va_list args;
  va_start(args, format);
  dlog_vprint(DLOG_INFO, "IOTJS", format, args);
  va_end(args);
  return 0;
}

static gboolean gmain_loop_check(GSource* source) {
  return TRUE;
}

static gboolean gmain_loop_dispatch(GSource* source, GSourceFunc callback,
                                    gpointer user_data) {
  iotjs_environment_t* env = ((iotjs_gmain_source_t*)source)->env;

  bool more = uv_run(iotjs_environment_loop(env), UV_RUN_NOWAIT);
  more |= iotjs_process_next_tick();

  jerry_value_t ret_val = jerry_run_all_enqueued_jobs();
  if (jerry_value_is_error(ret_val)) {
    DLOG("jerry_run_all_enqueued_jobs() failed");
  }

  if (more == false) {
    more = uv_loop_alive(iotjs_environment_loop(env));
  }

  if (!more || iotjs_environment_is_exiting(env)) {
    service_app_exit();
    return false;
  }
  return true;
}

static void loop_method_init_cb(int argc, char** argv, void* data) {
  int iotjs_argc = 2;
  char* iotjs_argv[2] = { "iotjs", js_absolute_path };

#ifdef ENABLE_DEBUG_LOG
  setenv("IOTJS_DEBUG_LEVEL", "3", 0); // Enable all log.
#endif

  // Initialize debug log and environments
  iotjs_debuglog_init();

  iotjs_environment_t* env = iotjs_environment_get();

  if (!iotjs_environment_parse_command_line_arguments(env, (uint32_t)iotjs_argc,
                                                      iotjs_argv)) {
    service_app_exit();
    return;
  }
  is_env_initialized = true;

  if (!iotjs_initialize(env)) {
    DLOG("iotjs_initialize failed");
    service_app_exit();
    return;
  }

  DDDLOG("%s", __func__);

  iotjs_conf_console_out(console_log);
}

static void loop_method_run_cb(void* data) {
  DDDLOG("%s", __func__);
  iotjs_environment_t* env = iotjs_environment_get();
  iotjs_environment_set_state(env, kRunningMain);

  // Load and call iotjs.js.
  iotjs_run(env);

  if (iotjs_environment_is_exiting(env)) {
    service_app_exit();
    return;
  }

  // Create GMain loop.
  gmain_loop = g_main_loop_new(g_main_context_default(), FALSE);

  // Add GSource in GMain context.
  GSourceFuncs source_funcs = {
    .check = gmain_loop_check, .dispatch = gmain_loop_dispatch,
  };

  iotjs_gmain_source_t* source =
      (iotjs_gmain_source_t*)g_source_new(&source_funcs,
                                          sizeof(iotjs_gmain_source_t));
  source->env = env;
  uv_loop_t* uv_loop = iotjs_environment_loop(env);
  g_source_add_unix_fd(&source->source, uv_loop->backend_fd,
                       (GIOCondition)(G_IO_IN | G_IO_OUT | G_IO_ERR));
  g_source_attach(&source->source, g_main_context_default());

  iotjs_environment_set_state(env, kRunningLoop);

  g_main_loop_run(gmain_loop); // Blocks until loop is quit.


  if (!iotjs_environment_is_exiting(env)) {
    // Emit 'exit' event.
    iotjs_process_emit_exit(iotjs_process_exitcode());

    iotjs_environment_set_state(env, kExiting);
  }

  DDDLOG("%s: Exit IoT.js(%d).", __func__, iotjs_process_exitcode());

  iotjs_end(env);
}

static void loop_method_exit_cb(void* data) {
  DDDLOG("%s", __func__);

  if (g_main_loop_is_running(gmain_loop)) {
    g_main_loop_quit(gmain_loop);
    g_main_loop_unref(gmain_loop);
  }
}

static void loop_method_fini_cb(void) {
  DDDLOG("%s", __func__);
  iotjs_environment_t* env = iotjs_environment_get();

  if (is_env_initialized) {
    iotjs_terminate(env);
  }

  iotjs_environment_release();
  iotjs_debuglog_release();
}

int iotjs_service_app_start(int argc, char** argv, char* js_path,
                            void* event_callbacks, void* user_data) {
  DDDLOG("%s", __func__);
  char* app_res_path = app_get_resource_path();
  if (!app_res_path) {
    DLOG("app_res_path is NULL!");
    return 1;
  }

  // The JavaScript entry file is located in application res directory.
  snprintf(js_absolute_path, sizeof(js_absolute_path), "%s%s", app_res_path,
           js_path);
  setenv(IOTJS_MAGIC_STRING_IOTJS_WORKING_DIR_PATH_U, app_res_path, 1);

  IOTJS_RELEASE(app_res_path);

  service_app_loop_method_s loop_method = {.init = loop_method_init_cb,
                                           .run = loop_method_run_cb,
                                           .exit = loop_method_exit_cb,
                                           .fini = loop_method_fini_cb };

  return service_app_main_ext(argc, argv, (service_app_lifecycle_callback_s*)
                                              event_callbacks,
                              &loop_method, user_data);
}
