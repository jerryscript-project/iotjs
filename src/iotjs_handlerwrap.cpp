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


namespace iotjs {

static void ObjectFreeCallback(const uintptr_t native_p) {
  // native_p is always non-null value
  HandleWrap* wrap = reinterpret_cast<HandleWrap*>(native_p);
  delete wrap;
}

HandleWrap::HandleWrap(JObject* othis, uv_handle_t* handle)
    : __handle(handle) {
  __handle->data = this;
  othis->SetNative((uintptr_t)this);
  othis->SetFreeCallback(ObjectFreeCallback);
}

HandleWrap::~HandleWrap() {
  __handle->data = NULL;
}

} // namespace iotjs
