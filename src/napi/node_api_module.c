/* Copyright 2018-present Rokid Co., Ltd. and other contributors
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

#include "iotjs.h"
#include "jerryscript-ext/handle-scope.h"
#include "internal/node_api_internal.h"

static napi_module* mod_pending;

void napi_module_register(napi_module* mod) {
  mod_pending = mod;
}

int napi_module_init_pending(jerry_value_t* exports) {
  if (mod_pending == NULL) {
    return napi_module_no_pending;
  }

  napi_addon_register_func init =
      (napi_addon_register_func)mod_pending->nm_register_func;

  if (init == NULL) {
    return napi_module_no_nm_register_func;
  }

  napi_env env = iotjs_get_current_napi_env();

  jerryx_handle_scope scope;
  jerryx_open_handle_scope(&scope);

  jerry_value_t jval_exports = jerry_create_object();
  napi_value nvalue_ret = (*init)(env, AS_NAPI_VALUE(jval_exports));

  if (nvalue_ret == NULL) {
    *exports = jerry_create_undefined();
    jerry_release_value(jval_exports);
  } else {
    jerry_value_t jval_ret = AS_JERRY_VALUE(nvalue_ret);
    if (jval_ret != jval_exports) {
      jerry_release_value(jval_exports);
      jerryx_remove_handle(scope, jval_ret, &jval_ret);
    }
    *exports = jval_ret;
  }

  jerryx_close_handle_scope(scope);

  mod_pending = NULL;

  if (iotjs_napi_is_exception_pending(env)) {
    jerry_value_t jval_err;
    jval_err = iotjs_napi_env_get_and_clear_exception(env);
    if (jval_err == (uintptr_t)NULL) {
      jval_err = iotjs_napi_env_get_and_clear_fatal_exception(env);
    }
    jerry_release_value(jval_exports);
    *exports = jval_err;
    return napi_pending_exception;
  }
  return napi_module_load_ok;
}
