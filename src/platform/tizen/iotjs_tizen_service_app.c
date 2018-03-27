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
#include <service_app_internal.h>
#include <stdio.h>

#include "iotjs_def.h"
#include "iotjs.h"

extern bool iotjs_jerry_init(iotjs_environment_t* env);
extern int iotjs_start(iotjs_environment_t* env);
extern void iotjs_uv_walk_to_close_callback(uv_handle_t* handle, void* arg);

static char js_absolute_path[128];

static int console_log(int level, const char* format, ...) {
  va_list args;
  va_start(args, format);
  dlog_vprint(DLOG_INFO, "IOTJS", format, args);
  va_end(args);
  return 0;
}

static void loop_method_init_cb(int argc, char** argv, void* data) {
  int iotjs_argc = 2;
  char* iotjs_argv[2] = { "iotjs", js_absolute_path };

  iotjs_debuglog_init();

  iotjs_environment_t* env = iotjs_environment_get();
  if (!iotjs_environment_parse_command_line_arguments(env, (uint32_t)iotjs_argc,
                                                      iotjs_argv)) {
    DLOG("iotjs_environment_parse_command_line_arguments failed");
    service_app_exit();
    return;
  }

  if (!iotjs_jerry_init(env)) {
    DLOG("iotjs_jerry_init failed");
    service_app_exit();
    return;
  }

  iotjs_conf_console_out(console_log);
}

static void loop_method_run_cb(void* data) {
  iotjs_environment_t* env = iotjs_environment_get();

  // Set event loop.
  iotjs_environment_set_loop(env, uv_default_loop());

  // Start IoT.js.
  iotjs_start(env);

  service_app_exit();
}

static void loop_method_exit_cb(void* data) {
  iotjs_environment_t* env = iotjs_environment_get();

  // Close uv loop.
  uv_walk(iotjs_environment_loop(env), iotjs_uv_walk_to_close_callback, NULL);
  uv_run(iotjs_environment_loop(env), UV_RUN_DEFAULT);

  int res = uv_loop_close(iotjs_environment_loop(env));
  IOTJS_ASSERT(res == 0);

  // Release builtin modules.
  iotjs_module_list_cleanup();
}

static void loop_method_fini_cb(void) {
  // Release JerryScript engine.
  jerry_cleanup();

  // Release environment.
  iotjs_environment_release();

  iotjs_debuglog_release();
}

int iotjs_service_app_start(int argc, char** argv, char* js_path,
                            void* event_callbacks, void* user_data) {
  char* app_res_path = app_get_resource_path();
  if (!app_res_path) {
    DLOG("app_res_path is NULL!");
    return 1;
  }

  snprintf(js_absolute_path, sizeof(js_absolute_path), "%s%s", app_res_path,
           js_path);
  IOTJS_RELEASE(app_res_path);

  service_app_loop_method_s loop_method = {.init = loop_method_init_cb,
                                           .run = loop_method_run_cb,
                                           .exit = loop_method_exit_cb,
                                           .fini = loop_method_fini_cb };

  return service_app_main_ext(argc, argv, (service_app_lifecycle_callback_s*)
                                              event_callbacks,
                              &loop_method, user_data);
}
