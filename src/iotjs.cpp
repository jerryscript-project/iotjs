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

#include "iotjs.h"
#include "iotjs_js.h"
#include "iotjs_string_ext.h"
#include "iotjs_handlewrap.h"

#include "jerry.h"
#include "jerry-api.h"

#include <stdio.h>
#include <string.h>


namespace iotjs {


Environment* Environment::_env = NULL;


/**
 * Initialize JerryScript.
 */
static bool InitJerry(Environment* env) {
  // Set jerry run flags.
  uint32_t jerry_flag = JERRY_FLAG_ABORT_ON_FAIL;

  if (env->config()->memstat) {
    jerry_flag |= JERRY_FLAG_MEM_STATS;
  }

  if (env->config()->show_opcode) {
    jerry_flag |= JERRY_FLAG_SHOW_OPCODES;
  }

  // Initialize jerry.
  jerry_init(jerry_flag);

  // Set magic strings.
  InitJerryMagicStringEx();

  // Do parse and run to generate initial javascript environment.
  if (!jerry_parse((jerry_api_char_t*)"", 0)) {
    DLOG("jerry_parse() failed");
    return false;
  }

  if (jerry_run() != JERRY_COMPLETION_CODE_OK) {
    DLOG("jerry_run() failed");
    return false;
  }

  return true;
}


static void ReleaseJerry() {
  jerry_cleanup();
}


static JObject* InitModules() {
  InitModuleList();
  return InitProcess();
}


static void CleanupModules() {
  CleanupModuleList();
}


static bool RunIoTjs(JObject* process) {
  // Evaluating 'iotjs.js' returns a function.
#ifndef ENABLE_SNAPSHOT
  JResult jmain = JObject::Eval(String(iotjs_s), false, false);
#else
  JResult jmain = JObject::ExecSnapshot(iotjs_s, iotjs_l);
#endif
  IOTJS_ASSERT(jmain.IsOk());

  // Run the entry function passing process builtin.
  // The entry function will continue initializing process module, global, and
  // other native modules, and finally load and run application.
  JArgList args(1);
  args.Add(*process);

  JObject global(JObject::Global());
  JResult jmain_res = jmain.value().Call(global, args);

  if (jmain_res.IsException()) {
    UncaughtException(jmain_res.value());
    return false;
  } else {
    return true;
  }
}


static bool StartIoTjs(Environment* env) {
  // Get jerry global object.
  JObject global = JObject::Global();

  // Bind environment to global object.
  global.SetNative((uintptr_t)(env), NULL);

  // Initialize builtin modules.
  JObject* process = InitModules();

  // Call the entry.
  // load and call iotjs.js
  env->GoStateRunningMain();

  RunIoTjs(process);

  // Run event loop.
  env->GoStateRunningLoop();

  bool more;
  do {
    more = uv_run(env->loop(), UV_RUN_ONCE);
    more |= ProcessNextTick();
    if (more == false) {
      more = uv_loop_alive(env->loop());
    }
  } while (more);

  env->GoStateExiting();

  // Emit 'exit' event.
  ProcessEmitExit(0);

  // Release bulitin modules.
  CleanupModules();

  return true;
}


static void UvWalkToCloseCallback(uv_handle_t* handle, void* arg) {
  HandleWrap* handle_wrap = HandleWrap::FromHandle(handle);
  IOTJS_ASSERT(handle_wrap != NULL);

  handle_wrap->Close(NULL);
}


int Start(int argc, char** argv) {
  // Initialize debug print.
  InitDebugSettings();

  // Create environtment.
  Environment* env = Environment::GetEnv();

  // Parse command line arguemnts.
  if (!env->ParseCommandLineArgument(argc, argv)) {
    DLOG("ParseCommandLineArgument failed");
    return 1;
  }

  // Set event loop.
  env->set_loop(uv_default_loop());

  // Initalize JerryScript engine.
  if (!InitJerry(env)) {
    DLOG("InitJerry failed");
    return 1;
  }

  // Start IoT.js
  if (!StartIoTjs(env)) {
    DLOG("StartIoTJs failed");
    return 1;
  }

  // close uv loop.
  //uv_stop(env->loop());
  uv_walk(env->loop(), UvWalkToCloseCallback, NULL);
  uv_run(env->loop(), UV_RUN_DEFAULT);

  int res = uv_loop_close(env->loop());
  IOTJS_ASSERT(res == 0);

  // Release JerryScript engine.
  ReleaseJerry();

  // Release environment.
  Environment::Release();

  // Release debug print setting.
  ReleaseDebugSettings();

  return 0;
}


} // namespace iotjs


extern "C" int iotjs_entry(int argc, char** argv) {
  return iotjs::Start(argc, argv);
}
