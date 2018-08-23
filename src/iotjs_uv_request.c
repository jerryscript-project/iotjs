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

#include "iotjs_uv_request.h"

#include "iotjs_def.h"

/**
 * Aligns @a value to @a alignment. @a must be the power of 2.
 *
 * Returns minimum positive value, that divides @a alignment and is more than or
 * equal to @a value
 */
#define IOTJS_ALIGNUP(value, alignment) \
  (((value) + ((alignment)-1)) & ~((alignment)-1))


uv_req_t* iotjs_uv_request_create(size_t request_size,
                                  const jerry_value_t jcallback,
                                  size_t extra_data_size) {
  IOTJS_ASSERT(jerry_value_is_function(jcallback));

  /* Make sure that the jerry_value_t is aligned */
  size_t aligned_request_size = IOTJS_ALIGNUP(request_size, 8u);

  char* request_memory = iotjs_buffer_allocate(
      aligned_request_size + sizeof(jerry_value_t) + extra_data_size);
  uv_req_t* uv_request = (uv_req_t*)request_memory;
  uv_request->data = request_memory + aligned_request_size;

  *IOTJS_UV_REQUEST_JSCALLBACK(uv_request) = jcallback;
  jerry_acquire_value(jcallback);

  return uv_request;
}


void iotjs_uv_request_destroy(uv_req_t* request) {
  jerry_release_value(*IOTJS_UV_REQUEST_JSCALLBACK(request));
  IOTJS_RELEASE(request);
}
