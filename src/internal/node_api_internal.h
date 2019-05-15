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

#ifndef IOTJS_NODE_API_H
#define IOTJS_NODE_API_H

#include "iotjs_def.h"
#include "jerryscript-ext/handle-scope.h"
#include "jerryscript.h"
#include "internal/node_api_internal_types.h"
#include "node_api.h"

#define GET_4TH_ARG(arg1, arg2, arg3, arg4, ...) arg4

#define AS_JERRY_VALUE(nvalue) (jerry_value_t)(uintptr_t) nvalue
#define AS_NAPI_VALUE(jval) (napi_value)(uintptr_t) jval

/**
 * MARK: - N-API Returns machenism:
 * If any non-napi-ok status code is returned in N-API functions, there
 * should be an error code and error message temporarily stored in napi-env
 * and can be fetched by `napi_get_last_error_info` until next napi function
 * called.
 */
#define NAPI_RETURN_WITH_MSG(status, message)                                \
  do {                                                                       \
    iotjs_napi_set_error_info(iotjs_get_current_napi_env(), status, message, \
                              0, NULL);                                      \
    return status;                                                           \
  } while (0)

#define NAPI_RETURN(status)                                                  \
  do {                                                                       \
    iotjs_napi_set_error_info(iotjs_get_current_napi_env(), status, NULL, 0, \
                              NULL);                                         \
    return status;                                                           \
  } while (0)
/** MARK: - END N-API Returns */

/** MARK: - N-API Asserts */
/**
 * A weak assertion, which don't crash the program on failed assertion
 * rather returning a napi error code back to caller.
 */
#define NAPI_WEAK_ASSERT(error_t, assertion)                              \
  do {                                                                    \
    if (!(assertion))                                                     \
      NAPI_RETURN_WITH_MSG(error_t, "Assertion (" #assertion ") failed"); \
  } while (0)

#define NAPI_WEAK_ASSERT_WITH_MSG(error_t, assertion, message) \
  do {                                                         \
    if (!(assertion))                                          \
      NAPI_RETURN_WITH_MSG(error_t, message);                  \
  } while (0)

/**
 * A convenience weak assertion on jerry value type.
 */
#define NAPI_TRY_TYPE(type, jval)                        \
  NAPI_WEAK_ASSERT_WITH_MSG(napi_##type##_expected,      \
                            jerry_value_is_##type(jval), \
                            #type " was expected")

/**
 * A convenience weak assertion on N-API Env matching.
 */
#define NAPI_TRY_ENV(env)                                             \
  do {                                                                \
    if (napi_try_env_helper(env)) {                                   \
      NAPI_RETURN_WITH_MSG(napi_invalid_arg, "N-API env not match."); \
    }                                                                 \
  } while (0)

/**
 * A convenience weak assertion expecting there is no pending exception
 * unhandled.
 */
#define NAPI_TRY_NO_PENDING_EXCEPTION(env) \
  NAPI_WEAK_ASSERT(napi_pending_exception, \
                   !iotjs_napi_is_exception_pending(env))
/** MARK: - N-API Asserts */

/**
 * In most N-API functions, there is an in-out pointer parameter to retrieve
 * return values, yet this pointer could be NULL anyway - it has to be ensured
 * no value were unexpectedly written to NULL pointer.
 */
#define NAPI_ASSIGN(result, value) \
  if ((result) != NULL)            \
    *(result) = (value);

/**
 * A convenience macro to call N-API functions internally and handle it's
 * non-napi-ok status code.
 */
#define NAPI_INTERNAL_CALL(call) \
  do {                           \
    napi_status status = (call); \
    if (status != napi_ok) {     \
      NAPI_RETURN(status);       \
    }                            \
  } while (0)

/**
 * A convenience macro to create jerry-values and add it to current top
 * handle scope.
 */
#define JERRYX_CREATE(var, create) \
  jerry_value_t var = (create);    \
  jerryx_create_handle(var);

/** MARK: - node_api_module.c */
int napi_module_init_pending(jerry_value_t* exports);
/** MARK: - END node_api_module.c */

/** MARK: - node_api_env.c */
napi_env iotjs_get_current_napi_env(void);
bool napi_try_env_helper(napi_env env);
void iotjs_napi_set_current_callback(napi_env env,
                                     iotjs_callback_info_t* callback_info);
iotjs_callback_info_t* iotjs_napi_get_current_callback(napi_env env);

void iotjs_napi_set_error_info(napi_env env, napi_status error_code,
                               const char* error_message,
                               uint32_t engine_error_code,
                               void* engine_reserved);
void iotjs_napi_clear_error_info(napi_env env);

bool iotjs_napi_is_exception_pending(napi_env env);
jerry_value_t iotjs_napi_env_get_and_clear_exception(napi_env env);
jerry_value_t iotjs_napi_env_get_and_clear_fatal_exception(napi_env env);
/** MARK: - END node_api_env.c */

/** MARK: - node_api_lifetime.c */
napi_status jerryx_status_to_napi_status(jerryx_handle_scope_status status);
iotjs_object_info_t* iotjs_get_object_native_info(jerry_value_t jval,
                                                  size_t native_info_size);
iotjs_object_info_t* iotjs_try_get_object_native_info(jerry_value_t jval,
                                                      size_t native_info_size);
void iotjs_setup_napi(void);
void iotjs_cleanup_napi(void);
/** MARK: - END node_api_lifetime.c */

napi_status napi_assign_bool(bool value, bool* result);
napi_status napi_assign_nvalue(jerry_value_t jvalue, napi_value* nvalue);

#endif // IOTJS_NODE_API_H
