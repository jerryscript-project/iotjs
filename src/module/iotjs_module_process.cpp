/* Copyright 2015-2016 Samsung Electronics Co., Ltd.
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
#include "iotjs_module_process.h"
#include "iotjs_js.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


namespace iotjs {


JObject* GetProcess() {
  Module* module = GetBuiltinModule(MODULE_PROCESS);
  IOTJS_ASSERT(module != NULL);

  JObject* process = module->module;
  IOTJS_ASSERT(process != NULL);
  IOTJS_ASSERT(process->IsObject());

  return process;
}


JHANDLER_FUNCTION(Binding) {
  JHANDLER_CHECK(iotjs_jhandler_get_arg_length(jhandler) == 1);
  JHANDLER_CHECK(iotjs_jhandler_get_arg(jhandler, 0)->IsNumber());

  int module_kind = iotjs_jhandler_get_arg(jhandler, 0)->GetInt32();

  Module* module = GetBuiltinModule(static_cast<ModuleKind>(module_kind));
  IOTJS_ASSERT(module != NULL);

  if (module->module == NULL) {
    IOTJS_ASSERT(module->fn_register != NULL);
    module->module = module->fn_register();
    IOTJS_ASSERT(module->module);
  }

  iotjs_jhandler_return_obj(jhandler, module->module);
}


static JResult WrapEval(const char* source, size_t length) {
  static const char* wrapper[2] = {
      "(function(exports, require, module) {\n",
      "\n});\n" };

  int len0 = strlen(wrapper[0]);
  int len1 = strlen(wrapper[1]);

  iotjs_string_t code = iotjs_string_create("");
  iotjs_string_reserve(&code, len0 + length + len1);
  iotjs_string_append(&code, wrapper[0], len0);
  iotjs_string_append(&code, source, length);
  iotjs_string_append(&code, wrapper[1], len1);

  JResult res = JObject::Eval(code);

  iotjs_string_destroy(&code);

  return res;
}


JHANDLER_FUNCTION(Compile){
  JHANDLER_CHECK(iotjs_jhandler_get_arg_length(jhandler) == 1);
  JHANDLER_CHECK(iotjs_jhandler_get_arg(jhandler, 0)->IsString());

  iotjs_string_t source = iotjs_jhandler_get_arg(jhandler, 0)->GetString();

  JResult jres = WrapEval(iotjs_string_data(&source),
                          iotjs_string_size(&source));

  if (jres.IsOk()) {
    iotjs_jhandler_return_obj(jhandler, &jres.value());
  } else {
    iotjs_jhandler_throw_obj(jhandler, &jres.value());
  }

  iotjs_string_destroy(&source);
}


JHANDLER_FUNCTION(CompileNativePtr){
  JHANDLER_CHECK(iotjs_jhandler_get_arg_length(jhandler) == 1);
  JHANDLER_CHECK(iotjs_jhandler_get_arg(jhandler, 0)->IsString());

  iotjs_string_t id = iotjs_jhandler_get_arg(jhandler, 0)->GetString();

  int i=0;
  while (natives[i].name != NULL) {
    if (!strcmp(natives[i].name, iotjs_string_data(&id))) {
      break;
    }

    i++;
  }

  iotjs_string_destroy(&id);

  if (natives[i].name != NULL) {
#ifdef ENABLE_SNAPSHOT
    JResult jres = JObject::ExecSnapshot(natives[i].code,
                                         natives[i].length);
#else
    JResult jres = WrapEval((const char*)natives[i].code, natives[i].length);
#endif

    if (jres.IsOk()) {
      iotjs_jhandler_return_obj(jhandler, &jres.value());
    } else {
      iotjs_jhandler_throw_obj(jhandler, &jres.value());
    }
  } else {
    JObject jerror = JObject::Error ("Unknown native module");
    iotjs_jhandler_throw_obj(jhandler, &jerror);
  }
}


JHANDLER_FUNCTION(ReadSource){
  JHANDLER_CHECK(iotjs_jhandler_get_arg_length(jhandler) == 1);
  JHANDLER_CHECK(iotjs_jhandler_get_arg(jhandler, 0)->IsString());

  iotjs_string_t path = iotjs_jhandler_get_arg(jhandler, 0)->GetString();
  iotjs_string_t code = iotjs_file_read(iotjs_string_data(&path));

  JObject ret(code);
  iotjs_jhandler_return_obj(jhandler, &ret);

  iotjs_string_destroy(&path);
  iotjs_string_destroy(&code);
}


JHANDLER_FUNCTION(Cwd){
  JHANDLER_CHECK(iotjs_jhandler_get_arg_length(jhandler) == 0);

  char path[IOTJS_MAX_PATH_SIZE];
  size_t size_path = sizeof(path);
  int err = uv_cwd(path, &size_path);
  if (err) {
    JHANDLER_THROW_RETURN(Error, "cwd error");
  }
  JObject ret(path);
  iotjs_jhandler_return_obj(jhandler, &ret);
}

JHANDLER_FUNCTION(Chdir){
  JHANDLER_CHECK(iotjs_jhandler_get_arg_length(jhandler) == 1);
  JHANDLER_CHECK(iotjs_jhandler_get_arg(jhandler, 0)->IsString());

  iotjs_string_t path = iotjs_jhandler_get_arg(jhandler, 0)->GetString();
  int err = uv_cd(iotjs_string_data(&path));

  if (err) {
    iotjs_string_destroy(&path);
    JHANDLER_THROW_RETURN(Error, "chdir error");
  }

  iotjs_string_destroy(&path);
}


JHANDLER_FUNCTION(DoExit) {
  JHANDLER_CHECK(iotjs_jhandler_get_arg_length(jhandler) == 1);
  JHANDLER_CHECK(iotjs_jhandler_get_arg(jhandler, 0)->IsNumber());

  int exit_code = iotjs_jhandler_get_arg(jhandler, 0)->GetInt32();

  exit(exit_code);
}


// Initialize `process.argv`
JHANDLER_FUNCTION(InitArgv) {
  JHANDLER_CHECK(iotjs_jhandler_get_this(jhandler)->IsObject());

  // environtment
  Environment* env = Environment::GetEnv();

  // process.argv
  JObject jargv = iotjs_jhandler_get_this(jhandler)->GetProperty("argv");

  for (int i = 0; i < env->argc(); ++i) {
    char index[10] = {0};
    sprintf(index, "%d", i);
    JObject value(env->argv()[i]);
    jargv.SetProperty(index, value);
  }
}


void SetNativeSources(JObject* native_sources) {
  for (int i = 0; natives[i].name; i++) {
    JObject native_source;
    native_source.SetNative((uintptr_t)(&natives[i]), NULL);
    native_sources->SetProperty(natives[i].name, native_source);
  }
}


static void SetProcessEnv(JObject* process){
  const char *homedir, *nodepath;
  homedir = getenv("HOME");
  if (homedir == NULL) {
    homedir = "";
  }
  JObject home(homedir);
  nodepath = getenv("NODE_PATH");
  if (nodepath == NULL) {
#if defined(__NUTTX__)
    nodepath = "/mnt/sdcard";
#else
    nodepath = "";
#endif
  }
  JObject node_path(nodepath);

  JObject env;
  env.SetProperty("HOME", home);
  env.SetProperty("NODE_PATH", node_path);

  process->SetProperty("env", env);
}


static void SetProcessIotjs(JObject* process) {
  // IoT.js specific
  JObject iotjs;
  process->SetProperty("iotjs", iotjs);

  JObject jboard(TOSTRING(TARGET_BOARD));
  iotjs.SetProperty("board", jboard);
}


JObject* InitProcess() {
  Module* module = GetBuiltinModule(MODULE_PROCESS);
  JObject* process = module->module;

  if (process == NULL) {
    process = new JObject();
    process->SetMethod("binding", Binding);
    process->SetMethod("compile", Compile);
    process->SetMethod("compileNativePtr", CompileNativePtr);
    process->SetMethod("readSource", ReadSource);
    process->SetMethod("cwd", Cwd);
    process->SetMethod("chdir", Chdir);
    process->SetMethod("doExit", DoExit);
    process->SetMethod("_initArgv", InitArgv);
    SetProcessEnv(process);

    // process.native_sources
    JObject native_sources;
    SetNativeSources(&native_sources);
    process->SetProperty("native_sources", native_sources);

    // process.platform
    JObject platform(TARGET_OS);
    process->SetProperty("platform", platform);

    // process.arch
    JObject arch(TARGET_ARCH);
    process->SetProperty("arch", arch);

    // Set iotjs
    SetProcessIotjs(process);

    // Binding module id.
    JObject jbinding = process->GetProperty("binding");

#define ENUMDEF_MODULE_LIST(upper, Camel, lower) \
    jbinding.SetProperty(#lower, iotjs_jval_number(MODULE_ ## upper));

    MAP_MODULE_LIST(ENUMDEF_MODULE_LIST)

#undef ENUMDEF_MODULE_LIST

    module->module = process;
  }

  return process;
}


} // namespace iotjs
