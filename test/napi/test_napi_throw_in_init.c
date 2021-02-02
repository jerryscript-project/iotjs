#include <assert.h>
#include <node_api.h>
#include <stdio.h>

napi_value Init(napi_env env, napi_value exports) {
  napi_throw_error(env, NULL,
                   "Directly raise exception when import napi native module");
  return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)
