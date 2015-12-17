/* Copyright 2015 Samsung Electronics Co., Ltd.
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
#include "native_js.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


namespace iotjs {


static JObject* GetProcess() {
  Module* module = GetBuiltinModule(MODULE_PROCESS);
  IOTJS_ASSERT(module != NULL);

  JObject* process = module->module;
  IOTJS_ASSERT(process != NULL);
  IOTJS_ASSERT(process->IsObject());

  return process;
}


void UncaughtException(JObject& jexception) {
  JObject* process = GetProcess();

  JObject jonuncaughtexception(process->GetProperty("_onUncaughtExcecption"));
  IOTJS_ASSERT(jonuncaughtexception.IsFunction());

  JArgList args(1);
  args.Add(jexception);

  JResult jres = jonuncaughtexception.Call(*process, args);
  IOTJS_ASSERT(jres.IsOk());
}


void ProcessEmitExit(int code) {
  JObject* process = GetProcess();

  JObject jexit(process->GetProperty("emitExit"));
  IOTJS_ASSERT(jexit.IsFunction());

  JArgList args(1);
  args.Add(JVal::Number(code));

  JResult jres = jexit.Call(JObject::Null(), args);
  if (!jres.IsOk()) {
    exit(2);
  }
}


// Calls next tick callbacks registered via `process.nextTick()`.
bool ProcessNextTick() {
  JObject* process = GetProcess();

  JObject jon_next_tick(process->GetProperty("_onNextTick"));
  IOTJS_ASSERT(jon_next_tick.IsFunction());

  JResult jres = jon_next_tick.Call(JObject::Null(), JArgList::Empty());
  IOTJS_ASSERT(jres.IsOk());
  IOTJS_ASSERT(jres.value().IsBoolean());

  return jres.value().GetBoolean();
}


// Make a callback for the given `function` with `this_` binding and `args`
// arguments. The next tick callbacks registered via `process.nextTick()`
// will be called after the callback function `function` returns.
JObject MakeCallback(JObject& function, JObject& this_, JArgList& args) {
  // Calls back the function.
  JResult jres = function.Call(this_, args);
  if (jres.IsException()) {
    UncaughtException(jres.value());
  }

  // Calls the next tick callbacks.
  ProcessNextTick();

  // Return value.
  return jres.value();
}


JHANDLER_FUNCTION(Binding) {
  JHANDLER_CHECK(handler.GetArgLength() == 1);
  JHANDLER_CHECK(handler.GetArg(0)->IsNumber());

  int module_kind = handler.GetArg(0)->GetInt32();

  Module* module = GetBuiltinModule(static_cast<ModuleKind>(module_kind));
  IOTJS_ASSERT(module != NULL);

  if (module->module == NULL) {
    IOTJS_ASSERT(module->fn_register != NULL);
    module->module = module->fn_register();
    IOTJS_ASSERT(module->module);
  }

  handler.Return(*module->module);

  return true;
}


static JResult WrapEval(const char* source) {
  static const char* wrapper[2] = {
      "(function(exports, require, module) {\n",
      "});\n" };

  int len1 = strlen(wrapper[0]);
  int len2 = strlen(source);
  int len3 = strlen(wrapper[1]);

  String code("", len1 + len2 + len3 + 1);
  strcpy(code.data(), wrapper[0]);
  strcat(code.data() + len1, source);
  strcat(code.data() + len1 + len2, wrapper[1]);

  return JObject::Eval(code);
}


JHANDLER_FUNCTION(Compile){
  JHANDLER_CHECK(handler.GetArgLength() == 1);
  JHANDLER_CHECK(handler.GetArg(0)->IsString());

  JResult jres = WrapEval(handler.GetArg(0)->GetString().data());

  if (jres.IsOk()) {
    handler.Return(jres.value());
  } else {
    handler.Throw(jres.value());
  }

  return !handler.HasThrown();
}


JHANDLER_FUNCTION(CompileNativePtr){
  JHANDLER_CHECK(handler.GetArgLength() == 1);
  JHANDLER_CHECK(handler.GetArg(0)->IsString());

  String id (handler.GetArg(0)->GetString());

  int i=0;
  while (natives[i].name != NULL) {
    const char *name_iter_p = natives[i].name;
    size_t name_len = 0;
    while (*name_iter_p != '\0')
    {
      name_len++;
      name_iter_p++;
    }

    if (name_len == (size_t) id.size ()
        && !strncmp (natives[i].name, id.data(), id.size())) {
      break;
    }

    i++;
  }

  if (natives[i].name != NULL) {
#ifdef ENABLE_SNAPSHOT
    JResult jres = JObject::ExecSnapshot(natives[i].code,
                                         natives[i].length);
#else
    JResult jres = WrapEval((const char*)natives[i].code);
#endif

    if (jres.IsOk()) {
      handler.Return(jres.value());
    } else {
      handler.Throw(jres.value());
    }
  } else {
    JObject jerror = JObject::Error ("Unknown native module");
    handler.Throw(jerror);
  }

  return !handler.HasThrown();
}


JHANDLER_FUNCTION(ReadSource){
  JHANDLER_CHECK(handler.GetArgLength() == 1);
  JHANDLER_CHECK(handler.GetArg(0)->IsString());

  String path = handler.GetArg(0)->GetString();
  String code = ReadFile(path.data());

  JObject ret(code);
  handler.Return(ret);

  return true;
}


JHANDLER_FUNCTION(Cwd){
  JHANDLER_CHECK(handler.GetArgLength() == 0);

  char path[IOTJS_MAX_PATH_SIZE];
  size_t size_path = sizeof(path);
  int err = uv_cwd(path, &size_path);
  if (err) {
    JHANDLER_THROW_RETURN(Error, "cwd error");
  }
  JObject ret(path);
  handler.Return(ret);

  return true;
}


JHANDLER_FUNCTION(DoExit) {
  JHANDLER_CHECK(handler.GetArgLength() == 1);
  JHANDLER_CHECK(handler.GetArg(0)->IsNumber());

  int exit_code = handler.GetArg(0)->GetInt32();

  exit(exit_code);
}


// Initialize `process.argv`
JHANDLER_FUNCTION(InitArgv) {
  JHANDLER_CHECK(handler.GetThis()->IsObject());

  // environtment
  Environment* env = Environment::GetEnv();

  // process.argv
  JObject jargv = handler.GetThis()->GetProperty("argv");

  for (int i = 0; i < env->argc(); ++i) {
    char index[10] = {0};
    sprintf(index, "%d", i);
    JObject value(env->argv()[i]);
    jargv.SetProperty(index, value);
  }

  return true;
}


void SetNativeSources(JObject* native_sources) {
  for (int i = 0; natives[i].name; i++) {
    JObject native_source;
    native_source.SetNative((uintptr_t)(&natives[i]), NULL);
    native_sources->SetProperty(natives[i].name, native_source);
  }
}


static void SetProcessEnv(JObject* process){
  const char *homedir;
  homedir = getenv("HOME");
  if (homedir == NULL) {
    homedir = "";
  }
  JObject home(homedir);
  JObject env;
  env.SetProperty("HOME", home);
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
    jbinding.SetProperty(#lower, JVal::Number(MODULE_ ## upper));

    MAP_MODULE_LIST(ENUMDEF_MODULE_LIST)

#undef ENUMDEF_MODULE_LIST

    module->module = process;
  }

  return process;
}


} // namespace iotjs
