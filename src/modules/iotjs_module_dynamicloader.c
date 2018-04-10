/* Copyright 2018-present Samsung Electronics Co., Ltd. and other contributors
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

#include <stdlib.h>

#include "iotjs_def.h"

#include <dlfcn.h>

#define XSTR(ARG) #ARG
#define STR(ARG) XSTR(ARG)


jerry_value_t iotjs_load_module(const char* path) {
  if (path == NULL) {
    const char* error = "Invalid module path";
    return jerry_create_error(JERRY_ERROR_TYPE, (const jerry_char_t*)error);
  }

  void* dynamic_lib = dlopen(path, RTLD_NOW);

  if (dynamic_lib == NULL) {
    const char* error = "Can not open module";
    return jerry_create_error(JERRY_ERROR_TYPE, (const jerry_char_t*)error);
  }

  void* loader_function = dlsym(dynamic_lib, STR(IOTJS_MODULE_ENTRYPOINT));
  if (loader_function == NULL) {
    dlclose(dynamic_lib);
    const char* error = "Entrypoint not found in module";
    return jerry_create_error(JERRY_ERROR_TYPE, (const jerry_char_t*)error);
  }

  const iotjs_module* module =
      (const iotjs_module*)((iotjs_module_info_getter)loader_function)();

  if (module == NULL) {
    dlclose(dynamic_lib);
    const char* error = "Invalid module info";
    return jerry_create_error(JERRY_ERROR_TYPE, (const jerry_char_t*)error);
  }

  if (module->iotjs_module_version != IOTJS_CURRENT_MODULE_VERSION) {
    dlclose(dynamic_lib);
    const char* error = "Incorrect version requested in the module";
    return jerry_create_error(JERRY_ERROR_TYPE, (const jerry_char_t*)error);
  }

  return module->initializer();
}


JS_FUNCTION(DLload) {
  DJS_CHECK_ARGS(1, string);

  iotjs_string_t file = JS_GET_ARG(0, string);
  const char* filename = iotjs_string_data(&file);

  jerry_value_t jresult = iotjs_load_module(filename);

  iotjs_string_destroy(&file);

  return jresult;
}


jerry_value_t InitDynamicloader() {
  jerry_value_t loader = jerry_create_external_function(DLload);
  return loader;
}
