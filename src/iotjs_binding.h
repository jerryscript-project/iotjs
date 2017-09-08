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


typedef enum {
  IOTJS_ERROR_COMMON = JERRY_ERROR_COMMON,
  IOTJS_ERROR_EVAL = JERRY_ERROR_EVAL,
  IOTJS_ERROR_RANGE = JERRY_ERROR_RANGE,
  IOTJS_ERROR_REFERENCE = JERRY_ERROR_REFERENCE,
  IOTJS_ERROR_SYNTAX = JERRY_ERROR_SYNTAX,
  IOTJS_ERROR_TYPE = JERRY_ERROR_TYPE,
  IOTJS_ERROR_URI = JERRY_ERROR_URI
} iotjs_error_t;


#define FOR_EACH_JVAL_TYPES(F) \
  F(undefined)                 \
  F(null)                      \
  F(boolean)                   \
  F(number)                    \
  F(string)                    \
  F(object)                    \
  F(array)                     \
  F(function)


typedef struct {
  jerry_value_t value; // JavaScript value representation
} IOTJS_VALIDATED_STRUCT(iotjs_jval_t);

typedef struct {
  iotjs_jval_t jfunc;
  iotjs_jval_t jthis;
  iotjs_jval_t* jargv;
  iotjs_jval_t jret;
  uint16_t jargc;
#ifndef NDEBUG
  bool finished;
#endif
} IOTJS_VALIDATED_STRUCT(iotjs_jhandler_t);

typedef void (*iotjs_native_handler_t)(iotjs_jhandler_t* jhandler);


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
iotjs_jval_t iotjs_jval_create_error_type(iotjs_error_t type, const char* msg);
iotjs_jval_t iotjs_jval_create_copied(const iotjs_jval_t* other);

iotjs_jval_t iotjs_jval_get_string_size(const iotjs_string_t* str);

iotjs_jval_t* iotjs_jval_get_undefined();
iotjs_jval_t* iotjs_jval_get_null();
iotjs_jval_t* iotjs_jval_get_boolean(bool v);
iotjs_jval_t* iotjs_jval_get_global_object();

/* Destructor */
void iotjs_jval_destroy(iotjs_jval_t* jval);

#define THIS_JVAL const iotjs_jval_t* jval

/* Type Checkers */
bool iotjs_jval_is_undefined(THIS_JVAL);
bool iotjs_jval_is_null(THIS_JVAL);
bool iotjs_jval_is_boolean(THIS_JVAL);
bool iotjs_jval_is_number(THIS_JVAL);
bool iotjs_jval_is_string(THIS_JVAL);
bool iotjs_jval_is_object(THIS_JVAL);
bool iotjs_jval_is_array(THIS_JVAL);
bool iotjs_jval_is_function(THIS_JVAL);

/* Type Converters */
bool iotjs_jval_as_boolean(THIS_JVAL);
double iotjs_jval_as_number(THIS_JVAL);
iotjs_string_t iotjs_jval_as_string(THIS_JVAL);
const iotjs_jval_t* iotjs_jval_as_object(THIS_JVAL);
const iotjs_jval_t* iotjs_jval_as_array(THIS_JVAL);
const iotjs_jval_t* iotjs_jval_as_function(THIS_JVAL);

/* Methods for General JavaScript Object */
bool iotjs_jval_set_prototype(const iotjs_jval_t* jobj, iotjs_jval_t* jproto);
void iotjs_jval_set_method(THIS_JVAL, const char* name,
                           iotjs_native_handler_t handler);
void iotjs_jval_set_property_jval(THIS_JVAL, const char* name,
                                  const iotjs_jval_t* value);
void iotjs_jval_set_property_null(THIS_JVAL, const char* name);
void iotjs_jval_set_property_undefined(THIS_JVAL, const char* name);
void iotjs_jval_set_property_boolean(THIS_JVAL, const char* name, bool v);
void iotjs_jval_set_property_number(THIS_JVAL, const char* name, double v);
void iotjs_jval_set_property_string(THIS_JVAL, const char* name,
                                    const iotjs_string_t* v);
void iotjs_jval_set_property_string_raw(THIS_JVAL, const char* name,
                                        const char* v);

iotjs_jval_t iotjs_jval_get_property(THIS_JVAL, const char* name);

void iotjs_jval_set_object_native_handle(THIS_JVAL, uintptr_t ptr,
                                         JNativeInfoType* native_info);
uintptr_t iotjs_jval_get_object_native_handle(THIS_JVAL);
uintptr_t iotjs_jval_get_object_from_jhandler(iotjs_jhandler_t* jhandler,
                                              JNativeInfoType* native_info);
uintptr_t iotjs_jval_get_arg_obj_from_jhandler(iotjs_jhandler_t* jhandler,
                                               uint16_t index,
                                               JNativeInfoType* native_info);

void iotjs_jval_set_property_by_index(THIS_JVAL, uint32_t idx,
                                      const iotjs_jval_t* value);
iotjs_jval_t iotjs_jval_get_property_by_index(THIS_JVAL, uint32_t idx);


#undef THIS_JVAL


typedef struct {
  uint16_t capacity;
  uint16_t argc;
  iotjs_jval_t* argv;
} IOTJS_VALIDATED_STRUCT(iotjs_jargs_t);


iotjs_jargs_t iotjs_jargs_create(uint16_t capacity);

const iotjs_jargs_t* iotjs_jargs_get_empty();

void iotjs_jargs_destroy(iotjs_jargs_t* jargs);

uint16_t iotjs_jargs_length(const iotjs_jargs_t* jargs);

void iotjs_jargs_append_jval(iotjs_jargs_t* jargs, const iotjs_jval_t* x);
void iotjs_jargs_append_undefined(iotjs_jargs_t* jargs);
void iotjs_jargs_append_null(iotjs_jargs_t* jargs);
void iotjs_jargs_append_bool(iotjs_jargs_t* jargs, bool x);
void iotjs_jargs_append_number(iotjs_jargs_t* jargs, double x);
void iotjs_jargs_append_string(iotjs_jargs_t* jargs, const iotjs_string_t* x);
void iotjs_jargs_append_string_raw(iotjs_jargs_t* jargs, const char* x);
void iotjs_jargs_append_error(iotjs_jargs_t* jargs, const char* msg);


void iotjs_jargs_replace(iotjs_jargs_t* jargs, uint16_t index,
                         const iotjs_jval_t* x);

const iotjs_jval_t* iotjs_jargs_get(const iotjs_jargs_t* jargs, uint16_t index);


// Calls JavaScript function.
iotjs_jval_t iotjs_jhelper_call(const iotjs_jval_t* jfunc,
                                const iotjs_jval_t* jthis,
                                const iotjs_jargs_t* jargs, bool* throws);

// Calls javascript function.
iotjs_jval_t iotjs_jhelper_call_ok(const iotjs_jval_t* jfunc,
                                   const iotjs_jval_t* jthis,
                                   const iotjs_jargs_t* jargs);

// Evaluates javascript source file.
iotjs_jval_t iotjs_jhelper_eval(const char* name, size_t name_len,
                                const uint8_t* data, size_t size,
                                bool strict_mode, bool* throws);
#ifdef ENABLE_SNAPSHOT
// Evaluates javascript snapshot.
iotjs_jval_t iotjs_jhelper_exec_snapshot(const void* snapshot_p,
                                         size_t snapshot_size, bool* throws);
#endif


void iotjs_jhandler_initialize(iotjs_jhandler_t* jhandler,
                               const jerry_value_t jfunc,
                               const jerry_value_t jthis,
                               const jerry_value_t jargv[],
                               const uint16_t jargc);

void iotjs_jhandler_destroy(iotjs_jhandler_t* jhandler);

const iotjs_jval_t* iotjs_jhandler_get_function(iotjs_jhandler_t* jhandler);
const iotjs_jval_t* iotjs_jhandler_get_this(iotjs_jhandler_t* jhandler);
const iotjs_jval_t* iotjs_jhandler_get_arg(iotjs_jhandler_t* jhandler,
                                           uint16_t index);
uint16_t iotjs_jhandler_get_arg_length(iotjs_jhandler_t* jhandler);

void iotjs_jhandler_return_jval(iotjs_jhandler_t* jhandler,
                                const iotjs_jval_t* ret);
void iotjs_jhandler_return_undefined(iotjs_jhandler_t* jhandler);
void iotjs_jhandler_return_null(iotjs_jhandler_t* jhandler);
void iotjs_jhandler_return_boolean(iotjs_jhandler_t* jhandler, bool x);
void iotjs_jhandler_return_number(iotjs_jhandler_t* jhandler, double x);
void iotjs_jhandler_return_string(iotjs_jhandler_t* jhandler,
                                  const iotjs_string_t* x);
void iotjs_jhandler_return_string_raw(iotjs_jhandler_t* jhandler,
                                      const char* x);

void iotjs_jhandler_throw(iotjs_jhandler_t* jhandler, const iotjs_jval_t* err);

iotjs_jval_t iotjs_jval_create_function_with_dispatch(
    iotjs_native_handler_t handler);


#define JHANDLER_THROW(TYPE, message)                                         \
  iotjs_jval_t e = iotjs_jval_create_error_type(IOTJS_ERROR_##TYPE, message); \
  iotjs_jhandler_throw(jhandler, &e);                                         \
  iotjs_jval_destroy(&e);

#define JHANDLER_CHECK(predicate)                          \
  if (!(predicate)) {                                      \
    char buffer[64];                                       \
    snprintf(buffer, 63, "Internal error (%s)", __func__); \
    JHANDLER_THROW(COMMON, buffer)                         \
    return;                                                \
  }

#define JHANDLER_CHECK_TYPE(jval, type) \
  JHANDLER_CHECK(iotjs_jval_is_##type(jval));

#define JHANDLER_CHECK_ARG(index, type) \
  JHANDLER_CHECK_TYPE(iotjs_jhandler_get_arg(jhandler, index), type);

#define JHANDLER_CHECK_ARG_IF_EXIST(index, type)                        \
  if (iotjs_jhandler_get_arg_length(jhandler) > index) {                \
    JHANDLER_CHECK_TYPE(iotjs_jhandler_get_arg(jhandler, index), type); \
  }

#define JHANDLER_CHECK_ARGS_0()

#define JHANDLER_CHECK_ARGS_1(type0) \
  JHANDLER_CHECK_ARGS_0();           \
  JHANDLER_CHECK_ARG(0, type0);

#define JHANDLER_CHECK_ARGS_2(type0, type1) \
  JHANDLER_CHECK_ARGS_1(type0);             \
  JHANDLER_CHECK_ARG(1, type1);

#define JHANDLER_CHECK_ARGS_3(type0, type1, type2) \
  JHANDLER_CHECK_ARGS_2(type0, type1);             \
  JHANDLER_CHECK_ARG(2, type2);

#define JHANDLER_CHECK_ARGS_4(type0, type1, type2, type3) \
  JHANDLER_CHECK_ARGS_3(type0, type1, type2);             \
  JHANDLER_CHECK_ARG(3, type3);

#define JHANDLER_CHECK_ARGS_5(type0, type1, type2, type3, type4) \
  JHANDLER_CHECK_ARGS_4(type0, type1, type2, type3);             \
  JHANDLER_CHECK_ARG(4, type4);

// Workaround for GCC type-limits warning
static inline bool ge(uint16_t a, uint16_t b) {
  return a >= b;
}

#define JHANDLER_CHECK_ARGS(argc, ...)                               \
  JHANDLER_CHECK(ge(iotjs_jhandler_get_arg_length(jhandler), argc)); \
  JHANDLER_CHECK_ARGS_##argc(__VA_ARGS__)

#define JHANDLER_CHECK_THIS(type) \
  JHANDLER_CHECK_TYPE(iotjs_jhandler_get_this(jhandler), type);

#define JHANDLER_GET_ARG(index, type) \
  iotjs_jval_as_##type(iotjs_jhandler_get_arg(jhandler, index))

#define JHANDLER_GET_ARG_IF_EXIST(index, type)                          \
  ((iotjs_jhandler_get_arg_length(jhandler) > index)                    \
       ? (iotjs_jval_is_##type(iotjs_jhandler_get_arg(jhandler, index)) \
              ? iotjs_jhandler_get_arg(jhandler, index)                 \
              : NULL)                                                   \
       : NULL)

#define JHANDLER_GET_THIS(type) \
  iotjs_jval_as_##type(iotjs_jhandler_get_this(jhandler))

#define JHANDLER_FUNCTION(name) static void name(iotjs_jhandler_t* jhandler)

#if defined(EXPERIMENTAL) && !defined(DEBUG)
// This code branch is to be in #ifdef NDEBUG
#define DJHANDLER_CHECK_ARG(index, type) ((void)0)
#define DJHANDLER_CHECK_ARGS(argc, ...) ((void)0)
#define DJHANDLER_CHECK_THIS(type) ((void)0)
#define DJHANDLER_CHECK_ARG_IF_EXIST(index, type) ((void)0)
#else
#define DJHANDLER_CHECK_ARG(index, type) JHANDLER_CHECK_ARG(index, type)
#define DJHANDLER_CHECK_ARGS(argc, ...) JHANDLER_CHECK_ARGS(argc, __VA_ARGS__)
#define DJHANDLER_CHECK_THIS(type) JHANDLER_CHECK_THIS(type)
#define DJHANDLER_CHECK_ARG_IF_EXIST(index, type) \
  JHANDLER_CHECK_ARG_IF_EXIST(index, type)
#endif

#define JHANDLER_DECLARE_THIS_PTR(type, name)                                  \
  iotjs_##type##_t* name = (iotjs_##type##_t*)                                 \
      iotjs_jval_get_object_from_jhandler(jhandler, &this_module_native_info); \
  if (!name) {                                                                 \
    return;                                                                    \
  }

#define JHANDLER_DECLARE_OBJECT_PTR(index, type, name)                \
  iotjs_##type##_t* name = (iotjs_##type##_t*)                        \
      iotjs_jval_get_arg_obj_from_jhandler(jhandler, index,           \
                                           &this_module_native_info); \
  if (!name) {                                                        \
    return;                                                           \
  }

#define DJHANDLER_GET_REQUIRED_CONF_VALUE(src, target, property, type)        \
  do {                                                                        \
    iotjs_jval_t jtmp = iotjs_jval_get_property(src, property);               \
    if (iotjs_jval_is_undefined(&jtmp)) {                                     \
      JHANDLER_THROW(TYPE, "Missing argument, required " property);           \
      return;                                                                 \
    } else if (iotjs_jval_is_##type(&jtmp))                                   \
      target = iotjs_jval_as_##type(&jtmp);                                   \
    else {                                                                    \
      JHANDLER_THROW(TYPE,                                                    \
                     "Bad arguments, required " property " is not a " #type); \
      return;                                                                 \
    }                                                                         \
    iotjs_jval_destroy(&jtmp);                                                \
  } while (0);

void iotjs_binding_initialize();
void iotjs_binding_finalize();

jerry_value_t vm_exec_stop_callback(void* user_p);

#endif /* IOTJS_BINDING_H */
