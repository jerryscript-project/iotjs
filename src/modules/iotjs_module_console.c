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

// This function should be able to print utf8 encoded string
// as utf8 is internal string representation in Jerryscript
static void Print(iotjs_jhandler_t* jhandler, FILE* out_fd) {
  JHANDLER_CHECK_ARGS(1, string);

  iotjs_string_t msg = JHANDLER_GET_ARG(0, string);
  const char* str = iotjs_string_data(&msg);
  unsigned str_len = iotjs_string_size(&msg);
  unsigned idx = 0;

  for (idx = 0; idx < str_len; idx++) {
    if (str[idx] != 0) {
      fprintf(out_fd, "%c", str[idx]);
    } else {
      fprintf(out_fd, "\\u0000");
    }
  }
  iotjs_string_destroy(&msg);
}


JHANDLER_FUNCTION(Stdout) {
  Print(jhandler, stdout);
}


JHANDLER_FUNCTION(Stderr) {
  Print(jhandler, stderr);
}


iotjs_jval_t InitConsole() {
  iotjs_jval_t console = iotjs_jval_create_object();

  iotjs_jval_set_method(&console, IOTJS_MAGIC_STRING_STDOUT, Stdout);
  iotjs_jval_set_method(&console, IOTJS_MAGIC_STRING_STDERR, Stderr);

  return console;
}
