#include <assert.h>
#include <node_api.h>
#include <stdio.h>
#include "common.h"

#define ERROR_CODE "ErrorCODE"
#define ERROR_MSG "ErrorMSG"

napi_value Throw(napi_env env, napi_callback_info info) {
  size_t argc = 1;
  napi_value argv[1];
  napi_status status;
  status = napi_get_cb_info(env, info, &argc, argv, NULL, NULL);
  assert(status == napi_ok);

  status = napi_throw(env, argv[0]);
  assert(status == napi_ok);

  return NULL;
}

napi_value ThrowError(napi_env env, napi_callback_info info) {
  napi_status status;

  status = napi_throw_error(env, ERROR_CODE, ERROR_MSG);
  assert(status == napi_ok);

  return NULL;
}

napi_value ThrowTypeError(napi_env env, napi_callback_info info) {
  napi_status status;

  status = napi_throw_type_error(env, ERROR_CODE, ERROR_MSG);
  assert(status == napi_ok);

  return NULL;
}

napi_value ThrowRangeError(napi_env env, napi_callback_info info) {
  napi_status status;

  status = napi_throw_range_error(env, ERROR_CODE, ERROR_MSG);
  assert(status == napi_ok);

  return NULL;
}

napi_value IsError(napi_env env, napi_callback_info info) {
  size_t argc = 1;
  napi_value argv[1];
  napi_value result;
  napi_status status;
  bool res;

  status = napi_get_cb_info(env, info, &argc, argv, NULL, NULL);
  assert(status == napi_ok);

  status = napi_is_error(env, argv[0], &res);
  assert(status == napi_ok);

  status = napi_get_boolean(env, res, &result);
  assert(status == napi_ok);

  return result;
}

napi_value CreateError(napi_env env, napi_callback_info info) {
  size_t argc = 2;
  napi_value argv[2];
  napi_value result;
  napi_status status;

  status = napi_get_cb_info(env, info, &argc, argv, NULL, NULL);
  assert(status == napi_ok);

  status = napi_create_error(env, argv[0], argv[1], &result);
  assert(status == napi_ok);

  return result;
}

napi_value CreateTypeError(napi_env env, napi_callback_info info) {
  size_t argc = 2;
  napi_value argv[2];
  napi_value result;
  napi_status status;

  status = napi_get_cb_info(env, info, &argc, argv, NULL, NULL);
  assert(status == napi_ok);

  status = napi_create_type_error(env, argv[0], argv[1], &result);
  assert(status == napi_ok);

  return result;
}

napi_value CreateRangeError(napi_env env, napi_callback_info info) {
  size_t argc = 2;
  napi_value argv[2];
  napi_value result;
  napi_status status;

  status = napi_get_cb_info(env, info, &argc, argv, NULL, NULL);
  assert(status == napi_ok);

  status = napi_create_range_error(env, argv[0], argv[1], &result);
  assert(status == napi_ok);

  return result;
}

napi_value GetandClearLastException(napi_env env, napi_callback_info info) {
  napi_status status;
  napi_value result;

  status = napi_get_and_clear_last_exception(env, &result);
  assert(status == napi_ok);

  return result;
}

napi_value IsExceptionPending(napi_env env, napi_callback_info info) {
  napi_status status;
  bool res;
  napi_value result;

  status = napi_is_exception_pending(env, &res);
  assert(status == napi_ok);

  status = napi_get_boolean(env, res, &result);
  assert(status == napi_ok);

  return result;
}

napi_value FatalException(napi_env env, napi_callback_info info) {
  napi_status status;
  size_t argc = 1;
  napi_value argv[1];

  status = napi_get_cb_info(env, info, &argc, argv, NULL, NULL);
  assert(status == napi_ok);

  status = napi_fatal_exception(env, argv[0]);
  assert(status == napi_ok);

  return NULL;
}


napi_value Init(napi_env env, napi_value exports) {
  SET_NAMED_METHOD(env, exports, "Throw", Throw);
  SET_NAMED_METHOD(env, exports, "ThrowError", ThrowError);
  SET_NAMED_METHOD(env, exports, "ThrowTypeError", ThrowTypeError);
  SET_NAMED_METHOD(env, exports, "ThrowRangeError", ThrowRangeError);
  SET_NAMED_METHOD(env, exports, "IsError", IsError);
  SET_NAMED_METHOD(env, exports, "CreateError", CreateError);
  SET_NAMED_METHOD(env, exports, "CreateTypeError", CreateTypeError);
  SET_NAMED_METHOD(env, exports, "CreateRangeError", CreateRangeError);
  SET_NAMED_METHOD(env, exports, "GetandClearLastException",
                   GetandClearLastException);
  SET_NAMED_METHOD(env, exports, "IsExceptionPending", IsExceptionPending);
  SET_NAMED_METHOD(env, exports, "FatalException", FatalException);

  return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)
