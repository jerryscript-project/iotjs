#include <node_api.h>
#include "common.h"

static napi_value Throw(napi_env env, napi_callback_info info) {
  size_t argc = 1;
  napi_value argv[1];
  NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));

  NAPI_CALL(env, napi_throw(env, argv[0]));

  return NULL;
}

static napi_value Return(napi_env env, napi_callback_info info) {
  size_t argc = 1;
  napi_value argv[1];
  NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));

  return argv[0];
}

static napi_value ReturnThis(napi_env env, napi_callback_info info) {
  napi_value this;
  NAPI_CALL(env, napi_get_cb_info(env, info, NULL, NULL, &this, NULL));

  return this;
}

static napi_value Init(napi_env env, napi_value exports) {
  SET_NAMED_METHOD(env, exports, "Throw", Throw);
  SET_NAMED_METHOD(env, exports, "Return", Return);
  SET_NAMED_METHOD(env, exports, "ReturnThis", ReturnThis);

  return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)
