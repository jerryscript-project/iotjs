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

#include "internal/node_api_internal.h"

#include "iotjs_def.h"

#if _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif
#include <stdlib.h>

JS_FUNCTION(open_native_module) {
  iotjs_string_t location = JS_GET_ARG(0, string);

#if _WIN32
  // Get a handle to the node module.
  HINSTANCE handle = LoadLibrary(iotjs_string_data(&location));
#else
  void* handle = dlopen(iotjs_string_data(&location), RTLD_LAZY);
#endif
  iotjs_string_destroy(&location);

  // If the handle is valid, try to get the function address.
  if (handle == NULL) {
#if _WIN32
    char* err_msg = "";
    DWORD dw = GetLastError();

    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                      FORMAT_MESSAGE_IGNORE_INSERTS,
                  NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), &err_msg,
                  0, NULL);
#else
    char* err_msg = dlerror();
#endif
    jerry_value_t jval_error =
        jerry_create_error(JERRY_ERROR_COMMON, (jerry_char_t*)err_msg);
    return jval_error;
  }

  jerry_value_t exports = jerry_create_undefined();

  int status = napi_module_init_pending(&exports);
  if (status == napi_module_load_ok) {
    return exports;
  }

  if (status == napi_pending_exception) {
/* exports is an error reference */
#if _WIN32
    FreeLibrary(handle);
#else
    dlclose(handle);
#endif
    return exports;
  }

  if (status == napi_module_no_nm_register_func) {
#if _WIN32
    FreeLibrary(handle);
#else
    dlclose(handle);
#endif
    jerry_value_t jval_error = jerry_create_error(
        JERRY_ERROR_COMMON,
        (jerry_char_t*)"Module has no declared entry point.");
    return jval_error;
  }

  return exports;
}

jerry_value_t iotjs_init_dynamicloader(void) {
  return jerry_create_external_function(open_native_module);
}
