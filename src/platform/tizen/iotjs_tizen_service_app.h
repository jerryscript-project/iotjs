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

#ifndef IOTJS_TIZEN_SERVICE_APP_H
#define IOTJS_TIZEN_SERVICE_APP_H

#include <app_control.h>
#include <string.h>

#ifdef __cplusplus
#define IOTJS_EXTERN_C extern "C"
#else /* !__cplusplus */
#define IOTJS_EXTERN_C extern
#endif /* !__cplusplus */

typedef void (*user_callback_t)(int error, const char* data);


IOTJS_EXTERN_C int iotjs_service_app_start(int argc, char** argv, char* js_path,
                                           void* event_callbacks,
                                           void* user_data);

IOTJS_EXTERN_C
void iotjs_tizen_app_control_cb(app_control_h app_control, void* user_data);

IOTJS_EXTERN_C
int iotjs_tizen_bridge_native(const char* fn_name, unsigned fn_name_size,
                              const char* message, unsigned message_size,
                              user_callback_t cb);

#define IOTJS_TIZEN_CALL_JFUNC(name, msg, cb)                              \
  ({                                                                       \
    if (name != NULL && (msg) != NULL)                                     \
      iotjs_tizen_bridge_native(name, strlen(name), msg, strlen(msg), cb); \
  })


#endif /* IOTJS_TIZEN_SERVICE_APP_H */
