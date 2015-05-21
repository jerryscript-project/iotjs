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


#include "iotjs_handlewrap.h"

#include "iotjs_debuglog.h"


namespace iotjs {


JFREE_HANDLER_FUNCTION(FreeHandleWrap, wrapper) {
  // native pointer must not be NULL.
  assert(wrapper != 0);
  delete reinterpret_cast<HandleWrap*>(wrapper);
}


HandleWrap::HandleWrap(JObject& jobj, uv_handle_t* handle)
    : __handle(handle)
    , _jobj(NULL) {
  if (!jobj.IsNull()) {
    // This wrapper hold pointer to the javascript object but never increase
    // reference count.
    JRawValueType raw_value = jobj.raw_value();
    _jobj = new JObject(&raw_value, false);
    // Set native pointer of the object to be this wrapper.
    // If the object is freed by GC, the wrapper instance should also be freed.
    _jobj->SetNative((uintptr_t)this, FreeHandleWrap);
  }
  __handle->data = this;
}


HandleWrap::~HandleWrap() {
  if (_jobj != NULL) {
    delete _jobj;
  }
}


HandleWrap* HandleWrap::FromHandle(uv_handle_t* handle) {
  HandleWrap* wrap = reinterpret_cast<HandleWrap*>(handle->data);
  assert(wrap != NULL);
  return wrap;
}


JObject* HandleWrap::jobject() {
  return _jobj;
}


// Close handle.
void HandleWrap::Close(OnCloseHandler on_close_cb) {
  if (__handle != NULL) {
    uv_close(__handle, on_close_cb);
    __handle = NULL;
  } else {
    DDLOG("Attempt to close uninitialized or already closed handle");
  }
}


} // namespace iotjs
