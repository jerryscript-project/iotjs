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

#include <string.h>

static napi_value new_scope(napi_env env, napi_callback_info info) {
  napi_handle_scope scope;
  napi_value output = NULL;

  NAPI_CALL(env, napi_open_handle_scope(env, &scope));
  NAPI_CALL(env, napi_create_object(env, &output));
  NAPI_CALL(env, napi_close_handle_scope(env, scope));
  return NULL;
}

static napi_value new_scope_escape(napi_env env, napi_callback_info info) {
  napi_escapable_handle_scope scope;
  napi_value output = NULL;
  napi_value escapee = NULL;

  NAPI_CALL(env, napi_open_escapable_handle_scope(env, &scope));
  NAPI_CALL(env, napi_create_object(env, &output));
  NAPI_CALL(env, napi_escape_handle(env, scope, output, &escapee));
  NAPI_CALL(env, napi_close_escapable_handle_scope(env, scope));
  return escapee;
}

static napi_value new_scope_escape_twice(napi_env env,
                                         napi_callback_info info) {
  napi_escapable_handle_scope scope;
  napi_value output = NULL;
  napi_value escapee = NULL;
  napi_status status;

  NAPI_CALL(env, napi_open_escapable_handle_scope(env, &scope));
  NAPI_CALL(env, napi_create_object(env, &output));
  NAPI_CALL(env, napi_escape_handle(env, scope, output, &escapee));
  status = napi_escape_handle(env, scope, output, &escapee);
  NAPI_ASSERT(env, status == napi_escape_called_twice, "Escaping twice fails");
  NAPI_CALL(env, napi_close_escapable_handle_scope(env, scope));
  return NULL;
}

static napi_value new_scope_with_exception(napi_env env,
                                           napi_callback_info info) {
  napi_handle_scope scope;
  size_t argc;
  napi_value exception_function;
  napi_status status;
  napi_value output = NULL;

  NAPI_CALL(env, napi_open_handle_scope(env, &scope));
  NAPI_CALL(env, napi_create_object(env, &output));

  argc = 1;
  NAPI_CALL(env, napi_get_cb_info(env, info, &argc, &exception_function, NULL,
                                  NULL));

  status = napi_call_function(env, output, exception_function, 0, NULL, NULL);
  NAPI_ASSERT(env, status == napi_pending_exception,
              "Function should have thrown.");

  // Closing a handle scope should still work while an exception is pending.
  NAPI_CALL(env, napi_close_handle_scope(env, scope));
  return NULL;
}

static napi_value init(napi_env env, napi_value exports) {
  napi_property_descriptor properties[] = {
    DECLARE_NAPI_PROPERTY("NewScope", new_scope),
    DECLARE_NAPI_PROPERTY("NewScopeEscape", new_scope_escape),
    DECLARE_NAPI_PROPERTY("NewScopeEscapeTwice", new_scope_escape_twice),
    DECLARE_NAPI_PROPERTY("NewScopeWithException", new_scope_with_exception),
  };

  NAPI_CALL(env, napi_define_properties(env, exports, sizeof(properties) /
                                                          sizeof(*properties),
                                        properties));

  return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, init);
