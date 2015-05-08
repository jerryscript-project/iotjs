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
#include <assert.h>

#include "iotjs_util.h"
#include "iotjs_binding.h"
#include "iotjs_module.h"
#include "iotjs_module_console.h"


namespace iotjs {


static bool Print(JHandlerInfo& handler, FILE* out_fd) {
  if (handler.GetArgLength() > 0) {
    if (handler.GetArg(0)->IsString()) {
      char* str = handler.GetArg(0)->GetCString();
      fprintf(out_fd, "%s\n", str);
      JObject::ReleaseCString(str);

      return true;
    }
  }
  return false;
}


JHANDLER_FUNCTION(Log, handler) {
  return Print(handler, stdout);
}


JHANDLER_FUNCTION(Error, handler) {
  return Print(handler, stderr);
}


JObject* InitConsole() {
  Module* module = GetBuiltinModule(MODULE_CONSOLE);
  JObject* console = module->module;

  if (console == NULL) {
    console = new JObject();
    console->SetMethod("log", Log);
    console->SetMethod("info", Log);
    console->SetMethod("error", Error);
    console->SetMethod("warn", Error);

    module->module = console;
  }

  return console;
}

} // namespace iotjs
