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

#ifndef IOTJS_HANDLEWRAP_H
#define IOTJS_HANDLEWRAP_H


#include <uv.h>

#include "iotjs_binding.h"
#include "iotjs_objectwrap.h"


typedef void (*OnCloseHandler)(uv_handle_t*);


// UV handle wrapper.
// This wrapper connects a Javascript object and a libuv handler.
// This wrapper will increase ref count for the Javascript object and decrease
//  it after corresponding handle has closed. Hence the Javascript object will
//  not turn into garbage until the handle is open.

// Javascript object
//   ->
// Create a handle wrap, initializing uv handle, increase ref count.
//   ->
// The javascript object will be alive until handle has closed.
//   ->
// Handle closed, release handle, decrease ref count.
//   ->
// The javascript object now can be reclaimed by GC.

typedef struct {
  iotjs_jobjectwrap_t jobjectwrap;
  uv_handle_t* handle;
  OnCloseHandler on_close_cb;
} IOTJS_VALIDATED_STRUCT(iotjs_handlewrap_t);


// jobject: Object that connect with the uv handle
void iotjs_handlewrap_initialize(iotjs_handlewrap_t* handlewrap,
                                 const iotjs_jval_t* jobject,
                                 uv_handle_t* handle,
                                 JFreeHandlerType jfreehandler);

void iotjs_handlewrap_destroy(iotjs_handlewrap_t* handlewrap);

void iotjs_handlewrap_close(iotjs_handlewrap_t* handlewrap,
                            OnCloseHandler on_close_cb);

iotjs_handlewrap_t* iotjs_handlewrap_from_handle(uv_handle_t* handle);
iotjs_handlewrap_t* iotjs_handlewrap_from_jobject(const iotjs_jval_t* jobject);

uv_handle_t* iotjs_handlewrap_get_uv_handle(iotjs_handlewrap_t* handlewrap);
iotjs_jval_t* iotjs_handlewrap_jobject(iotjs_handlewrap_t* handlewrap);

void iotjs_handlewrap_validate(iotjs_handlewrap_t* handlewrap);


#endif /* IOTJS_HANDLEWRAP_H */
