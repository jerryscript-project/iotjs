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

#include "uv.h"
#include "internal/node_api_internal.h"

static void iotjs_uv_work_cb(uv_work_t* req) {
  iotjs_async_work_t* async_work = (iotjs_async_work_t*)req->data;
  IOTJS_ASSERT(async_work != NULL);
  if (async_work->execute != NULL) {
    async_work->execute(async_work->env, async_work->data);
  }
}

static void iotjs_uv_work_after_cb(uv_work_t* req, int status) {
  iotjs_async_work_t* async_work = (iotjs_async_work_t*)req->data;
  IOTJS_ASSERT(async_work != NULL);
  napi_status cb_status;
  if (status == 0) {
    cb_status = napi_ok;
  } else if (status == UV_ECANCELED) {
    cb_status = napi_cancelled;
  } else {
    cb_status = napi_generic_failure;
  }

  napi_env env = async_work->env;
  napi_async_complete_callback complete = async_work->complete;
  void* data = async_work->data;

  if (complete != NULL) {
    jerryx_handle_scope scope;
    jerryx_open_handle_scope(&scope);
    /**
     * napi_async_work could be deleted by invocation of `napi_delete_asyncwork`
     * in its complete callback.
     */
    complete(env, cb_status, data);
    jerryx_close_handle_scope(scope);

    if (iotjs_napi_is_exception_pending(env)) {
      jerry_value_t jval_err;
      jval_err = iotjs_napi_env_get_and_clear_exception(env);
      if (jval_err == (uintptr_t)NULL) {
        jval_err = iotjs_napi_env_get_and_clear_fatal_exception(env);
      }

      /** Argument cannot have error flag */
      iotjs_uncaught_exception(jerry_get_value_from_error(jval_err, false));
      jerry_release_value(jval_err);
    }
  }

  iotjs_process_next_tick();
}

napi_status napi_create_async_work(napi_env env, napi_value async_resource,
                                   napi_value async_resource_name,
                                   napi_async_execute_callback execute,
                                   napi_async_complete_callback complete,
                                   void* data, napi_async_work* result) {
  NAPI_TRY_ENV(env);
  NAPI_WEAK_ASSERT(napi_invalid_arg, result != NULL);
  NAPI_WEAK_ASSERT(napi_invalid_arg, execute != NULL);
  NAPI_WEAK_ASSERT(napi_invalid_arg, complete != NULL);

  iotjs_async_work_t* async_work = IOTJS_ALLOC(iotjs_async_work_t);
  uv_work_t* work_req = &async_work->work_req;

  async_work->env = env;
  async_work->async_resource = async_resource;
  async_work->async_resource_name = async_resource_name;
  async_work->execute = execute;
  async_work->complete = complete;
  async_work->data = data;

  work_req->data = async_work;

  NAPI_ASSIGN(result, (napi_async_work)work_req);
  NAPI_RETURN(napi_ok);
}

napi_status napi_delete_async_work(napi_env env, napi_async_work work) {
  NAPI_TRY_ENV(env);
  uv_work_t* work_req = (uv_work_t*)work;
  iotjs_async_work_t* async_work = (iotjs_async_work_t*)work_req->data;
  IOTJS_RELEASE(async_work);
  NAPI_RETURN(napi_ok);
}

napi_status napi_queue_async_work(napi_env env, napi_async_work work) {
  NAPI_TRY_ENV(env);
  iotjs_environment_t* iotjs_env = iotjs_environment_get();
  uv_loop_t* loop = iotjs_environment_loop(iotjs_env);

  uv_work_t* work_req = (uv_work_t*)work;

  int status =
      uv_queue_work(loop, work_req, iotjs_uv_work_cb, iotjs_uv_work_after_cb);
  if (status != 0) {
    const char* err_name = uv_err_name(status);
    NAPI_RETURN_WITH_MSG(napi_generic_failure, err_name);
  }
  NAPI_RETURN(napi_ok);
}

napi_status napi_cancel_async_work(napi_env env, napi_async_work work) {
  NAPI_TRY_ENV(env);
  uv_work_t* work_req = (uv_work_t*)work;
  int status = uv_cancel((uv_req_t*)work_req);
  if (status != 0) {
    const char* err_name = uv_err_name(status);
    NAPI_RETURN_WITH_MSG(napi_generic_failure, err_name);
  }
  NAPI_RETURN(napi_ok);
}

napi_status napi_async_init(napi_env env, napi_value async_resource,
                            napi_value async_resource_name,
                            napi_async_context* result) {
  NAPI_TRY_ENV(env);

  iotjs_async_context_t* ctx = IOTJS_ALLOC(iotjs_async_context_t);
  ctx->env = env;
  ctx->async_resource = async_resource;
  ctx->async_resource_name = async_resource_name;

  NAPI_ASSIGN(result, (napi_async_context)ctx);
  return napi_ok;
}

napi_status napi_async_destroy(napi_env env, napi_async_context async_context) {
  NAPI_TRY_ENV(env);

  iotjs_async_context_t* ctx = (iotjs_async_context_t*)async_context;
  IOTJS_RELEASE(ctx);

  return napi_ok;
}

napi_status napi_make_callback(napi_env env, napi_async_context async_context,
                               napi_value recv, napi_value func, size_t argc,
                               const napi_value* argv, napi_value* result) {
  NAPI_TRY_ENV(env);

  napi_status status = napi_call_function(env, recv, func, argc, argv, result);
  if (!iotjs_napi_is_exception_pending(env)) {
    iotjs_process_next_tick();
  } else {
    // In this case explicit napi_async_destroy calls won't be executed, so
    // free the context manually.
    IOTJS_RELEASE(async_context);
  }

  return status;
}
