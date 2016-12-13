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


static void Print(iotjs_jhandler_t* jhandler, FILE* out_fd) {
  JHANDLER_CHECK_ARGS(1, string);

  iotjs_string_t msg = JHANDLER_GET_ARG(0, string);
  fprintf(out_fd, "%s", iotjs_string_data(&msg));
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

  iotjs_jval_set_method(&console, "stdout", Stdout);
  iotjs_jval_set_method(&console, "stderr", Stderr);

  return console;
}
