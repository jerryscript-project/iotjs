/* Copyright 2016-present Samsung Electronics Co., Ltd. and other contributors
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
#include "iotjs_module_stm32f4dis.h"


iotjs_jval_t InitStm32f4dis() {
  iotjs_jval_t stm32f4dis = iotjs_jval_create_object();

#if defined(__NUTTX__)

  iotjs_stm32f4dis_pin_initialize(&stm32f4dis);

#endif

  return stm32f4dis;
}
