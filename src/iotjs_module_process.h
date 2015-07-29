/* Copyright 2015 Samsung Electronics Co., Ltd.
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

#ifndef IOTJS_MODULE_PROCESS_H
#define IOTJS_MODULE_PROCESS_H

#include "iotjs_binding.h"


namespace iotjs {

void UncaughtException(JObject& jexception);

void ProcessEmitExit(int code);

bool ProcessNextTick();

JObject MakeCallback(JObject& function, JObject& this_, JArgList& args);

void InitArgv(int argc, char** argv);

JObject* InitProcess();

} // namespace iotjs

#endif /* IOTJS_MODULE_PROCESS_H */
