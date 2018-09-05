/* Copyright 2018-present Samsung Electronics Co., Ltd. and other contributors
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

#include "iotjs_uv_handle.h"


uv_handle_t* iotjs_uv_handle_create(size_t handle_size,
                                    const jerry_value_t jobject,
                                    JNativeInfoType* native_info,
                                    size_t extra_data_size) {
  IOTJS_ASSERT(jerry_value_is_object(jobject));

  /* Make sure that the jerry_value_t is aligned */
  size_t aligned_request_size = IOTJS_ALIGNUP(handle_size, 8u);

  char* request_memory = iotjs_buffer_allocate(
      aligned_request_size + sizeof(iotjs_uv_handle_data) + extra_data_size);
  uv_handle_t* uv_handle = (uv_handle_t*)request_memory;
  uv_handle->data = request_memory + aligned_request_size;

  IOTJS_UV_HANDLE_DATA(uv_handle)->jobject = jobject;
  IOTJS_UV_HANDLE_DATA(uv_handle)->on_close_cb = NULL;
  jerry_acquire_value(jobject);

  jerry_set_object_native_pointer(jobject, uv_handle, native_info);

  return uv_handle;
}


static void iotjs_uv_handle_close_processor(uv_handle_t* handle) {
  iotjs_uv_handle_data* handle_data = IOTJS_UV_HANDLE_DATA(handle);

  if (handle_data->on_close_cb != NULL) {
    handle_data->on_close_cb(handle);
  }

  // Decrease ref count of Javascript object. From now the object can be
  // reclaimed.
  jerry_release_value(handle_data->jobject);
  IOTJS_RELEASE(handle);
}


void iotjs_uv_handle_close(uv_handle_t* handle, OnCloseHandler close_handler) {
  if (handle == NULL || uv_is_closing(handle)) {
    DDLOG("Attempt to close uninitialized or already closed handle");
    return;
  }

  iotjs_uv_handle_data* handle_data = IOTJS_UV_HANDLE_DATA(handle);
  handle_data->on_close_cb = close_handler;
  uv_close(handle, iotjs_uv_handle_close_processor);
}
