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


void iotjs_uncaught_exception(const iotjs_jval_t* jexception);

void iotjs_process_emit_exit(int code);

bool iotjs_process_next_tick();

void iotjs_make_callback(const iotjs_jval_t* jfunction,
                         const iotjs_jval_t* jthis, const iotjs_jargs_t* jargs);

iotjs_jval_t iotjs_make_callback_with_result(const iotjs_jval_t* jfunction,
                                             const iotjs_jval_t* jthis,
                                             const iotjs_jargs_t* jargs);


const iotjs_jval_t* iotjs_init_process_module();


#endif /* IOTJS_BINDING_HELPER_H */
