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

#include <node_api.h>
#include <string.h>

static napi_value hello_world(napi_env env, napi_callback_info info) {
  napi_value world;
  const char* str = "Hello world!";
  size_t str_len = strlen(str);

  if (napi_create_string_utf8(env, str, str_len, &world) != napi_ok)
    return NULL;

  return world;
}

napi_value init_$MODULE_NAME$(napi_env env, napi_value exports) {
  napi_property_descriptor desc = { "hello", 0, hello_world,  0,
                                    0,       0, napi_default, 0 };

  if (napi_define_properties(env, exports, 1, &desc) != napi_ok)
    return NULL;

  return exports;
}

NAPI_MODULE($MODULE_NAME$, init_$MODULE_NAME$)
