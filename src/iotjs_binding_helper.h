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

#ifndef IOTJS_BINDING_HELPER_H
#define IOTJS_BINDING_HELPER_H


#include "iotjs_binding.h"


void iotjs_uncaught_exception(jerry_value_t jexception);

void iotjs_process_emit_exit(int code);

bool iotjs_process_next_tick(void);

void iotjs_invoke_callback(jerry_value_t jfunc, jerry_value_t jthis,
                           const jerry_value_t* jargv, size_t jargc);
jerry_value_t iotjs_invoke_callback_with_result(jerry_value_t jfunc,
                                                jerry_value_t jthis,
                                                const jerry_value_t* jargv,
                                                size_t jargc);

int iotjs_process_exitcode(void);
void iotjs_set_process_exitcode(int code);

#endif /* IOTJS_BINDING_HELPER_H */
