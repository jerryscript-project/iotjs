/* Copyright 2015-present Samsung Electronics Co., Ltd. and other contributors
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
#include "iotjs_debuglog.h"

// This function should be able to print utf8 encoded string
// as utf8 is internal string representation in Jerryscript
static jerry_value_t Print(const jerry_value_t* jargv,
                           const jerry_length_t jargc, FILE* out_fd) {
  JS_CHECK_ARGS(1, string);
  iotjs_string_t msg = JS_GET_ARG(0, string);
  const char* str = iotjs_string_data(&msg);
  unsigned str_len = iotjs_string_size(&msg);
  unsigned idx = 0;

  if (iotjs_console_out) {
    int level = (out_fd == stdout) ? DBGLEV_INFO : DBGLEV_ERR;
    iotjs_console_out(level, "%s", str);
  } else {
    for (idx = 0; idx < str_len; idx++) {
      if (str[idx] != 0) {
        fprintf(out_fd, "%c", str[idx]);
      } else {
        fprintf(out_fd, "\\u0000");
      }
    }
  }

  iotjs_string_destroy(&msg);
  return jerry_create_undefined();
}


JS_FUNCTION(Stdout) {
  return Print(jargv, jargc, stdout);
}


JS_FUNCTION(Stderr) {
  return Print(jargv, jargc, stderr);
}


jerry_value_t InitConsole() {
  jerry_value_t console = jerry_create_object();

  iotjs_jval_set_method(console, IOTJS_MAGIC_STRING_STDOUT, Stdout);
  iotjs_jval_set_method(console, IOTJS_MAGIC_STRING_STDERR, Stderr);

  return console;
}
