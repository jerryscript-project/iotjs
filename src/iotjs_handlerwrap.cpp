/* Copyright 2015 Samsung Electronics Co., Ltd.
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

#include "iotjs_binding.h"
#include "iotjs_handlewrap.h"


namespace iotjs {


JFREE_HANDLER_FUNCTION(FreeHandleWrap, handle_wrap_p) {
  // native_p is always non-null value
  assert(handle_wrap_p != 0);
  delete reinterpret_cast<HandleWrap*>(handle_wrap_p);
}


HandleWrap::HandleWrap(JObject* jobj, uv_handle_t* handle)
    : __handle(handle)
    , _jobj(NULL)
    , _jcallback(NULL) {

  if (jobj != NULL) {
    JRawValueType jraw_value = jobj->raw_value();
    _jobj = new JObject(&jraw_value, false);

    _jobj->SetNative((uintptr_t)this, FreeHandleWrap);
  }

  __handle->data = this;
}


HandleWrap::~HandleWrap() {
  if (_jobj != NULL) {
    delete _jobj;
  }

  if (_jcallback != NULL) {
    delete _jcallback;
  }
}


JObject* HandleWrap::object() {
    return _jobj;
}


JObject* HandleWrap::callback() {
  return _jcallback;
}


void HandleWrap::set_callback(JObject& jcallback) {
  if (_jcallback != NULL) {
    delete _jcallback;
  }

  // FIXME: Is there any risk that the `_jcallback` turns into dangling?
  JRawValueType jraw_value = jcallback.raw_value();
  _jcallback = new JObject(&jraw_value, false);
}


} // namespace iotjs
