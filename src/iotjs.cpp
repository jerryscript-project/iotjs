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
#include "iotjs_js.h"

namespace iotjs {


static bool InitJerry() {

  jerry_init(JERRY_FLAG_EMPTY);

  if (!jerry_parse(NULL, 0)) {
    fprintf(stderr, "jerry_parse() failed\n");
    return false;
  }

  if (jerry_run() != JERRY_COMPLETION_CODE_OK) {
    fprintf(stderr, "jerry_run() failed\n");
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

  JRawValueType retval;
  jerry_api_eval(mainjs,mainjs_length,
                 false, false, &retval);
  JObject iotjs_fun(&retval, true);
  JArgList args(1);
  args.Add(*process);
  JObject global(JObject::Global());
  iotjs_fun.Call(global, args);

  return true;
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
    if (more == false) {
      OnNextTick();
      more = uv_loop_alive(env.loop());
    }
  } while (more);

  return true;
}


int Start(char* src) {
  if (!InitJerry()) {
    fprintf(stderr, "InitJerry failed\n");
    return 1;
  }

  JObject* process = InitModules();

  // FIXME: this should be moved to seperate function
  {
    JObject argv;
    JObject user_filename(src);
    argv.SetProperty("1", user_filename);
    process->SetProperty("argv", argv);
  }

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

  int res = iotjs::Start(argv[1]);

  return res;
}
