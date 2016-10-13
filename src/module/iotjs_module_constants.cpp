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


#include "uv.h"

#include "iotjs_module.h"
#include "iotjs_module_constants.h"


namespace iotjs {


#define SET_CONSTANT(object, constant) \
  do { \
    JObject value(constant); \
    object->SetProperty(#constant, value); \
  } while (0)


JObject* InitConstants() {
  Module* module = GetBuiltinModule(MODULE_CONSTANTS);
  JObject* constants = module->module;

  if (constants == NULL) {
    constants = new JObject();

    SET_CONSTANT(constants, O_APPEND);
    SET_CONSTANT(constants, O_CREAT);
    SET_CONSTANT(constants, O_EXCL);
    SET_CONSTANT(constants, O_RDONLY);
    SET_CONSTANT(constants, O_RDWR);
    SET_CONSTANT(constants, O_SYNC);
    SET_CONSTANT(constants, O_TRUNC);
    SET_CONSTANT(constants, O_WRONLY);
    SET_CONSTANT(constants, S_IFMT);
    SET_CONSTANT(constants, S_IFDIR);
    SET_CONSTANT(constants, S_IFREG);

    module->module = constants;
  }

  return constants;
}


} // namespace iotjs
