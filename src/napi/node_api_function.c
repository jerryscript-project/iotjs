/* Copyright 2019-present Samsung Electronics Co., Ltd. and other contributors
 * Copyright 2018-present Rokid Co., Ltd. and other contributors
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

#include "jerryscript-ext/handle-scope.h"
#include "jerryscript.h"
#include <stdlib.h>
#include "internal/node_api_internal.h"
#include "node_api.h"

static jerry_value_t iotjs_napi_function_handler(
    const jerry_value_t func_obj, const jerry_value_t this_val,
    const jerry_value_t args_p[], const jerry_length_t args_cnt) {
  iotjs_function_info_t* function_info = (iotjs_function_info_t*)
      iotjs_try_get_object_native_info(func_obj, sizeof(iotjs_function_info_t));
  IOTJS_ASSERT(function_info != NULL);

  napi_env env = function_info->env;

  jerryx_handle_scope scope;
  jerryx_open_handle_scope(&scope);

  iotjs_callback_info_t* callback_info = IOTJS_ALLOC(iotjs_callback_info_t);
  callback_info->argc = args_cnt;
  callback_info->argv = (jerry_value_t*)args_p;
  callback_info->jval_this = this_val;
  callback_info->jval_func = func_obj;
  callback_info->function_info = function_info;

  callback_info->handle_scope = scope;
  callback_info->function_info = function_info;

  iotjs_napi_set_current_callback(env, callback_info);
  napi_value nvalue_ret =
      function_info->cb(env, (napi_callback_info)callback_info);
  iotjs_napi_set_current_callback(env, NULL);
  IOTJS_RELEASE(callback_info);

  jerry_value_t jval_ret;
  if (iotjs_napi_is_exception_pending(env)) {
    jerry_value_t jval_err = iotjs_napi_env_get_and_clear_exception(env);
    if (jval_err != (uintptr_t)NULL) {
      jval_ret = jval_err;
    } else {
      jval_err = iotjs_napi_env_get_and_clear_fatal_exception(env);
      IOTJS_ASSERT(jval_err != (uintptr_t)NULL);

      jval_ret = jval_err;
    }

    goto cleanup;
  }

  // TODO: check if nvalue_ret is escaped
  /**
   * Do not turn NULL pointer into undefined since number value `0` in
   * jerryscript also represented by NULL
   */
  jval_ret = AS_JERRY_VALUE(nvalue_ret);
  /**
   * - for N-API created value: value is scoped, would be released on :cleanup
   * - for passed-in params: value would be automatically release on end of
   * invocation
   * - for error values: error values has been acquired on thrown
   */
  jval_ret = jerry_acquire_value(jval_ret);

cleanup:
  jerryx_close_handle_scope(scope);
  /**
   * Clear N-API env extended error info on end of external function
   * execution to prevent error info been passed to next external function.
   */
  iotjs_napi_clear_error_info(env);
  return jval_ret;
}

napi_status napi_create_function(napi_env env, const char* utf8name,
                                 size_t length, napi_callback cb, void* data,
                                 napi_value* result) {
  NAPI_TRY_ENV(env);
  jerry_value_t jval_func =
      jerry_create_external_function(iotjs_napi_function_handler);
  jerryx_create_handle(jval_func);

  iotjs_function_info_t* function_info = (iotjs_function_info_t*)
      iotjs_get_object_native_info(jval_func, sizeof(iotjs_function_info_t));
  function_info->env = env;
  function_info->cb = cb;
  function_info->data = data;

  return napi_assign_nvalue(jval_func, result);
}

napi_status napi_call_function(napi_env env, napi_value recv, napi_value func,
                               size_t argc, const napi_value* argv,
                               napi_value* result) {
  NAPI_TRY_ENV(env);
  NAPI_TRY_NO_PENDING_EXCEPTION(env);

  jerry_value_t jval_func = AS_JERRY_VALUE(func);
  jerry_value_t jval_this = AS_JERRY_VALUE(recv);

  NAPI_TRY_TYPE(function, jval_func);

  jerry_value_t* jval_argv = IOTJS_CALLOC(argc, jerry_value_t);
  for (size_t idx = 0; idx < argc; ++idx) {
    jval_argv[idx] = AS_JERRY_VALUE(argv[idx]);
  }
  JERRYX_CREATE(jval_ret,
                jerry_call_function(jval_func, jval_this, jval_argv, argc));
  IOTJS_RELEASE(jval_argv);

  if (jerry_value_is_error(jval_ret)) {
    NAPI_INTERNAL_CALL(napi_throw(env, AS_NAPI_VALUE(jval_ret)));
    NAPI_RETURN_WITH_MSG(napi_pending_exception,
                         "Unexpected error flag on jerry_call_function.");
  }

  return napi_assign_nvalue(jval_ret, result);
}

napi_status napi_get_cb_info(napi_env env, napi_callback_info cbinfo,
                             size_t* argc, napi_value* argv,
                             napi_value* thisArg, void** data) {
  NAPI_TRY_ENV(env);
  iotjs_callback_info_t* callback_info = (iotjs_callback_info_t*)cbinfo;

  size_t _argc = (argc == NULL || argv == NULL) ? 0 : *argc;
  for (size_t i = 0; i < _argc; ++i) {
    if (i < callback_info->argc) {
      NAPI_ASSIGN(argv + i, AS_NAPI_VALUE(callback_info->argv[i]));
    } else {
      NAPI_ASSIGN(argv + i, AS_NAPI_VALUE(jerry_create_undefined()));
    }
  }
  NAPI_ASSIGN(argc, callback_info->argc);

  if (thisArg != NULL) {
    NAPI_ASSIGN(thisArg, AS_NAPI_VALUE(callback_info->jval_this));
  }

  if (data != NULL) {
    NAPI_ASSIGN(data, callback_info->function_info->data);
  }

  NAPI_RETURN(napi_ok);
}

napi_status napi_get_new_target(napi_env env, napi_callback_info cbinfo,
                                napi_value* result) {
  iotjs_callback_info_t* callback_info = (iotjs_callback_info_t*)cbinfo;
  jerry_value_t jval_this = callback_info->jval_this;
  jerry_value_t jval_target = callback_info->jval_func;
  jerry_value_t is_instance =
      jerry_binary_operation(JERRY_BIN_OP_INSTANCEOF, jval_this, jval_target);
  if (jerry_value_is_error(is_instance)) {
    jerry_release_value(is_instance);
    NAPI_ASSIGN(result, NULL);
    NAPI_RETURN(napi_generic_failure);
  }

  NAPI_ASSIGN(result, jerry_get_boolean_value(is_instance)
                          ? AS_NAPI_VALUE(jval_target)
                          : NULL);
  NAPI_RETURN(napi_ok);
}

napi_status napi_new_instance(napi_env env, napi_value constructor, size_t argc,
                              const napi_value* argv, napi_value* result) {
  NAPI_TRY_ENV(env);
  NAPI_TRY_NO_PENDING_EXCEPTION(env);

  jerry_value_t jval_cons = AS_JERRY_VALUE(constructor);

  NAPI_TRY_TYPE(function, jval_cons);

  jerry_value_t* jval_argv = IOTJS_CALLOC(argc, jerry_value_t);
  for (size_t idx = 0; idx < argc; ++idx) {
    jval_argv[idx] = AS_JERRY_VALUE(argv[idx]);
  }

  JERRYX_CREATE(jval_ret, jerry_construct_object(jval_cons, jval_argv, argc));
  IOTJS_RELEASE(jval_argv);

  if (jerry_value_is_error(jval_ret)) {
    NAPI_INTERNAL_CALL(napi_throw(env, AS_NAPI_VALUE(jval_ret)));
    NAPI_RETURN_WITH_MSG(napi_pending_exception,
                         "Unexpected error flag on jerry_construct_object.");
  }

  return napi_assign_nvalue(jval_ret, result);
}
