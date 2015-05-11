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

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "jerry.h"
#include "jerry-api.h"

#include "uv.h"

#include "iotjs_binding.h"
#include "iotjs_env.h"
#include "iotjs_util.h"
#include "iotjs_module.h"
#include "iotjs_module_process.h"

#include "iotjs.h"


namespace iotjs {


static bool InitJerry(char* src) {
  //char* src = ReadFile(src_path);

  jerry_init(JERRY_FLAG_EMPTY);

  if (!jerry_parse(src, strlen(src))) {
    fprintf(stderr, "jerry_parse() failed\n");
    return false;
  }

  if (jerry_run() != JERRY_COMPLETION_CODE_OK) {
    fprintf(stderr, "jerry_run() failed\n");
    return false;
  }

  //ReleaseCharBuffer(src);

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


static bool StartIoTjs(JObject* process) {
  // Get jerry global object.
  JObject global = JObject::Global();

  // Create environtment.
  Environment env(uv_default_loop());

  // Bind environment to global object.
  global.SetNative((uintptr_t)(&env));

  // Find entry function.
  JObject start_func = global.GetProperty("startIoTjs");
  assert(start_func.IsFunction());

  // Call the entry.
  JArgList args(1);
  args.Add(*process);
  JObject res = start_func.Call(JObject::Null(), args);

  bool more;
  do {
    more = uv_run(env.loop(), UV_RUN_ONCE);
    if (more == false) {
      OnNextTick();
      more = uv_loop_alive(env.loop());
    }
  } while (more);

  return true;
}


int Start(char* src) {
  if (!InitJerry(src)) {
    fprintf(stderr, "InitJerry failed\n");
    return 1;
  }

  JObject* process = InitModules();

  if (!StartIoTjs(process)) {
    fprintf(stderr, "StartIoTJs failed\n");
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

  char* src = iotjs::ReadFile(argv[1]);

  int res = iotjs::Start(src);

  iotjs::ReleaseCharBuffer(src);

  return res;
}
