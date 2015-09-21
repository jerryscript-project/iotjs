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

#include "jerry.h"
#include "jerry-api.h"

#include <stdio.h>
#include <string.h>


namespace iotjs {


Environment* Environment::_env = NULL;


static bool InitJerry() {

  uint32_t jerry_flag = JERRY_FLAG_ABORT_ON_FAIL;

#ifdef ENABLE_JERRY_MEM_STATS
  jerry_flag |= JERRY_FLAG_MEM_STATS;
  jerry_flag |= JERRY_FLAG_SHOW_OPCODES;
#endif

  jerry_init(jerry_flag);

  InitJerryMagicStringEx();

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
  JResult jmain = JObject::Eval(String(iotjs_s), false, false);
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


int Start(int argc, char** argv) {
  InitDebugSettings();

  // Initalize JerryScript engine.
  if (!InitJerry()) {
    DLOG("InitJerry failed");
    return 1;
  }

  // Create environtment.
  Environment* env = Environment::GetEnv();

  // Init environment with argument and uv loop.
  env->Init(argc, argv, uv_default_loop());

  // Start IoT.js
  if (!StartIoTjs(env)) {
    DLOG("StartIoTJs failed");
    return 1;
  }

  // close uv loop.
  uv_loop_close(env->loop());

  // Release JerryScript engine.
  ReleaseJerry();

  ReleaseDebugSettings();

  return 0;
}


} // namespace iotjs


extern "C" int iotjs_entry(int argc, char** argv) {
  if (argc < 2) {
    fprintf(stderr, "Usage: iotjs <js>\n");
    return 1;
  }

  return iotjs::Start(argc, argv);
}
