#include <node_api.h>
#include "common.h"

static napi_ref ConstructRef;

static void cleanup(void* data) {
  napi_env env = (napi_env)data;
  napi_delete_reference(env, ConstructRef);
}

napi_value Construct(napi_env env, napi_callback_info info) {
  size_t argc = 1;
  napi_value argv[1];
  napi_value this;
  NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &this, NULL));

  NAPI_CALL(env, napi_set_named_property(env, this, "value", argv[0]));

  return NULL;
}

napi_value Constructor(napi_env env, napi_callback_info info) {
  napi_value construct;
  NAPI_CALL(env, napi_get_reference_value(env, ConstructRef, &construct));

  size_t argc = 1;
  napi_value argv[1];
  napi_value result;
  NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));

  NAPI_CALL(env, napi_new_instance(env, construct, argc, argv, &result));
  return result;
}

NAPI_MODULE_INIT() {
  napi_value construct;
  NAPI_CALL(env, napi_create_function(env, "Constructor", NAPI_AUTO_LENGTH,
                                      Construct, NULL, &construct));
  NAPI_CALL(env, napi_create_reference(env, construct, 1, &ConstructRef));
  NAPI_CALL(env, napi_add_env_cleanup_hook(env, cleanup, env));

  SET_NAMED_METHOD(env, exports, "Constructor", Constructor);
  return exports;
}
