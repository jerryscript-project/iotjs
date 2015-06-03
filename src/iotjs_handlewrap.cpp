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

#include "iotjs_def.h"


namespace iotjs {


HandleWrap::HandleWrap(JObject& jnative, JObject& jholder, uv_handle_t* handle)
    : JObjectWrap(jnative)
    , __handle(handle)
    , _jholder(NULL) {
  if (!jholder.IsNull()) {
    JRawValueType raw_value = jholder.raw_value();
    _jholder = new JObject(&raw_value, false);
  }
  __handle->data = this;
}


HandleWrap::~HandleWrap() {
  if (_jholder != NULL) {
    delete _jholder;
  }
}


HandleWrap* HandleWrap::FromHandle(uv_handle_t* handle) {
  HandleWrap* wrap = reinterpret_cast<HandleWrap*>(handle->data);
  assert(wrap != NULL);
  return wrap;
}


JObject& HandleWrap::jnative() {
  return jobject();
}


JObject& HandleWrap::jholder() {
  assert(_jholder != NULL);
  return *_jholder;
}


void HandleWrap::set_jholder(JObject& jholder) {
  assert(_jholder == NULL);
  assert(jholder.IsObject());

  JRawValueType raw_value = jholder.raw_value();
  _jholder = new JObject(&raw_value, false);
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
