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

#include "iotjs_util.h"
#include "jerryscript.h"

#include <stdio.h>


typedef jerry_external_handler_t JHandlerType;
typedef const jerry_object_native_info_t JNativeInfoType;
typedef jerry_length_t JRawLengthType;
typedef jerry_value_t iotjs_jval_t;

/* Constructors */
iotjs_jval_t iotjs_jval_create_number(double v);
iotjs_jval_t iotjs_jval_create_string(const iotjs_string_t* v);
iotjs_jval_t iotjs_jval_create_string_raw(const char* data);
iotjs_jval_t iotjs_jval_create_object();
iotjs_jval_t iotjs_jval_create_array(uint32_t len);
iotjs_jval_t iotjs_jval_create_byte_array(uint32_t len, const char* data);
jerry_value_t iotjs_jval_dummy_function(const jerry_value_t function_obj,
                                        const jerry_value_t this_val,
                                        const jerry_value_t args_p[],
                                        const jerry_length_t args_count);
iotjs_jval_t iotjs_jval_create_function(JHandlerType handler);
iotjs_jval_t iotjs_jval_create_error(const char* msg);
iotjs_jval_t iotjs_jval_create_error_type(jerry_error_t type, const char* msg);

iotjs_jval_t iotjs_jval_get_string_size(const iotjs_string_t* str);


/* Type Converters */
bool iotjs_jval_as_boolean(iotjs_jval_t);
double iotjs_jval_as_number(iotjs_jval_t);
iotjs_string_t iotjs_jval_as_string(iotjs_jval_t);
iotjs_jval_t iotjs_jval_as_object(iotjs_jval_t);
iotjs_jval_t iotjs_jval_as_array(iotjs_jval_t);
iotjs_jval_t iotjs_jval_as_function(iotjs_jval_t);

/* Methods for General JavaScript Object */
void iotjs_jval_set_method(iotjs_jval_t jobj, const char* name,
                           jerry_external_handler_t handler);
void iotjs_jval_set_property_jval(iotjs_jval_t jobj, const char* name,
                                  iotjs_jval_t value);
void iotjs_jval_set_property_null(iotjs_jval_t jobj, const char* name);
void iotjs_jval_set_property_undefined(iotjs_jval_t jobj, const char* name);
void iotjs_jval_set_property_boolean(iotjs_jval_t jobj, const char* name,
                                     bool v);
void iotjs_jval_set_property_number(iotjs_jval_t jobj, const char* name,
                                    double v);
void iotjs_jval_set_property_string(iotjs_jval_t jobj, const char* name,
                                    const iotjs_string_t* v);
void iotjs_jval_set_property_string_raw(iotjs_jval_t jobj, const char* name,
                                        const char* v);

iotjs_jval_t iotjs_jval_get_property(iotjs_jval_t jobj, const char* name);

uintptr_t iotjs_jval_get_object_native_handle(iotjs_jval_t jobj);

void iotjs_jval_set_property_by_index(iotjs_jval_t jarr, uint32_t idx,
                                      iotjs_jval_t jval);
iotjs_jval_t iotjs_jval_get_property_by_index(iotjs_jval_t jarr, uint32_t idx);


typedef struct {
  uint16_t capacity;
  uint16_t argc;
  iotjs_jval_t* argv;
} IOTJS_VALIDATED_STRUCT(iotjs_jargs_t);


iotjs_jargs_t iotjs_jargs_create(uint16_t capacity);

const iotjs_jargs_t* iotjs_jargs_get_empty();

void iotjs_jargs_destroy(iotjs_jargs_t* jargs);

uint16_t iotjs_jargs_length(const iotjs_jargs_t* jargs);

void iotjs_jargs_append_jval(iotjs_jargs_t* jargs, iotjs_jval_t x);
void iotjs_jargs_append_undefined(iotjs_jargs_t* jargs);
void iotjs_jargs_append_null(iotjs_jargs_t* jargs);
void iotjs_jargs_append_bool(iotjs_jargs_t* jargs, bool x);
void iotjs_jargs_append_number(iotjs_jargs_t* jargs, double x);
void iotjs_jargs_append_string(iotjs_jargs_t* jargs, const iotjs_string_t* x);
void iotjs_jargs_append_string_raw(iotjs_jargs_t* jargs, const char* x);
void iotjs_jargs_append_error(iotjs_jargs_t* jargs, const char* msg);


void iotjs_jargs_replace(iotjs_jargs_t* jargs, uint16_t index, iotjs_jval_t x);

// Calls JavaScript function.
iotjs_jval_t iotjs_jhelper_call(iotjs_jval_t jfunc, iotjs_jval_t jthis,
                                const iotjs_jargs_t* jargs, bool* throws);

// Calls javascript function.
iotjs_jval_t iotjs_jhelper_call_ok(iotjs_jval_t jfunc, iotjs_jval_t jthis,
                                   const iotjs_jargs_t* jargs);

// Evaluates javascript source file.
iotjs_jval_t iotjs_jhelper_eval(const char* name, size_t name_len,
                                const uint8_t* data, size_t size,
                                bool strict_mode, bool* throws);

#define JS_CREATE_ERROR(TYPE, message) \
  jerry_create_error(JERRY_ERROR_##TYPE, (const jerry_char_t*)message);

#define JS_CHECK(predicate)                           \
  if (!(predicate)) {                                 \
    return JS_CREATE_ERROR(COMMON, "Internal error"); \
  }

#define JS_CHECK_TYPE(jval, type) JS_CHECK(jerry_value_is_##type(jval));

#define JS_CHECK_ARG(index, type) JS_CHECK_TYPE(jargv[index], type);

#define JS_CHECK_ARG_IF_EXIST(index, type) \
  if (jargc > index) {                     \
    JS_CHECK_TYPE(jargv[index], type);     \
  }

#define JS_CHECK_ARGS_0()

#define JS_CHECK_ARGS_1(type0) \
  JS_CHECK_ARGS_0();           \
  JS_CHECK_ARG(0, type0);

#define JS_CHECK_ARGS_2(type0, type1) \
  JS_CHECK_ARGS_1(type0);             \
  JS_CHECK_ARG(1, type1);

#define JS_CHECK_ARGS_3(type0, type1, type2) \
  JS_CHECK_ARGS_2(type0, type1);             \
  JS_CHECK_ARG(2, type2);

#define JS_CHECK_ARGS_4(type0, type1, type2, type3) \
  JS_CHECK_ARGS_3(type0, type1, type2);             \
  JS_CHECK_ARG(3, type3);

#define JS_CHECK_ARGS_5(type0, type1, type2, type3, type4) \
  JS_CHECK_ARGS_4(type0, type1, type2, type3);             \
  JS_CHECK_ARG(4, type4);

#define JS_CHECK_ARGS(argc, ...) \
  JS_CHECK(jargc >= argc);       \
  JS_CHECK_ARGS_##argc(__VA_ARGS__)

#define JS_CHECK_THIS(type) JS_CHECK_TYPE(jthis, type);

#define JS_GET_ARG(index, type) iotjs_jval_as_##type(jargv[index])

#define JS_GET_ARG_IF_EXIST(index, type)                  \
  ((jargc > index) && jerry_value_is_##type(jargv[index]) \
       ? jargv[index]                                     \
       : jerry_create_null())

#define JS_GET_THIS(type) iotjs_jval_as_##type(jthis)

#define JS_FUNCTION(name)                                \
  static jerry_value_t name(const jerry_value_t jfunc,   \
                            const jerry_value_t jthis,   \
                            const jerry_value_t jargv[], \
                            const jerry_length_t jargc)


#if defined(EXPERIMENTAL) && !defined(DEBUG)
// This code branch is to be in #ifdef NDEBUG
#define DJS_CHECK_ARG(index, type) ((void)0)
#define DJS_CHECK_ARGS(argc, ...) ((void)0)
#define DJS_CHECK_THIS(type) ((void)0)
#define DJS_CHECK_ARG_IF_EXIST(index, type) ((void)0)
#else
#define DJS_CHECK_ARG(index, type) JS_CHECK_ARG(index, type)
#define DJS_CHECK_ARGS(argc, ...) JS_CHECK_ARGS(argc, __VA_ARGS__)
#define DJS_CHECK_THIS(type) JS_CHECK_THIS(type)
#define DJS_CHECK_ARG_IF_EXIST(index, type) JS_CHECK_ARG_IF_EXIST(index, type)
#endif

#define JS_DECLARE_THIS_PTR(type, name)                                      \
  iotjs_##type##_t* name;                                                    \
  do {                                                                       \
    JNativeInfoType* out_native_info;                                        \
    jerry_get_object_native_pointer(jthis, (void**)&name, &out_native_info); \
    if (!name || out_native_info != &this_module_native_info) {              \
      return JS_CREATE_ERROR(COMMON, "");                                    \
    }                                                                        \
  } while (0)

#define JS_DECLARE_OBJECT_PTR(index, type, name)                 \
  iotjs_##type##_t* name;                                        \
  do {                                                           \
    JNativeInfoType* out_native_info;                            \
    jerry_get_object_native_pointer(jargv[index], (void**)&name, \
                                    &out_native_info);           \
    if (!name || out_native_info != &this_module_native_info) {  \
      return JS_CREATE_ERROR(COMMON, "");                        \
    }                                                            \
  } while (0)

#define DJS_GET_REQUIRED_CONF_VALUE(src, target, property, type)            \
  do {                                                                      \
    iotjs_jval_t jtmp = iotjs_jval_get_property(src, property);             \
    if (jerry_value_is_undefined(jtmp)) {                                   \
      return JS_CREATE_ERROR(TYPE, "Missing argument, required " property); \
    } else if (jerry_value_is_##type(jtmp)) {                               \
      target = iotjs_jval_as_##type(jtmp);                                  \
    } else {                                                                \
      return JS_CREATE_ERROR(TYPE, "Bad arguments, required " property      \
                                   " is not a " #type);                     \
    }                                                                       \
    jerry_release_value(jtmp);                                              \
  } while (0)

jerry_value_t vm_exec_stop_callback(void* user_p);

#endif /* IOTJS_BINDING_H */
