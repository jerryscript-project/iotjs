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
#include "iotjs_objectwrap.h"


namespace iotjs {


static void FreeObjectWrap(const uintptr_t wrapper) {
  // native pointer must not be NULL.
  IOTJS_ASSERT(wrapper != 0);

  JObjectWrap* object_wrap = reinterpret_cast<JObjectWrap*>(wrapper);
  object_wrap->Destroy();
}


JObjectWrap::JObjectWrap(const iotjs_jval_t* jobject) {
  IOTJS_ASSERT(iotjs_jval_is_object(jobject));

  // This wrapper holds pointer to the javascript object but never increases
  // reference count.
  _jobject = *((iotjs_jval_t*)jobject);

  // Set native pointer of the object to be this wrapper.
  // If the object is freed by GC, the wrapper instance should also be freed.
  uintptr_t handle = (uintptr_t)this;
  iotjs_jval_set_object_native_handle(&_jobject, handle, FreeObjectWrap);
}


JObjectWrap::~JObjectWrap() {
}


iotjs_jval_t* JObjectWrap::jobject() {
  return &_jobject;
}


void JObjectWrap::Destroy(void) {
  delete this;
}


} // namespace itojs
