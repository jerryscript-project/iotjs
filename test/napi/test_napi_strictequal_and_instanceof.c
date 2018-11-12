#include <node_api.h>
#include <stdio.h>
#include <stdlib.h>
#include "common.h"

napi_value SayHello(napi_env env, napi_callback_info info) {
  size_t argc = 0;
  // test if `napi_get_cb_info` tolerants NULL pointers.
  NAPI_CALL(env, napi_get_cb_info(env, info, &argc, NULL, NULL, NULL));

  napi_value str;
  NAPI_CALL(env, napi_create_string_utf8(env, "Hello", 5, &str));

  return str;
}

napi_value SayError(napi_env env, napi_callback_info info) {
  NAPI_CALL(env, napi_throw_error(env, "foo", "bar"));

  return NULL;
}

napi_value StrictEquals(napi_env env, napi_callback_info info) {
  size_t argc = 2;
  napi_value argv[2];
  napi_value thisArg;
  void* data;
  NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisArg, &data));

  bool result = false;
  NAPI_CALL(env, napi_strict_equals(env, argv[0], argv[1], &result));

  napi_value ret;
  NAPI_CALL(env, napi_get_boolean(env, result, &ret));

  return ret;
}

napi_value Instanceof(napi_env env, napi_callback_info info) {
  size_t argc = 2;
  napi_value argv[2];
  napi_value thisArg;
  void* data;
  NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisArg, &data));

  bool result = false;
  NAPI_CALL(env, napi_instanceof(env, argv[0], argv[1], &result));

  napi_value ret;
  NAPI_CALL(env, napi_get_boolean(env, result, &ret));

  return ret;
}

napi_value Init(napi_env env, napi_value exports) {
  SET_NAMED_METHOD(env, exports, "sayHello", SayHello);
  SET_NAMED_METHOD(env, exports, "sayError", SayError);
  SET_NAMED_METHOD(env, exports, "strictEquals", StrictEquals);
  SET_NAMED_METHOD(env, exports, "instanceof", Instanceof);

  napi_value id;
  NAPI_CALL(env, napi_create_int32(env, 321, &id));
  NAPI_CALL(env, napi_set_named_property(env, exports, "id", id));

  return exports;
}

NAPI_MODULE(napi_test, Init);
