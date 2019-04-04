#include <node_api.h>
#include <stdlib.h>
#include "common.h"

static size_t native_counter = 0;
static size_t native_hint = 0x8888;

static void finalize(napi_env env, void* finalize_data, void* finalize_hint) {
  size_t* f_data = (size_t*)finalize_data;
  size_t* f_hint = (size_t*)finalize_hint;
  if (*f_hint != native_hint)
    napi_fatal_error(__FILE__, NAPI_AUTO_LENGTH, "finalize hint not aligned.",
                     NAPI_AUTO_LENGTH);
  *f_data += 1;
}

static void cleanup(void* data) {
  if (native_counter == 0) {
    napi_fatal_error(__FILE__, NAPI_AUTO_LENGTH, "finalize not invoked.",
                     NAPI_AUTO_LENGTH);
  }
}

napi_value Wrap(napi_env env, napi_callback_info info) {
  size_t argc = 1;
  napi_value argv[1];
  NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));

  napi_ref weak_ref;
  NAPI_CALL(env, napi_wrap(env, argv[0], &native_counter, finalize,
                           &native_hint, &weak_ref));
  /**
   * `weak_ref` is a weak reference, so leave as it be.
   */
  return argv[0];
}

napi_value GetNativeCounter(napi_env env, napi_callback_info info) {
  napi_value count;
  NAPI_CALL(env, napi_create_uint32(env, native_counter, &count));
  return count;
}

NAPI_MODULE_INIT() {
  SET_NAMED_METHOD(env, exports, "Wrap", Wrap);
  SET_NAMED_METHOD(env, exports, "GetNativeCounter", GetNativeCounter);

  NAPI_CALL(env, napi_add_env_cleanup_hook(env, cleanup, NULL));
  return exports;
}
