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

#ifndef IOTJS_MODULE_TIMER_H
#define IOTJS_MODULE_TIMER_H


#include "iotjs_binding.h"
#include "iotjs_handlewrap.h"


typedef struct {
  iotjs_handlewrap_t handlewrap;
  uv_timer_t handle;
} IOTJS_VALIDATED_STRUCT(iotjs_timerwrap_t);


iotjs_timerwrap_t* iotjs_timerwrap_create(const iotjs_jval_t* jtimer);

iotjs_timerwrap_t* iotjs_timerwrap_from_jobject(const iotjs_jval_t* jtimer);
iotjs_timerwrap_t* iotjs_timerwrap_from_handle(uv_timer_t* timer_handle);

uv_timer_t* iotjs_timerwrap_handle(iotjs_timerwrap_t* timerwrap);
iotjs_jval_t* iotjs_timerwrap_jobject(iotjs_timerwrap_t* timerwrap);

// Start timer.
int iotjs_timerwrap_start(iotjs_timerwrap_t* timerwrap, int64_t timeout,
                          int64_t repeat);
// Stop & close timer.
int iotjs_timerwrap_stop(iotjs_timerwrap_t* timerwrap);


#endif /* IOTJS_MODULE_TIMER_H */
