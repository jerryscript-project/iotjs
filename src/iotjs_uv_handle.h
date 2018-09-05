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

#ifndef IOTJS_UV_HANDLE
#define IOTJS_UV_HANDLE

#include <uv.h>

#include "iotjs_binding.h"

typedef void (*OnCloseHandler)(uv_handle_t*);

typedef struct {
  jerry_value_t jobject;
  OnCloseHandler on_close_cb;
} iotjs_uv_handle_data;

#define IOTJS_ALIGNUP(value, alignment) \
  (((value) + ((alignment)-1)) & ~((alignment)-1))

/**
 * Allocate and initialize an uv_handle_t structure with a jerry callback and
 * extra data.
 *
 * The allocated memory has the following layout:
 *
 *  |-------------|  <- start of uv_handle_t*
 *  | uv_handle_t |
 *  |             |
 *  |-------------|
 *  | PADDING     |  <- alignment padding
 *  |-------------|  <- start of the iotjs_uv_handle_data struct
 *  | handle_data |
 *  |-------------|  <- start of the extra data if required
 *  |  extra      |
 *  |-------------|
 *
 */
uv_handle_t* iotjs_uv_handle_create(size_t handle_size,
                                    const jerry_value_t jobject,
                                    JNativeInfoType* native_info,
                                    size_t extra_data_size);
void iotjs_uv_handle_close(uv_handle_t* handle, OnCloseHandler close_handler);

/**
 * Returns a pointer to the handle data struct referenced
 * by the uv_handle_t->data member.
 */
#define IOTJS_UV_HANDLE_DATA(UV_HANDLE) \
  ((iotjs_uv_handle_data*)((UV_HANDLE)->data))

/**
 * Returns a char* pointer for any extra data.
 *
 * IMPORTANT!
 * Make sure that the extra data is correctly allocated by using the
 * iotjs_uv_handle_create method call.
 */
#define IOTJS_UV_HANDLE_EXTRA_DATA(UV_HANDLE) \
  ((char*)((char*)((UV_HANDLE)->data) + sizeof(iotjs_uv_handle_data)))


#endif /* IOTJS_UV_HANDLE */
