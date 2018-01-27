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
#include "iotjs_compatibility.h"
#include "iotjs_js.h"
#include "jerryscript-debugger.h"

#include <stdlib.h>


static jerry_value_t WrapEval(const char* name, size_t name_len,
                              const char* source, size_t length) {
  static const char* args = "exports, require, module, native";
  jerry_value_t res =
      jerry_parse_function((const jerry_char_t*)name, name_len,
                           (const jerry_char_t*)args, strlen(args),
                           (const jerry_char_t*)source, length,
                           JERRY_PARSE_NO_OPTS);

  return res;
}


JS_FUNCTION(Compile) {
  DJS_CHECK_ARGS(2, string, string);

  iotjs_string_t file = JS_GET_ARG(0, string);
  iotjs_string_t source = JS_GET_ARG(1, string);

  const char* filename = iotjs_string_data(&file);
  const iotjs_environment_t* env = iotjs_environment_get();

  if (iotjs_environment_config(env)->debugger != NULL) {
    jerry_debugger_stop();
  }

  jerry_value_t jres =
      WrapEval(filename, strlen(filename), iotjs_string_data(&source),
               iotjs_string_size(&source));

  iotjs_string_destroy(&file);
  iotjs_string_destroy(&source);

  return jres;
}


// Callback function for DebuggerGetSource
static jerry_value_t wait_for_source_callback(
    const jerry_char_t* resource_name_p, size_t resource_name_size,
    const jerry_char_t* source_p, size_t size, void* data) {
  IOTJS_UNUSED(data);

  jerry_value_t ret_val = jerry_create_array(2);
  jerry_value_t jname =
      jerry_create_string_sz(resource_name_p, resource_name_size);
  jerry_value_t jsource = jerry_create_string_sz(source_p, size);
  jerry_set_property_by_index(ret_val, 0, jname);
  jerry_set_property_by_index(ret_val, 1, jsource);

  jerry_release_value(jname);
  jerry_release_value(jsource);

  return ret_val;
}


// Export JS module received from the debugger client
JS_FUNCTION(DebuggerGetSource) {
  jerry_debugger_wait_for_source_status_t receive_status;
  jerry_value_t ret_val = jerry_create_array(0);
  uint8_t counter = 0;
  do {
    jerry_value_t res;
    receive_status =
        jerry_debugger_wait_for_client_source(wait_for_source_callback, NULL,
                                              &res);

    if (receive_status == JERRY_DEBUGGER_SOURCE_RECEIVED) {
      jerry_set_property_by_index(ret_val, counter++, res);
      jerry_release_value(res);
    }

    if (receive_status == JERRY_DEBUGGER_CONTEXT_RESET_RECEIVED) {
      iotjs_environment_config(iotjs_environment_get())
          ->debugger->context_reset = true;
      break;
    }
  } while (receive_status != JERRY_DEBUGGER_SOURCE_END);

  return ret_val;
}


JS_FUNCTION(CompileModule) {
  DJS_CHECK_ARGS(2, object, function);

  jerry_value_t jmodule = JS_GET_ARG(0, object);
  jerry_value_t jrequire = JS_GET_ARG(1, function);

  jerry_value_t jid = iotjs_jval_get_property(jmodule, "id");
  if (!jerry_value_is_string(jid)) {
    return JS_CREATE_ERROR(COMMON, "Unknown native module");
  }
  iotjs_string_t id = iotjs_jval_as_string(jid);
  jerry_release_value(jid);
  const char* name = iotjs_string_data(&id);

  int i = 0;
  while (js_modules[i].name != NULL) {
    if (!strcmp(js_modules[i].name, name)) {
      break;
    }

    i++;
  }

  jerry_value_t native_module_jval = iotjs_module_get(name);
  if (jerry_value_is_error(native_module_jval)) {
    return native_module_jval;
  }

  jerry_value_t jexports = iotjs_jval_get_property(jmodule, "exports");
  jerry_value_t jres = jerry_create_undefined();

  if (js_modules[i].name != NULL) {
#ifdef ENABLE_SNAPSHOT
    jres = jerry_exec_snapshot((const uint32_t*)iotjs_js_modules_s,
                               iotjs_js_modules_l, js_modules[i].idx, 0);
#else
    jres = WrapEval(name, iotjs_string_size(&id),
                    (const char*)js_modules[i].code, js_modules[i].length);
#endif

    if (!jerry_value_is_error(jres)) {
      jerry_value_t args[] = { jexports, jrequire, jmodule,
                               native_module_jval };

      jerry_value_t jfunc = jres;
      jres = jerry_call_function(jfunc, jerry_create_undefined(), args,
                                 sizeof(args) / sizeof(jerry_value_t));
      jerry_release_value(jfunc);
    }
  } else if (!jerry_value_is_undefined(native_module_jval)) {
    iotjs_jval_set_property_jval(jmodule, "exports", native_module_jval);
  } else {
    jres = iotjs_jval_create_error_without_error_flag("Unknown native module");
  }

  jerry_release_value(jexports);
  iotjs_string_destroy(&id);
  return jres;
}


JS_FUNCTION(ReadSource) {
  DJS_CHECK_ARGS(1, string);

  iotjs_string_t path = JS_GET_ARG(0, string);
  const iotjs_environment_t* env = iotjs_environment_get();
  int err;
  uv_fs_t fs_req;
  err = uv_fs_stat(iotjs_environment_loop(env), &fs_req,
                   iotjs_string_data(&path), NULL);
  uv_fs_req_cleanup(&fs_req);

  if (err || !S_ISREG(fs_req.statbuf.st_mode)) {
    iotjs_string_destroy(&path);
    return JS_CREATE_ERROR(COMMON, "ReadSource error, not a regular file");
  }

  iotjs_string_t code = iotjs_file_read(iotjs_string_data(&path));

  jerry_value_t ret_val = iotjs_jval_create_string(&code);

  iotjs_string_destroy(&path);
  iotjs_string_destroy(&code);

  return ret_val;
}


JS_FUNCTION(Cwd) {
  char path[IOTJS_MAX_PATH_SIZE];
  size_t size_path = sizeof(path);
  int err = uv_cwd(path, &size_path);
  if (err) {
    return JS_CREATE_ERROR(COMMON, "cwd error");
  }

  return jerry_create_string_from_utf8((const jerry_char_t*)path);
}

JS_FUNCTION(Chdir) {
  DJS_CHECK_ARGS(1, string);

  iotjs_string_t path = JS_GET_ARG(0, string);
  int err = uv_chdir(iotjs_string_data(&path));

  if (err) {
    iotjs_string_destroy(&path);
    return JS_CREATE_ERROR(COMMON, "chdir error");
  }

  iotjs_string_destroy(&path);
  return jerry_create_undefined();
}


JS_FUNCTION(DoExit) {
  iotjs_environment_t* env = iotjs_environment_get();

  if (!iotjs_environment_is_exiting(env)) {
    DJS_CHECK_ARGS(1, number);
    int exit_code = JS_GET_ARG(0, number);

    iotjs_set_process_exitcode(exit_code);
    iotjs_environment_set_state(env, kExiting);
  }
  return jerry_create_undefined();
}


void SetNativeSources(jerry_value_t native_sources) {
  for (int i = 0; js_modules[i].name; i++) {
    iotjs_jval_set_property_jval(native_sources, js_modules[i].name,
                                 jerry_create_boolean(true));
  }
}


static void SetProcessEnv(jerry_value_t process) {
  const char *homedir, *iotjspath, *iotjsenv, *extra_module_path,
      *working_dir_path;

  homedir = getenv(IOTJS_MAGIC_STRING_HOME_U);
  if (homedir == NULL) {
    homedir = "";
  }

  iotjspath = getenv(IOTJS_MAGIC_STRING_IOTJS_PATH_U);
  if (iotjspath == NULL) {
#if defined(__NUTTX__)
    iotjspath = "/mnt/sdcard";
#elif defined(__TIZENRT__)
    iotjspath = "/rom";
#else
    iotjspath = "";
#endif
  }

#if defined(EXPERIMENTAL)
  iotjsenv = "experimental";
#else
  iotjsenv = "";
#endif

  extra_module_path = getenv(IOTJS_MAGIC_STRING_IOTJS_EXTRA_MODULE_PATH_U);
  working_dir_path = getenv(IOTJS_MAGIC_STRING_IOTJS_WORKING_DIR_PATH_U);

  jerry_value_t env = jerry_create_object();
  iotjs_jval_set_property_string_raw(env, IOTJS_MAGIC_STRING_HOME_U, homedir);
  iotjs_jval_set_property_string_raw(env, IOTJS_MAGIC_STRING_IOTJS_PATH_U,
                                     iotjspath);
  iotjs_jval_set_property_string_raw(env, IOTJS_MAGIC_STRING_IOTJS_ENV_U,
                                     iotjsenv);
  iotjs_jval_set_property_string_raw(
      env, IOTJS_MAGIC_STRING_IOTJS_EXTRA_MODULE_PATH_U,
      extra_module_path ? extra_module_path : "");
  iotjs_jval_set_property_string_raw(
      env, IOTJS_MAGIC_STRING_IOTJS_WORKING_DIR_PATH_U,
      working_dir_path ? working_dir_path : "");

  iotjs_jval_set_property_jval(process, IOTJS_MAGIC_STRING_ENV, env);

  jerry_release_value(env);
}


static void SetProcessIotjs(jerry_value_t process) {
  // IoT.js specific
  jerry_value_t iotjs = jerry_create_object();
  iotjs_jval_set_property_jval(process, IOTJS_MAGIC_STRING_IOTJS, iotjs);

  iotjs_jval_set_property_string_raw(iotjs, IOTJS_MAGIC_STRING_BOARD,
                                     TOSTRING(TARGET_BOARD));
  jerry_release_value(iotjs);
}


static void SetProcessArgv(jerry_value_t process) {
  const iotjs_environment_t* env = iotjs_environment_get();
  uint32_t argc = iotjs_environment_argc(env);

  jerry_value_t argv = jerry_create_array(argc);

  for (uint32_t i = 0; i < argc; ++i) {
    const char* argvi = iotjs_environment_argv(env, i);
    jerry_value_t arg = jerry_create_string((const jerry_char_t*)argvi);
    iotjs_jval_set_property_by_index(argv, i, arg);
    jerry_release_value(arg);
  }
  iotjs_jval_set_property_jval(process, IOTJS_MAGIC_STRING_ARGV, argv);

  jerry_release_value(argv);
}


static void SetBuiltinModules(jerry_value_t builtin_modules) {
  for (unsigned i = 0; js_modules[i].name; i++) {
    iotjs_jval_set_property_jval(builtin_modules, js_modules[i].name,
                                 jerry_create_boolean(true));
  }
  for (unsigned i = 0; i < iotjs_module_count; i++) {
    iotjs_jval_set_property_jval(builtin_modules, iotjs_module_ro_data[i].name,
                                 jerry_create_boolean(true));
  }
}

static void SetProcessPrivate(jerry_value_t process, bool wait_source) {
  jerry_value_t private = jerry_create_object();
  iotjs_jval_set_property_jval(process, IOTJS_MAGIC_STRING_PRIVATE, private);

  iotjs_jval_set_method(private, IOTJS_MAGIC_STRING_COMPILE, Compile);
  iotjs_jval_set_method(private, IOTJS_MAGIC_STRING_COMPILEMODULE,
                        CompileModule);
  iotjs_jval_set_method(private, IOTJS_MAGIC_STRING_READSOURCE, ReadSource);

  // debugger
  iotjs_jval_set_method(private, IOTJS_MAGIC_STRING_DEBUGGERGETSOURCE,
                        DebuggerGetSource);

  jerry_value_t wait_source_val = jerry_create_boolean(wait_source);
  iotjs_jval_set_property_jval(private, IOTJS_MAGIC_STRING_DEBUGGERWAITSOURCE,
                               wait_source_val);

  jerry_release_value(wait_source_val);
  jerry_release_value(private);
}


jerry_value_t InitProcess() {
  jerry_value_t process = jerry_create_object();

  iotjs_jval_set_method(process, IOTJS_MAGIC_STRING_CWD, Cwd);
  iotjs_jval_set_method(process, IOTJS_MAGIC_STRING_CHDIR, Chdir);
  iotjs_jval_set_method(process, IOTJS_MAGIC_STRING_DOEXIT, DoExit);
  SetProcessEnv(process);

  // process.builtin_modules
  jerry_value_t builtin_modules = jerry_create_object();
  SetBuiltinModules(builtin_modules);
  iotjs_jval_set_property_jval(process, IOTJS_MAGIC_STRING_BUILTIN_MODULES,
                               builtin_modules);
  jerry_release_value(builtin_modules);

  // process.platform
  iotjs_jval_set_property_string_raw(process, IOTJS_MAGIC_STRING_PLATFORM,
                                     TARGET_OS);

  // process.arch
  iotjs_jval_set_property_string_raw(process, IOTJS_MAGIC_STRING_ARCH,
                                     TARGET_ARCH);

  // process.version
  iotjs_jval_set_property_string_raw(process, IOTJS_MAGIC_STRING_VERSION,
                                     IOTJS_VERSION);

  // Set iotjs
  SetProcessIotjs(process);
  bool wait_source = false;
  if (iotjs_environment_config(iotjs_environment_get())->debugger != NULL) {
    wait_source = iotjs_environment_config(iotjs_environment_get())
                      ->debugger->wait_source;
  }

  if (!wait_source) {
    SetProcessArgv(process);
  }

  SetProcessPrivate(process, wait_source);

  return process;
}
