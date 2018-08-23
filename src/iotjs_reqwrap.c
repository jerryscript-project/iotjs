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

#include "iotjs_def.h"
#include "iotjs_reqwrap.h"


void iotjs_reqwrap_initialize(iotjs_reqwrap_t* reqwrap, jerry_value_t jcallback,
                              uv_req_t* request) {
  reqwrap->jcallback = jerry_acquire_value(jcallback);
  reqwrap->request = request;
  reqwrap->request->data = reqwrap;
}


void iotjs_reqwrap_create_for_uv_data(uv_req_t* request,
                                      jerry_value_t jcallback) {
  IOTJS_ASSERT(request != NULL);
  iotjs_reqwrap_t* reqwrap = IOTJS_ALLOC(iotjs_reqwrap_t);
  iotjs_reqwrap_initialize(reqwrap, jcallback, request);
}


void iotjs_reqwrap_destroy(iotjs_reqwrap_t* reqwrap) {
  jerry_release_value(reqwrap->jcallback);
}


static void iotjs_reqwrap_validate(iotjs_reqwrap_t* reqwrap) {
  IOTJS_ASSERT(reqwrap->request->data == reqwrap);
}


jerry_value_t iotjs_reqwrap_jcallback(iotjs_reqwrap_t* reqwrap) {
  iotjs_reqwrap_validate(reqwrap);
  return reqwrap->jcallback;
}


uv_req_t* iotjs_reqwrap_req(iotjs_reqwrap_t* reqwrap) {
  iotjs_reqwrap_validate(reqwrap);
  return reqwrap->request;
}


iotjs_reqwrap_t* iotjs_reqwrap_from_request(uv_req_t* req) {
  iotjs_reqwrap_t* reqwrap = req->data;
  iotjs_reqwrap_validate(reqwrap);
  return reqwrap;
}
