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
#include <app_control.h>
#include <bundle.h>

typedef enum {
  IOTJS_ERROR_NONE = 0,
  IOTJS_ERROR_RESULT_FAILED,
  IOTJS_ERROR_INVALID_PARAMETER,
} iotjs_error_t;

// application control
#include <app_control_internal.h>
#include <bundle_internal.h>

iotjs_error_t send_launch_request(const char* json, void* hbridge) {
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


void iotjs_service_app_control_cb(app_control_h app_control, void* user_data) {
  DDDLOG("%s", __func__);

  iotjs_environment_t* env = iotjs_environment_get();

  if (env->state != kRunningMain && env->state != kRunningLoop) {
    return;
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

  // prepare emit
  const char* event_emitter_name = IOTJS_MAGIC_STRING_TIZEN;
  const char* event_name = IOTJS_MAGIC_STRING_APP_CONTROL;

  jerry_value_t tizen = iotjs_module_get(event_emitter_name);
  jerry_value_t fn = iotjs_jval_get_property(tizen, IOTJS_MAGIC_STRING_EMIT);

  // call emit
  iotjs_jargs_t jargv = iotjs_jargs_create(2);
  iotjs_jargs_append_string_raw(&jargv, event_name);
  iotjs_jargs_append_string_raw(&jargv, json);

  iotjs_make_callback(fn, tizen, &jargv);

  IOTJS_RELEASE(json);
  bundle_free(b);

  jerry_release_value(fn);
  iotjs_jargs_destroy(&jargv);
}


void iotjs_tizen_func(const char* command, const char* message, void* handle) {
  DDDLOG("%s, cmd: %s, msg: %s", __func__, command, message);

  if (strncmp(command, "getResPath", strlen("getResPath")) == 0) {
    char* app_res_path = app_get_resource_path();
    iotjs_bridge_set_msg(handle, app_res_path);

  } else if (strncmp(command, "launchAppControl", strlen("launchAppControl")) ==
             0) {
    iotjs_error_t err = send_launch_request(message, handle);
    if (err == IOTJS_ERROR_NONE) {
      iotjs_bridge_set_msg(handle, "OK");
    }

  } else {
    iotjs_bridge_set_err(handle, "Can't find command");
  }
}
