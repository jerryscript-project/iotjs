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

#include "iotjs_def.h"

/**
 * Demo method
 */
static jerry_value_t demo_method(
    const jerry_value_t func_value, /**< function object */
    const jerry_value_t this_value, /**< this arg */
    const jerry_value_t *args_p,    /**< function arguments */
    const jerry_length_t args_cnt)  /**< number of function arguments */
{
  if (args_cnt < 2) {
    static char *error_msg = "Incorrect parameter count";
    return jerry_create_error(JERRY_ERROR_TYPE,
                              (const jerry_char_t *)error_msg);
  }

  if (!jerry_value_is_number(args_p[0]) || !jerry_value_is_number(args_p[1])) {
    static char *error_msg = "Incorrect parameter type(s)";
    return jerry_create_error(JERRY_ERROR_TYPE,
                              (const jerry_char_t *)error_msg);
  }

  int arg_a = jerry_get_number_value(args_p[0]);
  int arg_b = jerry_get_number_value(args_p[1]);

  return jerry_create_number(arg_a + arg_b);
}

/**
 * Init method called by IoT.js
 */
jerry_value_t Init$MODULE_NAME$() {
  jerry_value_t mymodule = jerry_create_object();
  iotjs_jval_set_property_string_raw(mymodule, "message", "Hello world!");
  iotjs_jval_set_method(mymodule, "add", demo_method);
  return mymodule;
}
