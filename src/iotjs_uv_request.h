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

#ifndef IOTJS_UV_REQUEST
#define IOTJS_UV_REQUEST

#include <uv.h>

#include "iotjs_binding.h"

/**
 * Allocate and initialize an uv_req_t structure with a jerry callback and extra
 * data.
 *
 * The allocated memory has the following layout:
 *
 *  |----------|  <- start of uv_req_t*
 *  | uv_req_t |
 *  |          |
 *  |----------|
 *  | PADDING  |  <- alignment padding
 *  |----------|  <- start of jerry_value_t* which is the callback
 *  | callback |
 *  |----------|  <- start of the extra data if required
 *  |  extra   |
 *  |----------|
 *
 */
uv_req_t* iotjs_uv_request_create(size_t request_size,
                                  const jerry_value_t jcallback,
                                  size_t extra_data_size);
void iotjs_uv_request_destroy(uv_req_t* request);

/**
 * Returns a pointer to the js callback referenced by the uv_req_t->data member.
 */
#define IOTJS_UV_REQUEST_JSCALLBACK(UV_REQ) ((jerry_value_t*)(UV_REQ->data))

/**
 * Returns a char* pointer for any extra data.
 *
 * IMPORTANT!
 * Make sure that the extra data is correctly allocated via the
 * iotjs_uv_request_create method call.
 */
#define IOTJS_UV_REQUEST_EXTRA_DATA(UV_REQ) \
  ((char*)((char*)(UV_REQ->data) + sizeof(jerry_value_t)))

#endif /* IOTJS_UV_REQUEST */
