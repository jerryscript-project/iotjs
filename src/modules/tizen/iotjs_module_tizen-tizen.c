/* Copyright 2018-present Samsung Electronics Co., Ltd. and other contributors
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

#include "iotjs_def.h"
#include "modules/iotjs_module_bridge.h"

#include <app_common.h>
#include <stdlib.h>

typedef enum {
  IOTJS_ERROR_RESULT_FAILED = INT_MIN,
  IOTJS_ERROR_INVALID_PARAMETER,
  IOTJS_ERROR_OUT_OF_MEMORY,
  IOTJS_ERROR_NONE = 0,
} iotjs_error_t;

// # tizen app-control
#include <app_control.h>
#include <app_control_internal.h>
#include <bundle.h>
#include <bundle_internal.h>

static iotjs_error_t tizen_send_launch_request(const char* json,
                                               void* hbridge) {
  DDDLOG("%s", __func__);

  bundle* b;
  int ret;

  ret = bundle_from_json(json, &b);
  if (ret != BUNDLE_ERROR_NONE) {
    DDLOG("bundle_from_json failed");
    return IOTJS_ERROR_INVALID_PARAMETER;
  }

  app_control_h app_control = NULL;

  app_control_create(&app_control);
  app_control_import_from_bundle(app_control, b);

  ret = app_control_send_launch_request(app_control, NULL, NULL);

  if (ret != APP_CONTROL_ERROR_NONE) {
    DDDLOG("app_control_send_launch_request failed");
    switch (ret) {
      case APP_CONTROL_ERROR_INVALID_PARAMETER:
        iotjs_bridge_set_err(hbridge, "APP_CONTROL_ERROR_INVALID_PARAMETER");
        break;
      case APP_CONTROL_ERROR_OUT_OF_MEMORY:
        iotjs_bridge_set_err(hbridge, "APP_CONTROL_ERROR_OUT_OF_MEMORY");
        break;
      case APP_CONTROL_ERROR_APP_NOT_FOUND:
        iotjs_bridge_set_err(hbridge, "APP_CONTROL_ERROR_APP_NOT_FOUND");
        break;
      case APP_CONTROL_ERROR_LAUNCH_REJECTED:
        iotjs_bridge_set_err(hbridge, "APP_CONTROL_ERROR_LAUNCH_REJECTED");
        break;
      case APP_CONTROL_ERROR_LAUNCH_FAILED:
        iotjs_bridge_set_err(hbridge, "APP_CONTROL_ERROR_LAUNCH_FAILED");
        break;
      case APP_CONTROL_ERROR_TIMED_OUT:
        iotjs_bridge_set_err(hbridge, "APP_CONTROL_ERROR_TIMED_OUT");
        break;
      case APP_CONTROL_ERROR_PERMISSION_DENIED:
        iotjs_bridge_set_err(hbridge, "APP_CONTROL_ERROR_PERMISSION_DENIED");
        break;
      default:
        iotjs_bridge_set_err(hbridge, "APP_CONTROL_ERROR_UNKNOWN");
        break;
    }
    return IOTJS_ERROR_RESULT_FAILED;
  }

  bundle_free(b);
  app_control_destroy(app_control);

  return IOTJS_ERROR_NONE;
}


void iotjs_tizen_app_control_cb(app_control_h app_control, void* user_data) {
  DDDLOG("%s", __func__);

  iotjs_environment_t* env = iotjs_environment_get();

  if (env->state != kRunningMain && env->state != kRunningLoop) {
    return;
  }

  const char* event_emitter_name = IOTJS_MAGIC_STRING_TIZEN;
  const char* event_name = IOTJS_MAGIC_STRING_APP_CONTROL;

  jerry_value_t tizen = iotjs_module_get(event_emitter_name);
  jerry_value_t fn = iotjs_jval_get_property(tizen, IOTJS_MAGIC_STRING_EMIT);

  if (jerry_value_is_function(fn) == false) {
    DDDLOG("tizen module is not loaded");
    goto exit;
  }

  // parse app control
  char* json = NULL;
  bundle* b = NULL;

  app_control_export_as_bundle(app_control, &b);

  if (BUNDLE_ERROR_NONE != bundle_to_json(b, &json)) {
    DDLOG("bundle_to_json failed");
    bundle_free(b);
    return;
  }
  DDDLOG("JSON: %s", json);

  // call emit
  jerry_value_t jargv[2] = { jerry_create_string(
                                 (const jerry_char_t*)event_name),
                             jerry_create_string((const jerry_char_t*)json) };

  iotjs_invoke_callback(fn, tizen, jargv, 2);
  jerry_release_value(jargv[0]);
  jerry_release_value(jargv[1]);

  free(json);
  bundle_free(b);

exit:
  jerry_release_value(fn);
}


// # tizen bridge
void iotjs_tizen_func(const char* command, const char* message, void* handle) {
  DDDLOG("%s, cmd: %s, msg: %s", __func__, command, message);

  if (strncmp(command, "getResPath", strlen("getResPath")) == 0) {
    char* app_res_path = app_get_resource_path();
    iotjs_bridge_set_msg(handle, app_res_path);
    if (app_res_path != NULL) {
      free(app_res_path);
    }
  } else if (strncmp(command, "getDataPath", strlen("getDataPath")) == 0) {
    char* app_data_path = app_get_data_path();
    iotjs_bridge_set_msg(handle, app_data_path);
    if (app_data_path != NULL) {
      free(app_data_path);
    }
  } else if (strncmp(command, "launchAppControl", strlen("launchAppControl")) ==
             0) {
    iotjs_error_t err = tizen_send_launch_request(message, handle);
    if (err == IOTJS_ERROR_NONE) {
      iotjs_bridge_set_msg(handle, "OK");
    }

  } else {
    iotjs_bridge_set_err(handle, "Can't find command");
  }
}


// # tizen bridge-native
typedef void (*user_callback_t)(int error, const char* data);

typedef struct {
  uv_async_t async;
  char* module;
  char* fn_name;
  char* message;
  user_callback_t cb;
} iotjs_call_jfunc_t;


static char* create_string_buffer(const char* src, size_t size) {
  char* dest = IOTJS_CALLOC(size + 1, char);
  strncpy(dest, src, size);
  dest[size] = '\0'; // just for being sure
  return dest;
}


static bool bridge_native_call(const char* module_name, const char* func_name,
                               const char* message,
                               iotjs_string_t* output_str) {
  bool result = false;

  jerry_value_t jmodule = iotjs_module_get(module_name);
  jerry_value_t jfunc = iotjs_jval_get_property(jmodule, func_name);

  if (jerry_value_is_function(jfunc) == false) {
    return result;
  }

  jerry_value_t jval = jerry_create_string((const jerry_char_t*)message);
  jerry_value_t jres =
      iotjs_invoke_callback_with_result(jfunc, jmodule, &jval, 1);

  if (jerry_value_is_string(jres)) {
    IOTJS_ASSERT(output_str != NULL);
    *output_str = iotjs_jval_as_string(jres);
    result = true;
  }

  jerry_release_value(jfunc);
  jerry_release_value(jres);
  jerry_release_value(jval);
  return result;
}


static void bridge_native_async_handler(uv_async_t* handle) {
  DDDLOG("%s\n", __func__);
  iotjs_call_jfunc_t* data = (iotjs_call_jfunc_t*)handle->data;

  bool result;
  iotjs_string_t output;

  result = bridge_native_call(IOTJS_MAGIC_STRING_TIZEN, data->fn_name,
                              data->message, &output);

  if (data->cb) {
    data->cb((int)!result, iotjs_string_data(&output));
  }

  iotjs_string_destroy(&output);

  // release
  uv_close((uv_handle_t*)&data->async, NULL);
  IOTJS_RELEASE(data->module);
  IOTJS_RELEASE(data->fn_name);
  IOTJS_RELEASE(data->message);
  IOTJS_RELEASE(data);
}


int iotjs_tizen_bridge_native(const char* fn_name, unsigned fn_name_size,
                              const char* message, unsigned message_size,
                              user_callback_t cb) {
  iotjs_environment_t* env = iotjs_environment_get();

  if (env->state != kRunningMain && env->state != kRunningLoop) {
    return IOTJS_ERROR_RESULT_FAILED;
  }

  iotjs_call_jfunc_t* handle = IOTJS_ALLOC(iotjs_call_jfunc_t);

  if (handle == NULL) {
    return IOTJS_ERROR_OUT_OF_MEMORY;
  }

  handle->async.data = (void*)handle;
  handle->fn_name = create_string_buffer(fn_name, fn_name_size);
  handle->message = create_string_buffer(message, message_size);
  handle->module = create_string_buffer(IOTJS_MAGIC_STRING_TIZEN,
                                        sizeof(IOTJS_MAGIC_STRING_TIZEN));
  handle->cb = cb;

  uv_loop_t* loop = iotjs_environment_loop(env);
  uv_async_init(loop, &handle->async, bridge_native_async_handler);
  uv_async_send(&handle->async);

  return IOTJS_ERROR_NONE;
}
