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


void iotjs_reqwrap_initialize(iotjs_reqwrap_t* reqwrap,
                              const iotjs_jval_t* jcallback,
                              uv_req_t* request) {
  IOTJS_VALIDATED_STRUCT_CONSTRUCTOR(iotjs_reqwrap_t, reqwrap);
  IOTJS_ASSERT(iotjs_jval_is_function(jcallback));
  _this->jcallback = iotjs_jval_create_copied(jcallback);
  _this->request = request;
  _this->request->data = reqwrap;
}


void iotjs_reqwrap_destroy(iotjs_reqwrap_t* reqwrap) {
  IOTJS_VALIDATED_STRUCT_DESTRUCTOR(iotjs_reqwrap_t, reqwrap);
  iotjs_jval_destroy(&_this->jcallback);
}


static void iotjs_reqwrap_validate(iotjs_reqwrap_t* reqwrap) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_reqwrap_t, reqwrap);
  IOTJS_ASSERT(_this->request->data == reqwrap);
}


const iotjs_jval_t* iotjs_reqwrap_jcallback(iotjs_reqwrap_t* reqwrap) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_reqwrap_t, reqwrap);
  iotjs_reqwrap_validate(reqwrap);
  return &_this->jcallback;
}


uv_req_t* iotjs_reqwrap_req(iotjs_reqwrap_t* reqwrap) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_reqwrap_t, reqwrap);
  iotjs_reqwrap_validate(reqwrap);
  return _this->request;
}


iotjs_reqwrap_t* iotjs_reqwrap_from_request(uv_req_t* req) {
  iotjs_reqwrap_t* reqwrap = req->data;
  iotjs_reqwrap_validate(reqwrap);
  return reqwrap;
}
