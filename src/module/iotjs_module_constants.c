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

#include "iotjs_def.h"
#include "iotjs_module.h"


#define SET_CONSTANT(object, constant)                           \
  do {                                                           \
    iotjs_jval_set_property_number(object, #constant, constant); \
  } while (0)

iotjs_jval_t InitConstants() {
  iotjs_jval_t constants = iotjs_jval_create_object();

  SET_CONSTANT(&constants, O_APPEND);
  SET_CONSTANT(&constants, O_CREAT);
  SET_CONSTANT(&constants, O_EXCL);
  SET_CONSTANT(&constants, O_RDONLY);
  SET_CONSTANT(&constants, O_RDWR);
  SET_CONSTANT(&constants, O_SYNC);
  SET_CONSTANT(&constants, O_TRUNC);
  SET_CONSTANT(&constants, O_WRONLY);
  SET_CONSTANT(&constants, S_IFMT);
  SET_CONSTANT(&constants, S_IFDIR);
  SET_CONSTANT(&constants, S_IFREG);

  return constants;
}
