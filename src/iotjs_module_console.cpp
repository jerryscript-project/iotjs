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
#include "iotjs_module_console.h"

#include <stdio.h>


namespace iotjs {


static bool Print(JHandlerInfo& handler, FILE* out_fd) {
  JHANDLER_CHECK(handler.GetArgLength() == 1);
  JHANDLER_CHECK(handler.GetArg(0)->IsString());

  String msg = handler.GetArg(0)->GetString();
  fprintf(out_fd, "%s", msg.data());

  return true;
}


JHANDLER_FUNCTION(Stdout) {
  return Print(handler, stdout);
}


JHANDLER_FUNCTION(Stderr) {
  return Print(handler, stderr);
}


JObject* InitConsole() {
  Module* module = GetBuiltinModule(MODULE_CONSOLE);
  JObject* console = module->module;

  if (console == NULL) {
    console = new JObject();
    console->SetMethod("stdout", Stdout);
    console->SetMethod("stderr", Stderr);

    module->module = console;
  }

  return console;
}

} // namespace iotjs
