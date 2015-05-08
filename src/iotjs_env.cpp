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

#include "iotjs_binding.h"

#include "iotjs_env.h"


namespace iotjs {

Environment::Environment(uv_loop_t* loop)
  : _loop(loop) {
}

Environment* Environment::GetEnv() {
  JObject global = JObject::Global();
  return (Environment*)global.GetNative();
}

} // namespace iotjs
