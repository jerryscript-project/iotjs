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

char buffer[128];

static bool Print(const jerry_api_object_t *function_obj_p,
                  const jerry_api_value_t *this_p,
                  jerry_api_value_t *ret_val_p,
                  const jerry_api_value_t args_p [],
                  const uint16_t args_cnt,
                  FILE* out_f) {
  if (args_cnt > 0) {
    if (args_p[0].type == JERRY_API_DATA_TYPE_STRING) {
      while (true) {
        ssize_t len = jerry_api_string_to_char_buffer(
            args_p[0].v_string, buffer, sizeof(buffer)-1);

        fprintf(out_f, buffer);

        if (len >= 0) break;
      }
      fprintf(out_f, "\n");
      return true;
    }
  }
  return false;
}

static bool Log(const jerry_api_object_t *function_obj_p,
                const jerry_api_value_t *this_p,
                jerry_api_value_t *ret_val_p,
                const jerry_api_value_t args_p [],
                const uint16_t args_cnt) {

  return Print(function_obj_p, this_p, ret_val_p, args_p, args_cnt, stdout);
}

static bool Error(const jerry_api_object_t *function_obj_p,
                  const jerry_api_value_t *this_p,
                  jerry_api_value_t *ret_val_p,
                  const jerry_api_value_t args_p [],
                  const uint16_t args_cnt) {
  return Print(function_obj_p, this_p, ret_val_p, args_p, args_cnt, stderr);
}


JObject* InitConsole() {
  Module* module = GetBuiltinModule(MODULE_CONSOLE);
  JObject* console = module->module;

  if (console == NULL) {
    console = new JObject();
    console->CreateMethod("log", Log);
    console->CreateMethod("info", Log);
    console->CreateMethod("error", Error);
    console->CreateMethod("warn", Error);

    module->module = console;
  }

  return console;
}

} // namespace iotjs
