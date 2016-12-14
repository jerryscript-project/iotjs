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
#include "iotjs_objectwrap.h"


void iotjs_jobjectwrap_initialize(iotjs_jobjectwrap_t* jobjectwrap,
                                  const iotjs_jval_t* jobject,
                                  JFreeHandlerType jfreehandler) {
  IOTJS_VALIDATED_STRUCT_CONSTRUCTOR(iotjs_jobjectwrap_t, jobjectwrap);

  IOTJS_ASSERT(iotjs_jval_is_object(jobject));

  // This wrapper holds pointer to the javascript object but never increases
  // reference count.
  _this->jobject = *((iotjs_jval_t*)jobject);

  // Set native pointer of the object to be this wrapper.
  // If the object is freed by GC, the wrapper instance should also be freed.
  iotjs_jval_set_object_native_handle(&_this->jobject, (uintptr_t)jobjectwrap,
                                      jfreehandler);
}


void iotjs_jobjectwrap_destroy(iotjs_jobjectwrap_t* jobjectwrap) {
  IOTJS_VALIDATED_STRUCT_DESTRUCTOR(iotjs_jobjectwrap_t, jobjectwrap);
  /* Do nothing on _this->jobject */
}


iotjs_jval_t* iotjs_jobjectwrap_jobject(iotjs_jobjectwrap_t* jobjectwrap) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_jobjectwrap_t, jobjectwrap);
  iotjs_jval_t* jobject = &_this->jobject;
  IOTJS_ASSERT((uintptr_t)jobjectwrap ==
               iotjs_jval_get_object_native_handle(jobject));
  IOTJS_ASSERT(iotjs_jval_is_object(jobject));
  return jobject;
}


iotjs_jobjectwrap_t* iotjs_jobjectwrap_from_jobject(
    const iotjs_jval_t* jobject) {
  iotjs_jobjectwrap_t* wrap =
      (iotjs_jobjectwrap_t*)(iotjs_jval_get_object_native_handle(jobject));
  IOTJS_ASSERT(iotjs_jval_is_object(iotjs_jobjectwrap_jobject(wrap)));
  return wrap;
}
