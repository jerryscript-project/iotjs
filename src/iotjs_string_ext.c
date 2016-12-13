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

//
// strings in iotjs built-in scripts
//
#include "iotjs_string_ext.inl.h"

//
// declare string items
//
#define MAGICSTR_EX_DEF(NAME, STRING) \
  static const char jerry_magic_string_ex_##NAME[] = STRING;

JERRY_MAGIC_STRING_ITEMS

#undef MAGICSTR_EX_DEF


//
// declare strings length array
//
static const jerry_length_t magic_string_lengths[] = {
#define MAGICSTR_EX_DEF(NAME, STRING) \
  (jerry_length_t)(sizeof(jerry_magic_string_ex_##NAME) - 1u),

  JERRY_MAGIC_STRING_ITEMS

#undef MAGICSTR_EX_DEF
};


//
// declare strings table
//
static const jerry_char_ptr_t magic_string_items[] = {
#define MAGICSTR_EX_DEF(NAME, STRING) \
  (const jerry_char_ptr_t) jerry_magic_string_ex_##NAME,

  JERRY_MAGIC_STRING_ITEMS

#undef MAGICSTR_EX_DEF
};


void iotjs_register_jerry_magic_string(void) {
  uint32_t num_magic_string_items =
      (uint32_t)(sizeof(magic_string_items) / sizeof(jerry_char_ptr_t));
  jerry_register_magic_strings(magic_string_items, num_magic_string_items,
                               magic_string_lengths);
}
