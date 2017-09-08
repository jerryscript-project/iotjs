/* Copyright 2015-present Samsung Electronics Co., Ltd. and other contributors
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

#ifndef IOTJS_OBJECTWRAP_H
#define IOTJS_OBJECTWRAP_H


#include "iotjs_binding.h"


// This wrapper refer javascript object but never increase reference count
// If the object is freed by GC, then this wrapper instance will be also freed.
typedef struct {
  iotjs_jval_t jobject;
} IOTJS_VALIDATED_STRUCT(iotjs_jobjectwrap_t);

void iotjs_jobjectwrap_initialize(iotjs_jobjectwrap_t* jobjectwrap,
                                  const iotjs_jval_t* jobject,
                                  JNativeInfoType* native_info);

void iotjs_jobjectwrap_destroy(iotjs_jobjectwrap_t* jobjectwrap);

iotjs_jval_t* iotjs_jobjectwrap_jobject(iotjs_jobjectwrap_t* jobjectwrap);
iotjs_jobjectwrap_t* iotjs_jobjectwrap_from_jobject(
    const iotjs_jval_t* jobject);

#define IOTJS_DEFINE_NATIVE_HANDLE_INFO(module)                              \
  static const jerry_object_native_info_t module##_native_info = {           \
    .free_cb = (jerry_object_native_free_callback_t)iotjs_##module##_destroy \
  }

#define IOTJS_DEFINE_NATIVE_HANDLE_INFO_THIS_MODULE(name)                  \
  static void iotjs_##name##_destroy(iotjs_##name##_t* wrap);              \
  static const jerry_object_native_info_t this_module_native_info = {      \
    .free_cb = (jerry_object_native_free_callback_t)iotjs_##name##_destroy \
  }

#endif /* IOTJS_OBJECTWRAP_H */
