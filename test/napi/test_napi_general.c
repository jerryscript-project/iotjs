/* Copyright 2019-present Samsung Electronics Co., Ltd. and other contributors
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

#include "common.h"
#include "node_api.h"

static napi_value get_null(napi_env env, napi_callback_info info) {
  napi_value result;
  NAPI_CALL(env, napi_get_null(env, &result));
  return result;
}

static napi_value get_undefined(napi_env env, napi_callback_info info) {
  napi_value result;
  NAPI_CALL(env, napi_get_undefined(env, &result));
  return result;
}

static napi_value test_typeof(napi_env env, napi_callback_info info) {
  size_t argc = 1;
  napi_value args[1];
  NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, NULL, NULL));

  napi_valuetype argument_type;
  NAPI_CALL(env, napi_typeof(env, args[0], &argument_type));

  napi_value result = NULL;
  if (argument_type == napi_number) {
    NAPI_CALL(env, napi_create_string_utf8(env, "number", NAPI_AUTO_LENGTH,
                                           &result));
  } else if (argument_type == napi_string) {
    NAPI_CALL(env, napi_create_string_utf8(env, "string", NAPI_AUTO_LENGTH,
                                           &result));
  } else if (argument_type == napi_function) {
    NAPI_CALL(env, napi_create_string_utf8(env, "function", NAPI_AUTO_LENGTH,
                                           &result));
  } else if (argument_type == napi_object) {
    NAPI_CALL(env, napi_create_string_utf8(env, "object", NAPI_AUTO_LENGTH,
                                           &result));
  } else if (argument_type == napi_boolean) {
    NAPI_CALL(env, napi_create_string_utf8(env, "boolean", NAPI_AUTO_LENGTH,
                                           &result));
  } else if (argument_type == napi_undefined) {
    NAPI_CALL(env, napi_create_string_utf8(env, "undefined", NAPI_AUTO_LENGTH,
                                           &result));
  } else if (argument_type == napi_symbol) {
    NAPI_CALL(env, napi_create_string_utf8(env, "symbol", NAPI_AUTO_LENGTH,
                                           &result));
  } else if (argument_type == napi_null) {
    NAPI_CALL(env,
              napi_create_string_utf8(env, "null", NAPI_AUTO_LENGTH, &result));
  }
  return result;
}

static napi_value init(napi_env env, napi_value exports) {
  napi_property_descriptor descriptors[] = {
    DECLARE_NAPI_PROPERTY("GetNull", get_null),
    DECLARE_NAPI_PROPERTY("GetUndefined", get_undefined),
    DECLARE_NAPI_PROPERTY("TypeOf", test_typeof),
  };

  NAPI_CALL(env, napi_define_properties(env, exports, sizeof(descriptors) /
                                                          sizeof(*descriptors),
                                        descriptors));

  return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, init)
