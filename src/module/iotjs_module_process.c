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
#include "iotjs_js.h"

#include <stdlib.h>


JHANDLER_FUNCTION(Binding) {
  JHANDLER_CHECK_ARGS(1, number);

  ModuleKind module_kind = (ModuleKind)JHANDLER_GET_ARG(0, number);

  const iotjs_jval_t* jmodule =
      iotjs_module_initialize_if_necessary(module_kind);

  iotjs_jhandler_return_jval(jhandler, jmodule);
}


static iotjs_jval_t WrapEval(const char* source, size_t length, bool* throws) {
  static const char* wrapper[2] = { "(function(exports, require, module) {\n",
                                    "\n});\n" };

  int len0 = strlen(wrapper[0]);
  int len1 = strlen(wrapper[1]);

  iotjs_string_t code = iotjs_string_create("");
  iotjs_string_reserve(&code, len0 + length + len1);
  iotjs_string_append(&code, wrapper[0], len0);
  iotjs_string_append(&code, source, length);
  iotjs_string_append(&code, wrapper[1], len1);

  iotjs_jval_t res =
      iotjs_jhelper_eval(iotjs_string_data(&code), iotjs_string_size(&code),
                         false, throws);

  iotjs_string_destroy(&code);

  return res;
}


JHANDLER_FUNCTION(Compile) {
  JHANDLER_CHECK_ARGS(1, string);

  iotjs_string_t source = JHANDLER_GET_ARG(0, string);

  bool throws;
  iotjs_jval_t jres =
      WrapEval(iotjs_string_data(&source), iotjs_string_size(&source), &throws);

  if (!throws) {
    iotjs_jhandler_return_jval(jhandler, &jres);
  } else {
    iotjs_jhandler_throw(jhandler, &jres);
  }

  iotjs_string_destroy(&source);
  iotjs_jval_destroy(&jres);
}


JHANDLER_FUNCTION(CompileNativePtr) {
  JHANDLER_CHECK_ARGS(1, string);

  iotjs_string_t id = JHANDLER_GET_ARG(0, string);

  int i = 0;
  while (natives[i].name != NULL) {
    if (!strcmp(natives[i].name, iotjs_string_data(&id))) {
      break;
    }

    i++;
  }

  iotjs_string_destroy(&id);

  if (natives[i].name != NULL) {
    bool throws;
#ifdef ENABLE_SNAPSHOT
    iotjs_jval_t jres = iotjs_jhelper_exec_snapshot(natives[i].code,
                                                    natives[i].length, &throws);
#else
    iotjs_jval_t jres =
        WrapEval((const char*)natives[i].code, natives[i].length, &throws);
#endif

    if (!throws) {
      iotjs_jhandler_return_jval(jhandler, &jres);
    } else {
      iotjs_jhandler_throw(jhandler, &jres);
    }
    iotjs_jval_destroy(&jres);
  } else {
    iotjs_jval_t jerror = iotjs_jval_create_error("Unknown native module");
    iotjs_jhandler_throw(jhandler, &jerror);
    iotjs_jval_destroy(&jerror);
  }
}


JHANDLER_FUNCTION(ReadSource) {
  JHANDLER_CHECK_ARGS(1, string);

  iotjs_string_t path = JHANDLER_GET_ARG(0, string);
  iotjs_string_t code = iotjs_file_read(iotjs_string_data(&path));

  iotjs_jhandler_return_string(jhandler, &code);

  iotjs_string_destroy(&path);
  iotjs_string_destroy(&code);
}


JHANDLER_FUNCTION(Cwd) {
  JHANDLER_CHECK_ARGS(0);

  char path[IOTJS_MAX_PATH_SIZE];
  size_t size_path = sizeof(path);
  int err = uv_cwd(path, &size_path);
  if (err) {
    JHANDLER_THROW(COMMON, "cwd error");
    return;
  }
  iotjs_jhandler_return_string_raw(jhandler, path);
}

JHANDLER_FUNCTION(Chdir) {
  JHANDLER_CHECK_ARGS(1, string);

  iotjs_string_t path = JHANDLER_GET_ARG(0, string);
  int err = uv_chdir(iotjs_string_data(&path));

  if (err) {
    iotjs_string_destroy(&path);
    JHANDLER_THROW(COMMON, "chdir error");
    return;
  }

  iotjs_string_destroy(&path);
}


JHANDLER_FUNCTION(DoExit) {
  JHANDLER_CHECK_ARGS(1, number);

  int exit_code = JHANDLER_GET_ARG(0, number);

  exit(exit_code);
}


// Initialize `process.argv`
JHANDLER_FUNCTION(InitArgv) {
  JHANDLER_CHECK_THIS(object);

  // environment
  const iotjs_environment_t* env = iotjs_environment_get();

  // process.argv
  const iotjs_jval_t* thisObj = JHANDLER_GET_THIS(object);
  iotjs_jval_t jargv = iotjs_jval_get_property(thisObj, "argv");

  int argc = iotjs_environment_argc(env);

  for (int i = 0; i < argc; ++i) {
    const char* argvi = iotjs_environment_argv(env, i);
    iotjs_jval_t arg = iotjs_jval_create_string_raw(argvi);
    iotjs_jval_set_property_by_index(&jargv, i, &arg);
    iotjs_jval_destroy(&arg);
  }
  iotjs_jval_destroy(&jargv);
}


void SetNativeSources(iotjs_jval_t* native_sources) {
  for (int i = 0; natives[i].name; i++) {
    iotjs_jval_t native_src = iotjs_jval_create_object();
    uintptr_t handle = (uintptr_t)(&natives[i]);
    iotjs_jval_set_object_native_handle(&native_src, handle, NULL);
    iotjs_jval_set_property_jval(native_sources, natives[i].name, &native_src);
    iotjs_jval_destroy(&native_src);
  }
}


static void SetProcessEnv(iotjs_jval_t* process) {
  const char *homedir, *nodepath;

  homedir = getenv("HOME");
  if (homedir == NULL) {
    homedir = "";
  }

  nodepath = getenv("NODE_PATH");
  if (nodepath == NULL) {
#if defined(__NUTTX__)
    nodepath = "/mnt/sdcard";
#else
    nodepath = "";
#endif
  }

  iotjs_jval_t env = iotjs_jval_create_object();
  iotjs_jval_set_property_string_raw(&env, "HOME", homedir);
  iotjs_jval_set_property_string_raw(&env, "NODE_PATH", nodepath);

  iotjs_jval_set_property_jval(process, "env", &env);

  iotjs_jval_destroy(&env);
}


static void SetProcessIotjs(iotjs_jval_t* process) {
  // IoT.js specific
  iotjs_jval_t iotjs = iotjs_jval_create_object();
  iotjs_jval_set_property_jval(process, "iotjs", &iotjs);

  iotjs_jval_set_property_string_raw(&iotjs, "board", TOSTRING(TARGET_BOARD));
  iotjs_jval_destroy(&iotjs);
}


iotjs_jval_t InitProcess() {
  iotjs_jval_t process = iotjs_jval_create_object();

  iotjs_jval_set_method(&process, "binding", Binding);
  iotjs_jval_set_method(&process, "compile", Compile);
  iotjs_jval_set_method(&process, "compileNativePtr", CompileNativePtr);
  iotjs_jval_set_method(&process, "readSource", ReadSource);
  iotjs_jval_set_method(&process, "cwd", Cwd);
  iotjs_jval_set_method(&process, "chdir", Chdir);
  iotjs_jval_set_method(&process, "doExit", DoExit);
  iotjs_jval_set_method(&process, "_initArgv", InitArgv);
  SetProcessEnv(&process);

  // process.native_sources
  iotjs_jval_t native_sources = iotjs_jval_create_object();
  SetNativeSources(&native_sources);
  iotjs_jval_set_property_jval(&process, "native_sources", &native_sources);
  iotjs_jval_destroy(&native_sources);

  // process.platform
  iotjs_jval_set_property_string_raw(&process, "platform", TARGET_OS);

  // process.arch
  iotjs_jval_set_property_string_raw(&process, "arch", TARGET_ARCH);

  // Set iotjs
  SetProcessIotjs(&process);

  // Binding module id.
  iotjs_jval_t jbinding = iotjs_jval_get_property(&process, "binding");

#define ENUMDEF_MODULE_LIST(upper, Camel, lower) \
  iotjs_jval_set_property_number(&jbinding, #lower, MODULE_##upper);

  MAP_MODULE_LIST(ENUMDEF_MODULE_LIST)

#undef ENUMDEF_MODULE_LIST

  iotjs_jval_destroy(&jbinding);

  return process;
}
