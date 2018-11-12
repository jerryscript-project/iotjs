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

static napi_value get_property(napi_env env, napi_callback_info info) {
  size_t argc = 2;
  napi_value args[2];
  NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, NULL, NULL));

  NAPI_ASSERT(env, argc >= 2, "Wrong number of arguments");

  napi_valuetype valuetype0;
  NAPI_CALL(env, napi_typeof(env, args[0], &valuetype0));

  NAPI_ASSERT(env, valuetype0 == napi_object,
              "Wrong type of arguments. Expects an object as first argument.");

  napi_valuetype valuetype1;
  NAPI_CALL(env, napi_typeof(env, args[1], &valuetype1));

  NAPI_ASSERT(env, valuetype1 == napi_string,
              "Wrong type of arguments. Expects a string as second argument.");

  napi_value object = args[0];
  napi_value output;
  NAPI_CALL(env, napi_get_property(env, object, args[1], &output));

  return output;
}

static napi_value set_property(napi_env env, napi_callback_info info) {
  size_t argc = 3;
  napi_value args[3];
  NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, NULL, NULL));

  NAPI_ASSERT(env, argc >= 3, "Wrong number of arguments");

  napi_valuetype valuetype0;
  NAPI_CALL(env, napi_typeof(env, args[0], &valuetype0));

  NAPI_ASSERT(env, valuetype0 == napi_object,
              "Wrong type of arguments. Expects an object as first argument.");

  napi_valuetype valuetype1;
  NAPI_CALL(env, napi_typeof(env, args[1], &valuetype1));

  NAPI_ASSERT(env, valuetype1 == napi_string,
              "Wrong type of arguments. Expects a string as second argument.");

  NAPI_CALL(env, napi_set_property(env, args[0], args[1], args[2]));

  napi_value valuetrue;
  NAPI_CALL(env, napi_get_boolean(env, true, &valuetrue));

  return valuetrue;
}

static napi_value has_property(napi_env env, napi_callback_info info) {
  size_t argc = 2;
  napi_value args[2];
  NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, NULL, NULL));

  NAPI_ASSERT(env, argc >= 2, "Wrong number of arguments");

  napi_valuetype valuetype0;
  NAPI_CALL(env, napi_typeof(env, args[0], &valuetype0));

  NAPI_ASSERT(env, valuetype0 == napi_object,
              "Wrong type of arguments. Expects an object as first argument.");

  napi_valuetype valuetype1;
  NAPI_CALL(env, napi_typeof(env, args[1], &valuetype1));

  NAPI_ASSERT(env, valuetype1 == napi_string,
              "Wrong type of arguments. Expects a string as second argument.");

  bool has_property;
  NAPI_CALL(env, napi_has_property(env, args[0], args[1], &has_property));

  napi_value ret;
  NAPI_CALL(env, napi_get_boolean(env, has_property, &ret));

  return ret;
}

static napi_value has_own_property(napi_env env, napi_callback_info info) {
  size_t argc = 2;
  napi_value args[2];
  NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, NULL, NULL));

  NAPI_ASSERT(env, argc == 2, "Wrong number of arguments");

  napi_valuetype valuetype0;
  NAPI_CALL(env, napi_typeof(env, args[0], &valuetype0));

  NAPI_ASSERT(env, valuetype0 == napi_object,
              "Wrong type of arguments. Expects an object as first argument.");

  napi_valuetype valuetype1;
  NAPI_CALL(env, napi_typeof(env, args[1], &valuetype1));

  NAPI_ASSERT(env, valuetype1 == napi_string,
              "Wrong type of arguments. Expects a string as second argument.");

  bool has_property;
  NAPI_CALL(env, napi_has_own_property(env, args[0], args[1], &has_property));

  napi_value ret;
  NAPI_CALL(env, napi_get_boolean(env, has_property, &ret));

  return ret;
}

static napi_value get_property_names(napi_env env, napi_callback_info info) {
  size_t argc = 1;
  napi_value args[1];
  NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, NULL, NULL));

  NAPI_ASSERT(env, argc >= 1, "Wrong number of arguments");

  napi_valuetype valuetype0;
  NAPI_CALL(env, napi_typeof(env, args[0], &valuetype0));

  NAPI_ASSERT(env, valuetype0 == napi_object,
              "Wrong type of arguments. Expects an object as first argument.");

  napi_value obj = args[0];
  napi_value propertynames;
  NAPI_CALL(env, napi_get_property_names(env, obj, &propertynames));
  return propertynames;
}

static napi_value delete_property(napi_env env, napi_callback_info info) {
  size_t argc = 2;
  napi_value args[2];

  NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, NULL, NULL));
  NAPI_ASSERT(env, argc == 2, "Wrong number of arguments");

  napi_valuetype valuetype0;
  NAPI_CALL(env, napi_typeof(env, args[0], &valuetype0));
  NAPI_ASSERT(env, valuetype0 == napi_object,
              "Wrong type of arguments. Expects an object as first argument.");

  napi_valuetype valuetype1;
  NAPI_CALL(env, napi_typeof(env, args[1], &valuetype1));
  NAPI_ASSERT(env, valuetype1 == napi_string,
              "Wrong type of arguments. Expects a string as second argument.");

  bool result;
  napi_value ret;
  NAPI_CALL(env, napi_delete_property(env, args[0], args[1], &result));
  NAPI_CALL(env, napi_get_boolean(env, result, &ret));

  return ret;
}

static napi_value init(napi_env env, napi_value exports) {
  napi_property_descriptor descriptors[] = {
    DECLARE_NAPI_PROPERTY("GetProperty", get_property),
    DECLARE_NAPI_PROPERTY("SetProperty", set_property),
    DECLARE_NAPI_PROPERTY("HasProperty", has_property),
    DECLARE_NAPI_PROPERTY("HasOwnProperty", has_own_property),
    DECLARE_NAPI_PROPERTY("GetNames", get_property_names),
    DECLARE_NAPI_PROPERTY("DeleteProperty", delete_property),
  };

  NAPI_CALL(env, napi_define_properties(env, exports, sizeof(descriptors) /
                                                          sizeof(*descriptors),
                                        descriptors));

  return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, init)
