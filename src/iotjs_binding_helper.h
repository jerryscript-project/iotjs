/* Copyright 2015-2016 Samsung Electronics Co., Ltd.
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


namespace iotjs {


void UncaughtException(const iotjs_jval_t* jexception);

void ProcessEmitExit(int code);

bool ProcessNextTick();

void MakeCallback(const iotjs_jval_t* jfunction,
                  const iotjs_jval_t* jthis,
                  const iotjs_jargs_t* jargs);

iotjs_jval_t MakeCallbackWithResult(const iotjs_jval_t* jfunction,
                                    const iotjs_jval_t* jthis,
                                    const iotjs_jargs_t* jargs);


iotjs_jval_t* InitProcessModule();


} // namespace iotjs


#endif /* IOTJS_BINDING_HELPER_H */
