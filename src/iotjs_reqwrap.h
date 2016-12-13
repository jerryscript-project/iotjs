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

#ifndef IOTJS_REQWRAP_H
#define IOTJS_REQWRAP_H


#include <uv.h>

#include "iotjs_binding.h"


// UV request wrapper.
// Wrapping UV request and JavaScript callback.
// When an instance of request wrapper is created. it will increase ref count
// for JavaScript callback function to prevent it from reclaimed by GC. The
// reference count will decrease back when wrapper is being freed.
typedef struct {
  iotjs_jval_t jcallback;
  uv_req_t* request;
} IOTJS_VALIDATED_STRUCT(iotjs_reqwrap_t);


void iotjs_reqwrap_initialize(iotjs_reqwrap_t* reqwrap,
                              const iotjs_jval_t* jcallback, uv_req_t* request);
void iotjs_reqwrap_destroy(iotjs_reqwrap_t* reqwrap);

// To retrieve javascript callback function object.
const iotjs_jval_t* iotjs_reqwrap_jcallback(iotjs_reqwrap_t* reqwrap);

// To retrieve pointer to uv request.
uv_req_t* iotjs_reqwrap_req(iotjs_reqwrap_t* reqwrap);


iotjs_reqwrap_t* iotjs_reqwrap_from_request(uv_req_t* req);


#endif /* IOTJS_REQWRAP_H */
