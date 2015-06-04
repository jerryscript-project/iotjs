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


#include "iotjs_def.h"
#include "iotjs_module_stream.h"


namespace iotjs {


JHANDLER_FUNCTION(DoWrite, handler) {
  IOTJS_ASSERT(handler.GetArgLength() == 3);
  IOTJS_ASSERT(handler.GetArg(0)->IsObject());
  IOTJS_ASSERT(handler.GetArg(1)->IsObject());
  IOTJS_ASSERT(handler.GetArg(2)->IsFunction());

  // FIXME: Implement me.

  return true;
}


JObject* InitStream() {
  Module* module = GetBuiltinModule(MODULE_STREAM);
  JObject* stream = module->module;

  if (stream == NULL) {
    stream = new JObject();
    stream->SetMethod("doWrite", DoWrite);

    module->module = stream;
  }

  return stream;
}


} // namespace iotjs
