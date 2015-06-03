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

#include "iotjs_objectwrap.h"

#include "iotjs_def.h"


namespace iotjs {


JFREE_HANDLER_FUNCTION(FreeObjectWrap, wrapper) {
  // native pointer must not be NULL.
  assert(wrapper != 0);
  delete reinterpret_cast<JObjectWrap*>(wrapper);
}


JObjectWrap::JObjectWrap(JObject& jobject)
    : _jobject(NULL) {
  assert(jobject.IsObject());

  // This wrapper hold pointer to the javascript object but never increase
  // reference count.
  JRawValueType raw_value = jobject.raw_value();
  _jobject = new JObject(&raw_value, false);
  // Set native pointer of the object to be this wrapper.
  // If the object is freed by GC, the wrapper instance should also be freed.
  _jobject->SetNative((uintptr_t)this, FreeObjectWrap);
}


JObjectWrap::~JObjectWrap() {
  assert(_jobject != NULL);
  assert(_jobject->IsObject());
  delete _jobject;
}


JObject& JObjectWrap::jobject() {
  assert(_jobject);
  return *_jobject;
}


} // namespace itojs
