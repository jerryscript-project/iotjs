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


static bool InitJerry() {

#ifdef ENABLE_JERRY_MEM_STATS
  jerry_init(JERRY_FLAG_MEM_STATS | JERRY_FLAG_SHOW_OPCODES);
#else
  jerry_init(JERRY_FLAG_EMPTY);
#endif

  InitJerryMagicStringEx();

  if (!jerry_parse("", 0)) {
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


static bool InitIoTjs(JObject* process) {
  JResult jmain = JObject::Eval(mainjs, false, false);
  IOTJS_ASSERT(jmain.IsOk());

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


static bool StartIoTjs(JObject* process) {

  // Get jerry global object.
  JObject global = JObject::Global();

  // Create environtment.
  Environment env(uv_default_loop());

  // Bind environment to global object.
  global.SetNative((uintptr_t)(&env), NULL);

  // Call the entry.
  // load and call iotjs.js
  InitIoTjs(process);

  bool more;
  do {
    more = uv_run(env.loop(), UV_RUN_ONCE);
    more |= ProcessNextTick();
    if (more == false) {
      more = uv_loop_alive(env.loop());
    }
  } while (more);

  ProcessExit(0);

  return true;
}


int Start(char* src) {
  if (!InitJerry()) {
    DLOG("InitJerry failed");
    return 1;
  }

  JObject* process = InitModules();

  SetProcessIotjs(process);

  // FIXME: this should be moved to seperate function
  {
    JObject argv;
    JObject user_filename(src);
    argv.SetProperty("1", user_filename);
    process->SetProperty("argv", argv);
  }

  if (!StartIoTjs(process)) {
    DLOG("StartIoTJs failed");
    return 1;
  }

  CleanupModules();

  ReleaseJerry();

  return 0;
}


} // namespace iotjs


extern "C" int iotjs_entry(int argc, char** argv) {
  if (argc < 2) {
    fprintf(stderr, "Usage: iotjs <js>\n");
    return 1;
  }
  iotjs::InitDebugSettings();

  int res = iotjs::Start(argv[1]);

  iotjs::ReleaseDebugSettings();

  return res;
}
