#include <node_api.h>
#include <stdio.h>
#include <stdlib.h>
#include "common.h"

static napi_value TestUtf8(napi_env env, napi_callback_info info) {
  size_t argc = 1;
  napi_value args[1];
  NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, NULL, NULL));

  NAPI_ASSERT(env, argc >= 1, "Wrong number of arguments");

  napi_valuetype valuetype;
  NAPI_CALL(env, napi_typeof(env, args[0], &valuetype));

  NAPI_ASSERT(env, valuetype == napi_string,
              "Wrong type of argment. Expects a string.");

  char buffer[128];
  size_t buffer_size = 128;
  size_t copied;

  NAPI_CALL(env, napi_get_value_string_utf8(env, args[0], buffer, buffer_size,
                                            &copied));

  napi_value output;
  NAPI_CALL(env, napi_create_string_utf8(env, buffer, copied, &output));

  return output;
}


static napi_value TestUtf8Insufficient(napi_env env, napi_callback_info info) {
  size_t argc = 1;
  napi_value args[1];
  NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, NULL, NULL));

  NAPI_ASSERT(env, argc >= 1, "Wrong number of arguments");

  napi_valuetype valuetype;
  NAPI_CALL(env, napi_typeof(env, args[0], &valuetype));

  NAPI_ASSERT(env, valuetype == napi_string,
              "Wrong type of argment. Expects a string.");

  char buffer[4];
  size_t buffer_size = 4;
  size_t copied;

  NAPI_CALL(env, napi_get_value_string_utf8(env, args[0], buffer, buffer_size,
                                            &copied));

  napi_value output;
  NAPI_CALL(env, napi_create_string_utf8(env, buffer, copied, &output));

  return output;
}

static napi_value Utf8Length(napi_env env, napi_callback_info info) {
  size_t argc = 1;
  napi_value args[1];
  NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, NULL, NULL));

  NAPI_ASSERT(env, argc >= 1, "Wrong number of arguments");

  napi_valuetype valuetype;
  NAPI_CALL(env, napi_typeof(env, args[0], &valuetype));

  NAPI_ASSERT(env, valuetype == napi_string,
              "Wrong type of argment. Expects a string.");

  size_t length;
  NAPI_CALL(env, napi_get_value_string_utf8(env, args[0], NULL, 0, &length));

  napi_value output;
  NAPI_CALL(env, napi_create_uint32(env, (uint32_t)length, &output));

  return output;
}

napi_value Init(napi_env env, napi_value exports) {
  SET_NAMED_METHOD(env, exports, "TestUtf8", TestUtf8);
  SET_NAMED_METHOD(env, exports, "TestUtf8Insufficient", TestUtf8Insufficient);
  SET_NAMED_METHOD(env, exports, "Utf8Length", Utf8Length);

  return exports;
}

NAPI_MODULE(napi_test, Init);
