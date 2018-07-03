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

#ifndef IOTJS_BINDING_H
#define IOTJS_BINDING_H

#include <stdbool.h>
#include <string.h>

#include "iotjs_util.h"
#include "jerryscript.h"

#include <stdio.h>


typedef const jerry_object_native_info_t JNativeInfoType;

/* Constructors */
jerry_value_t iotjs_jval_create_string(const iotjs_string_t* v);
jerry_value_t iotjs_jval_create_byte_array(uint32_t len, const char* data);
jerry_value_t iotjs_jval_create_function(jerry_external_handler_t handler);
jerry_value_t iotjs_jval_create_error_without_error_flag(const char* msg);

/* Type Converters */
bool iotjs_jval_as_boolean(jerry_value_t);
double iotjs_jval_as_number(jerry_value_t);
iotjs_string_t iotjs_jval_as_string(jerry_value_t);
jerry_value_t iotjs_jval_as_object(jerry_value_t);
jerry_value_t iotjs_jval_as_array(jerry_value_t);
jerry_value_t iotjs_jval_as_function(jerry_value_t);
bool iotjs_jbuffer_as_string(jerry_value_t jval, iotjs_string_t* out_string);

/* Temporary Buffers for JavaScript Values */

typedef struct {
  jerry_value_t jval;
  char* buffer;
  size_t length;
} iotjs_tmp_buffer_t;

void iotjs_jval_as_tmp_buffer(jerry_value_t jval,
                              iotjs_tmp_buffer_t* out_buffer);
void iotjs_jval_get_jproperty_as_tmp_buffer(jerry_value_t jobj,
                                            const char* name,
                                            iotjs_tmp_buffer_t* out_buffer);
void iotjs_free_tmp_buffer(iotjs_tmp_buffer_t* tmp_buffer);

/* Methods for General JavaScript Object */
void iotjs_jval_set_method(jerry_value_t jobj, const char* name,
                           jerry_external_handler_t handler);
bool iotjs_jval_set_prototype(jerry_value_t jobj, jerry_value_t jproto);
void iotjs_jval_set_property_jval(jerry_value_t jobj, const char* name,
                                  jerry_value_t value);
void iotjs_jval_set_property_null(jerry_value_t jobj, const char* name);
void iotjs_jval_set_property_undefined(jerry_value_t jobj, const char* name);
void iotjs_jval_set_property_boolean(jerry_value_t jobj, const char* name,
                                     bool v);
void iotjs_jval_set_property_number(jerry_value_t jobj, const char* name,
                                    double v);
void iotjs_jval_set_property_string(jerry_value_t jobj, const char* name,
                                    const iotjs_string_t* v);
void iotjs_jval_set_property_string_raw(jerry_value_t jobj, const char* name,
                                        const char* v);

jerry_value_t iotjs_jval_get_property(jerry_value_t jobj, const char* name);

uintptr_t iotjs_jval_get_object_native_handle(jerry_value_t jobj);

void iotjs_jval_set_property_by_index(jerry_value_t jarr, uint32_t idx,
                                      jerry_value_t jval);
jerry_value_t iotjs_jval_get_property_by_index(jerry_value_t jarr,
                                               uint32_t idx);

// Evaluates javascript source file.
jerry_value_t iotjs_jhelper_eval(const char* name, size_t name_len,
                                 const uint8_t* data, size_t size,
                                 bool strict_mode);

/* Note:
 *      Defines started with underscores should not be used
 *      outside of this header.
 */
#define JS_CREATE_ERROR(TYPE, message) \
  jerry_create_error(JERRY_ERROR_##TYPE, (const jerry_char_t*)message)

#define jerry_value_is_any(value) true
#define iotjs_jval_as_any(value) (value)

#define JS_CHECK(predicate)                           \
  if (!(predicate)) {                                 \
    return JS_CREATE_ERROR(COMMON, "Internal error"); \
  }

#define __JS_CHECK_TYPE(index, type) jerry_value_is_##type(jargv[index])

#define JS_CHECK_ARG(index, type) JS_CHECK(__JS_CHECK_TYPE(index, type))

#define JS_CHECK_ARG_IF_EXIST(index, type) \
  if (jargc > index) {                     \
    JS_CHECK(__JS_CHECK_TYPE(index, type)) \
  }

#define JS_CHECK_ARGS_0()

#define JS_CHECK_ARGS_1(type0) \
  JS_CHECK_ARGS_0()            \
  __JS_CHECK_TYPE(0, type0)

#define JS_CHECK_ARGS_2(type0, type1) \
  JS_CHECK_ARGS_1(type0)              \
  &&__JS_CHECK_TYPE(1, type1)

#define JS_CHECK_ARGS_3(type0, type1, type2) \
  JS_CHECK_ARGS_2(type0, type1)              \
  &&__JS_CHECK_TYPE(2, type2)

#define JS_CHECK_ARGS_4(type0, type1, type2, type3) \
  JS_CHECK_ARGS_3(type0, type1, type2)              \
  &&__JS_CHECK_TYPE(3, type3)

#define JS_CHECK_ARGS_5(type0, type1, type2, type3, type4) \
  JS_CHECK_ARGS_4(type0, type1, type2, type3)              \
  &&__JS_CHECK_TYPE(4, type4)

#define JS_CHECK_ARGS(argc, ...) \
  JS_CHECK(jargc >= argc && JS_CHECK_ARGS_##argc(__VA_ARGS__))

#define JS_CHECK_THIS() JS_CHECK(jerry_value_is_object(jthis))

#define JS_GET_ARG(index, type) iotjs_jval_as_##type(jargv[index])

#define JS_GET_ARG_IF_EXIST(index, type)                  \
  ((jargc > index) && jerry_value_is_##type(jargv[index]) \
       ? jargv[index]                                     \
       : jerry_create_null())

#define JS_GET_THIS() iotjs_jval_as_object(jthis)

#define JS_FUNCTION(name)                                \
  static jerry_value_t name(const jerry_value_t jfunc,   \
                            const jerry_value_t jthis,   \
                            const jerry_value_t jargv[], \
                            const jerry_length_t jargc)

#if defined(EXPERIMENTAL) && !defined(DEBUG)
// This code branch is to be in #ifdef NDEBUG
#define DJS_CHECK_ARG(index, type) ((void)0)
#define DJS_CHECK_ARGS(argc, ...) ((void)0)
#define DJS_CHECK_THIS() ((void)0)
#define DJS_CHECK_ARG_IF_EXIST(index, type) ((void)0)
#else
#define DJS_CHECK_ARG(index, type) JS_CHECK_ARG(index, type)
#define DJS_CHECK_ARGS(argc, ...) JS_CHECK_ARGS(argc, __VA_ARGS__)
#define DJS_CHECK_THIS() JS_CHECK_THIS()
#define DJS_CHECK_ARG_IF_EXIST(index, type) JS_CHECK_ARG_IF_EXIST(index, type)
#endif

#define __JS_DECLARE_PTR(type, name, value)                                  \
  iotjs_##type##_t* name;                                                    \
  do {                                                                       \
    JNativeInfoType* out_native_info;                                        \
    jerry_get_object_native_pointer(value, (void**)&name, &out_native_info); \
    if (!name || out_native_info != &this_module_native_info) {              \
      return JS_CREATE_ERROR(COMMON, "");                                    \
    }                                                                        \
  } while (0)

#define JS_DECLARE_THIS_PTR(type, name) __JS_DECLARE_PTR(type, name, jthis)

#define JS_DECLARE_OBJECT_PTR(index, type, name) \
  __JS_DECLARE_PTR(type, name, jargv[index])

#define __JS_GET_REQUIRED_VALUE(target, property, type, value)              \
  do {                                                                      \
    if (jerry_value_is_undefined(value)) {                                  \
      return JS_CREATE_ERROR(TYPE, "Missing argument, required " property); \
    } else if (jerry_value_is_##type(value)) {                              \
      target = iotjs_jval_as_##type(value);                                 \
    } else {                                                                \
      return JS_CREATE_ERROR(TYPE, "Bad arguments, required " property      \
                                   " is not a " #type);                     \
    }                                                                       \
  } while (0)

#define JS_GET_REQUIRED_ARG_VALUE(index, target, property, type) \
  __JS_GET_REQUIRED_VALUE(target, property, type, jargv[index])

#define JS_GET_REQUIRED_CONF_VALUE(src, target, property, type)  \
  do {                                                           \
    jerry_value_t jtmp = iotjs_jval_get_property(src, property); \
    __JS_GET_REQUIRED_VALUE(target, property, type, jtmp);       \
    jerry_release_value(jtmp);                                   \
  } while (0)

jerry_value_t vm_exec_stop_callback(void* user_p);

/**
 * Dynamic module defintions (.iotjs)
 */
#define IOTJS_CURRENT_MODULE_VERSION ((uint32_t)1)

typedef jerry_value_t (*ModuleInitializer)(void);

typedef struct {
  uint32_t iotjs_module_version;
  uint32_t module_version;
  ModuleInitializer initializer;
} iotjs_module;

typedef iotjs_module* (*iotjs_module_info_getter)(void);

#define IOTJS_MODULE_ENTRYPOINT iotjs_module_info
#define IOTJS_MODULE(IOTJS_VERSION, MODULE_VERSION, NAME) \
  static const iotjs_module __module = {                  \
    IOTJS_VERSION, MODULE_VERSION, init_##NAME,           \
  };                                                      \
  const iotjs_module* IOTJS_MODULE_ENTRYPOINT(void) {     \
    return &__module;                                     \
  }

#endif /* IOTJS_BINDING_H */
