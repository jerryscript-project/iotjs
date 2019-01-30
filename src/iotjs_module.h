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

#ifndef IOTJS_MODULE_H
#define IOTJS_MODULE_H

#include "iotjs_binding.h"

typedef jerry_value_t (*register_func)(void);

typedef struct {
  const char* name;
  register_func fn_register;
} iotjs_module_ro_data_t;

extern const unsigned iotjs_module_count;
extern const iotjs_module_ro_data_t iotjs_module_ro_data[];

void iotjs_module_list_cleanup(void);

jerry_value_t iotjs_module_get(const char* name);

#endif /* IOTJS_MODULE_H */
