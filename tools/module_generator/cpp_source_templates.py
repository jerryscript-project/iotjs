#!/usr/bin/env python

# Copyright 2019-present Samsung Electronics Co., Ltd. and other contributors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.


# Templates for create/set a C++ variable

# String to char[]
JS_TO_STRING = '''
  // create an array of characters from a jerry_value_t
  {TYPE} * {NAME} = NULL;
  if (jerry_value_is_string ({JVAL}))
  {{
    jerry_size_t {NAME}_size = jerry_get_string_size ({JVAL});
    {NAME} = new {TYPE}[{NAME}_size + 1];
    jerry_string_to_char_buffer ({JVAL}, (jerry_char_t*){NAME}, {NAME}_size);
    {NAME}[{NAME}_size] = '\\0';
  }}
'''

JS_FREE_STRING = '''
  // TODO: if you won't use {NAME} pointer, uncomment the line below
  //if (jerry_value_is_string ({JVAL}))
  //  delete[] {NAME};
'''

# Set a char* variable
JS_SET_CHAR_PTR = '''
  // set the value of {NAME}
  jerry_size_t size = jerry_get_string_size ({JVAL});
  if ({NAME} == NULL)
  {{
    {NAME} = new {TYPE}[size + 1];
  }}
  jerry_string_to_char_buffer ({JVAL}, (jerry_char_t*){NAME}, size);
  {NAME}[size] = '\\0';
'''

# TypedArray to number pointer
JS_TO_TYPEDARRAY = '''
  // create a pointer to number from a jerry_value_t
  {TYPE} * {NAME} = NULL;
  jerry_length_t {NAME}_byteLength = 0;
  jerry_length_t {NAME}_byteOffset = 0;
  jerry_value_t {NAME}_buffer;
  if (jerry_value_is_typedarray ({JVAL}))
  {{
    {NAME}_buffer = jerry_get_typedarray_buffer ({JVAL}, &{NAME}_byteOffset, &{NAME}_byteLength);
    {NAME} = new {TYPE}[{NAME}_byteLength / sizeof({TYPE})];
    jerry_arraybuffer_read ({NAME}_buffer, {NAME}_byteOffset, (uint8_t*){NAME}, {NAME}_byteLength);
  }}
'''

JS_FREE_WRITE_BUFFER = '''
  // write the values back into an arraybuffer from a pointer
  if (jerry_value_is_typedarray ({JVAL}))
  {{
    jerry_arraybuffer_write ({NAME}_buffer, {NAME}_byteOffset, (uint8_t*){NAME}, {NAME}_byteLength);
    jerry_release_value ({NAME}_buffer);
    // TODO: if you won't use {NAME} pointer, uncomment the line below
    //delete[] {NAME};
  }}
'''

# Set a number pointer
JS_SET_TYPEDARRAY = '''
  // set the value of {NAME}
  jerry_length_t byteLength = 0;
  jerry_length_t byteOffset = 0;
  jerry_value_t buffer;
  if (jerry_value_is_typedarray ({JVAL}))
  {{
    buffer = jerry_get_typedarray_buffer ({JVAL}, &byteOffset, &byteLength);
    if ({NAME} == NULL)
    {{
      {NAME} = new {TYPE}[byteLength / sizeof({TYPE})];
    }}
    jerry_arraybuffer_read (buffer, byteOffset, (uint8_t*){NAME}, byteLength);
    jerry_release_value (buffer);
  }}
  else
  {{
    {NAME} = NULL;
  }}
'''

# Return Object
JS_RETURN_OBJECT = '''
  // create object from record
  jerry_value_t {NAME} = {RECORD}_js_creator({FROM});
  jerry_set_object_native_pointer({NAME}, {FROM}, &{RECORD}_type_info);
'''

# Alloc record
JS_ALLOC_RECORD = '''
  {RECORD}* {NAME} = ({RECORD}*)calloc(1, sizeof({RECORD}));
'''


# Template for check js type

JS_VALUE_IS = '''jerry_value_is_{TYPE} ({JVAL})'''

JS_POINTER_IS = '''(jerry_value_is_{TYPE} ({JVAL}) || jerry_value_is_null ({JVAL}))'''

JS_CHECK_RECORD = '''
bool jerry_value_is_{RECORD} (jerry_value_t jval)
{{
  if (!jerry_value_is_object (jval))
  {{
    return false;
  }}

  void* ptr;
  const jerry_object_native_info_t* type_ptr;
  bool has_ptr = jerry_get_object_native_pointer(jval, &ptr, &type_ptr);

  if (!has_ptr ||
      (type_ptr != &{RECORD}_type_info && type_ptr != &{RECORD}_type_info_static))
  {{
    return false;
  }}

  return true;
}}
'''


# Templates for record types

# Record destructor
JS_RECORD_DESTRUCTOR = '''
void {RECORD}_js_destructor(void* ptr) {{
    delete ({TYPE}*)ptr;
}}

static const jerry_object_native_info_t {RECORD}_type_info = {{
    .free_cb = {RECORD}_js_destructor
}};

static const jerry_object_native_info_t {RECORD}_type_info_static = {{
  .free_cb = NULL
}};
'''

# Record constructor
JS_RECORD_CONSTRUCTOR = '''
// external function for record constructor
jerry_value_t {RECORD}{SUFF}_js_constructor (const jerry_value_t function_obj,
                                             const jerry_value_t this_val,
                                             const jerry_value_t args_p[],
                                             const jerry_length_t args_cnt)
{{
  {TYPE}* native_ptr;
  switch (args_cnt) {{
 {CASE}
     default: {{
       char const *msg = "Wrong argument count for {RECORD} constructor.";
       return jerry_create_error (JERRY_ERROR_TYPE, (const jerry_char_t*)msg);
     }}
  }}

  jerry_value_t ret_val = {RECORD}_js_creator(native_ptr);
  jerry_set_object_native_pointer(ret_val, native_ptr, &{RECORD}_type_info);
  return ret_val;
}}
'''

JS_CONSTR_CALL = '''
      if ({CONDITION})
      {{
        {GET_VAL}
        native_ptr = new {NAME}({PARAMS});
        {FREE}
        break;
      }}
'''

JS_CONSTR_CASE_0 = '''
    case 0: {{
      native_ptr = new {NAME}();
      break;
    }}
'''

JS_CONSTR_CASE = '''
    case {NUM}: {{
{CALLS}
    char const *msg = "Wrong argument type for {NAME} constructor.";
    return jerry_create_error (JERRY_ERROR_TYPE, (const jerry_char_t*)msg);
    }}
'''

JS_REGIST_METHOD = '''
  // set record method as a property to the object
  jerry_value_t {NAME}_name = jerry_create_string ((const jerry_char_t*)"{NAME}");
  jerry_value_t {NAME}_func = jerry_create_external_function ({RECORD}_{NAME}_handler);
  jerry_value_t {NAME}_ret = jerry_set_property (js_obj, {NAME}_name, {NAME}_func);
  jerry_release_value ({NAME}_name);
  jerry_release_value ({NAME}_func);
  jerry_release_value ({NAME}_ret);
'''

JS_REGIST_CONST_MEMBER = '''
  // set a constant record member as a property to the object
  jerry_property_descriptor_t {RECORD}_{NAME}_prop_desc;
  jerry_init_property_descriptor_fields (&{RECORD}_{NAME}_prop_desc);
  {RECORD}_{NAME}_prop_desc.is_value_defined = true;
  {RECORD}_{NAME}_prop_desc.value = {RECORD}_{NAME}_js;
  jerry_value_t {RECORD}_{NAME}_prop_name = jerry_create_string ((const jerry_char_t *)"{NAME}");
  jerry_value_t {RECORD}_{NAME}_return_value = jerry_define_own_property (js_obj, {RECORD}_{NAME}_prop_name, &{RECORD}_{NAME}_prop_desc);
  jerry_release_value ({RECORD}_{NAME}_return_value);
  jerry_release_value ({RECORD}_{NAME}_prop_name);
  jerry_free_property_descriptor_fields (&{RECORD}_{NAME}_prop_desc);
'''

# Record method
JS_RECORD_METHOD = '''
// external function for record method
jerry_value_t {RECORD}_{NAME}{SUFF}_handler (const jerry_value_t function_obj,
                                             const jerry_value_t this_val,
                                             const jerry_value_t args_p[],
                                             const jerry_length_t args_cnt)
{{
  void* void_ptr;
  const jerry_object_native_info_t* type_ptr;
  bool has_ptr = jerry_get_object_native_pointer(this_val, &void_ptr, &type_ptr);

  if (!has_ptr ||
      (type_ptr != &{RECORD}_type_info && type_ptr != &{RECORD}_type_info_static)) {{
    char const *msg = "Failed to get native {RECORD} pointer";
    return jerry_create_error(JERRY_ERROR_TYPE, (const jerry_char_t *)msg);
  }}

  {TYPE}* native_ptr = ({TYPE}*)(void_ptr);

  {RESULT}
  switch (args_cnt) {{
{CASE}
    default: {{
      char const *msg = "Wrong argument count for {RECORD}.{NAME}().";
      return jerry_create_error (JERRY_ERROR_TYPE, (const jerry_char_t*)msg);
    }}
  }}

  {RET_VAL}
  return ret_val;
}}
'''

JS_METHOD_CALL = '''
      if ({CONDITION})
      {{
        {GET_VAL}
        {RESULT}native_ptr->{NAME}({PARAMS});
        {FREE}
        break;
      }}
'''

JS_METHOD_CASE_0 = '''
    case 0: {{
      {RESULT}native_ptr->{NAME}();
      break;
    }}
'''

JS_METHOD_CASE = '''
    case {NUM}: {{
{CALLS}
    char const *msg = "Wrong argument type for {RECORD}.{NAME}().";
    return jerry_create_error (JERRY_ERROR_TYPE, (const jerry_char_t*)msg);
    }}
'''


# Templates for C++ functions

# Function
JS_EXT_CPP_FUNC = '''
// external function for API functions
jerry_value_t {NAME}{SUFF}_handler (const jerry_value_t function_obj,
                                    const jerry_value_t this_val,
                                    const jerry_value_t args_p[],
                                    const jerry_length_t args_cnt)
{{
  {RESULT}
  switch (args_cnt) {{
{CASE}
    default: {{
      char const *msg = "Wrong argument count for {NAME}().";
      return jerry_create_error (JERRY_ERROR_TYPE, (const jerry_char_t*)msg);
    }}
  }}

  {RET_VAL}
  return ret_val;
}}
'''

JS_FUNC_CALL = '''
      if ({CONDITION})
      {{
        {GET_VAL}
        {RESULT}{NAME}({PARAMS});
        {FREE}
        break;
      }}
'''

JS_FUNC_CASE_0 = '''
    case 0: {{
      {RESULT}{NAME}();
      break;
    }}
'''

JS_FUNC_CASE = '''
    case {NUM}: {{
{CALLS}
    char const *msg = "Wrong argument type for {NAME}().";
    return jerry_create_error (JERRY_ERROR_TYPE, (const jerry_char_t*)msg);
    }}
'''


# Templates for the module initialization function

INIT_FUNC = '''
// init function for the module
extern "C" jerry_value_t Init_{NAME}()
{{
{BODY}
  return object;
}}
'''


# Template for include the right headers

INCLUDE = '''
#include <cstdlib>
#include <cstring>
#include "jerryscript.h"
#include "{HEADER}"
'''


# Templates for modules.json, module.cmake and CMakeLists.txt

MODULES_JSON = '''
{{
  "modules": {{
    "{NAME}_module": {{
      "native_files": [],
      "init": "Init_{NAME}",
      "cmakefile": "{CMAKE}"
    }}
  }}
}}
'''

MODULE_CMAKE = '''
set(MODULE_NAME "{NAME}_module")
add_subdirectory(${{MODULE_DIR}}/src/ ${{MODULE_BINARY_DIR}}/${{MODULE_NAME}})
link_directories(${{MODULE_DIR}})
list(APPEND MODULE_LIBS {NAME}_binding {LIBRARY} stdc++)
'''

CMAKE_LISTS = '''
project({NAME} CXX)

add_library({NAME}_binding STATIC
    {NAME}_js_binding.cpp
)
target_include_directories({NAME}_binding PRIVATE ${{JERRY_INCLUDE_DIR}})
target_link_libraries({NAME}_binding PUBLIC stdc++)
'''
