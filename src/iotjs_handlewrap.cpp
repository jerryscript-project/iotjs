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

#include "iotjs_def.h"
#include "iotjs_handlewrap.h"


namespace iotjs {


HandleWrap::HandleWrap(JObject& jobject, uv_handle_t* handle)
    : JObjectWrap(jobject)
    , __handle(handle)
    , _on_close_cb(NULL) {
  __handle->data = this;

  // Increase ref count of Javascirpt object to gurantee it is alive until the
  // handle has closed.
  _jobject->Ref();
}


HandleWrap::~HandleWrap() {
  // Handle should have been release before this.
  IOTJS_ASSERT(__handle == NULL);
}


HandleWrap* HandleWrap::FromHandle(uv_handle_t* handle) {
  HandleWrap* wrap = reinterpret_cast<HandleWrap*>(handle->data);
  IOTJS_ASSERT(wrap != NULL);
  IOTJS_ASSERT(wrap->__handle == handle);
  return wrap;
}


void HandleWrap::OnClose() {
  // The handle closed.
  // Calls registered close handler function.
  if (_on_close_cb) {
    _on_close_cb(__handle);
  }

  // Set handle null.
  __handle = NULL;

  // Decrease ref count of Javascript object. From now the object can be
  // recliamed.
  _jobject->Unref();
}


static void OnHandleClosed(uv_handle_t* handle) {
  HandleWrap* wrap = HandleWrap::FromHandle(handle);
  wrap->OnClose();
}


// Close handle.
void HandleWrap::Close(OnCloseHandler on_close_cb) {
  if (__handle != NULL && !uv_is_closing(__handle)) {
    _on_close_cb = on_close_cb;
    uv_close(__handle, OnHandleClosed);
  } else {
    DDLOG("Attempt to close uninitialized or already closed handle");
  }
}


void HandleWrap::Destroy(void) {
  // Handle should have been release before this.
  IOTJS_ASSERT(__handle == NULL);

  JObjectWrap::Destroy();
}


} // namespace iotjs
