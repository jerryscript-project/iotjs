/* Copyright 2018-present Samsung Electronics Co., Ltd. and other contributors
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

#include <iotjs_binding.h>

jerry_value_t init_$MODULE_NAME$(void) {
  jerry_value_t object = jerry_create_object();

  jerry_value_t prop_name = jerry_create_string((const jerry_char_t*)"demokey");
  jerry_value_t prop_value = jerry_create_number(3.4);

  jerry_set_property(object, prop_name, prop_value);

  jerry_release_value(prop_name);
  jerry_release_value(prop_value);

  return object;
}

IOTJS_MODULE(IOTJS_CURRENT_MODULE_VERSION, 1, $MODULE_NAME$)
