/* Copyright 2017-present Samsung Electronics Co., Ltd. and other contributors
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
#include "iotjs_objectwrap.h"
#include "iotjs_reqwrap.h"
#include "../common.h"
#include "../structures.h"

#define BOILER_PLATE_SET_HANDLE(name)                                       \
  OCResourceHandle member_##name;                                           \
  const iotjs_jval_t* js_##name = JHANDLER_GET_ARG(0, object);              \
  uintptr_t handle_##name = iotjs_jval_get_object_native_handle(js_##name); \
  member_##name = (OCResourceHandle)handle_##name;

#define BOILER_PLATE_BIND(name1, name2)                                       \
  OCResourceHandle member_##name1;                                            \
  const iotjs_jval_t* js_##name1 = JHANDLER_GET_ARG(0, object);               \
  uintptr_t handle_##name1 = iotjs_jval_get_object_native_handle(js_##name1); \
  member_##name1 = (OCResourceHandle)handle_##name1;                          \
  OCResourceHandle member_##name2;                                            \
  const iotjs_jval_t* js_##name2 = JHANDLER_GET_ARG(1, object);               \
  uintptr_t handle_##name2 = iotjs_jval_get_object_native_handle(js_##name2); \
  member_##name2 = (OCResourceHandle)handle_##name2;
