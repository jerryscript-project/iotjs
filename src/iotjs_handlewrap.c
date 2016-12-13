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
#include "iotjs_handlewrap.h"


void iotjs_handlewrap_initialize(iotjs_handlewrap_t* handlewrap,
                                 const iotjs_jval_t* jobject,
                                 uv_handle_t* handle,
                                 JFreeHandlerType jfreehandler) {
  IOTJS_VALIDATED_STRUCT_CONSTRUCTOR(iotjs_handlewrap_t, handlewrap);

  // Increase ref count of Javascript object to guarantee it is alive until the
  // handle has closed.
  iotjs_jval_t jobjectref = iotjs_jval_create_copied(jobject);
  iotjs_jobjectwrap_initialize(&_this->jobjectwrap, &jobjectref, jfreehandler);

  _this->handle = handle;
  _this->on_close_cb = NULL;

  handle->data = handlewrap;

  iotjs_handlewrap_validate(handlewrap);
}


void iotjs_handlewrap_destroy(iotjs_handlewrap_t* handlewrap) {
  IOTJS_VALIDATED_STRUCT_DESTRUCTOR(iotjs_handlewrap_t, handlewrap);

  // Handle should have been release before this.
  IOTJS_ASSERT(_this->handle == NULL);

  iotjs_jobjectwrap_destroy(&_this->jobjectwrap);
}


iotjs_handlewrap_t* iotjs_handlewrap_from_handle(uv_handle_t* handle) {
  iotjs_handlewrap_t* handlewrap = (iotjs_handlewrap_t*)(handle->data);
  iotjs_handlewrap_validate(handlewrap);
  return handlewrap;
}


iotjs_handlewrap_t* iotjs_handlewrap_from_jobject(const iotjs_jval_t* jobject) {
  iotjs_handlewrap_t* handlewrap =
      (iotjs_handlewrap_t*)(iotjs_jval_get_object_native_handle(jobject));
  iotjs_handlewrap_validate(handlewrap);
  return handlewrap;
}


uv_handle_t* iotjs_handlewrap_get_uv_handle(iotjs_handlewrap_t* handlewrap) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_handlewrap_t, handlewrap);
  iotjs_handlewrap_validate(handlewrap);
  return _this->handle;
}


iotjs_jval_t* iotjs_handlewrap_jobject(iotjs_handlewrap_t* handlewrap) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_handlewrap_t, handlewrap);
  iotjs_handlewrap_validate(handlewrap);
  return iotjs_jobjectwrap_jobject(&_this->jobjectwrap);
}


static void iotjs_handlewrap_on_close(iotjs_handlewrap_t* handlewrap) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_handlewrap_t, handlewrap);

  // The handle closed.
  // Calls registered close handler function.
  if (_this->on_close_cb) {
    _this->on_close_cb(_this->handle);
  }

  // Set handle null.
  _this->handle = NULL;

  // Decrease ref count of Javascript object. From now the object can be
  // reclaimed.
  iotjs_jval_destroy(iotjs_jobjectwrap_jobject(&_this->jobjectwrap));
}


static void iotjs_on_handle_closed(uv_handle_t* handle) {
  iotjs_handlewrap_t* handlewrap = iotjs_handlewrap_from_handle(handle);
  iotjs_handlewrap_on_close(handlewrap);
}


void iotjs_handlewrap_close(iotjs_handlewrap_t* handlewrap,
                            OnCloseHandler on_close_cb) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_handlewrap_t, handlewrap);

  if (_this->handle != NULL && !uv_is_closing(_this->handle)) {
    _this->on_close_cb = on_close_cb;
    uv_close(_this->handle, iotjs_on_handle_closed);
  } else {
    DDLOG("Attempt to close uninitialized or already closed handle");
  }
}


void iotjs_handlewrap_validate(iotjs_handlewrap_t* handlewrap) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_handlewrap_t, handlewrap);

  IOTJS_ASSERT((iotjs_handlewrap_t*)_this == handlewrap);
  IOTJS_ASSERT((iotjs_jobjectwrap_t*)_this == &_this->jobjectwrap);
  IOTJS_ASSERT((void*)_this == _this->handle->data);
  IOTJS_ASSERT((uintptr_t)_this ==
               iotjs_jval_get_object_native_handle(
                   iotjs_jobjectwrap_jobject(&_this->jobjectwrap)));
}
