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
#include "iotjs_binding.h"

#include <string.h>


static iotjs_jval_t jglobal;
static iotjs_jargs_t jargs_empty;


iotjs_jval_t iotjs_jval_create_number(double v) {
  return jerry_create_number(v);
}


iotjs_jval_t iotjs_jval_create_string(const iotjs_string_t* v) {
  iotjs_jval_t jval;

  const jerry_char_t* data = (const jerry_char_t*)(iotjs_string_data(v));
  jerry_size_t size = iotjs_string_size(v);

  if (jerry_is_valid_utf8_string(data, size)) {
    jval = jerry_create_string_sz_from_utf8(data, size);
  } else {
    jval = jerry_create_error(JERRY_ERROR_TYPE,
                              (const jerry_char_t*)"Invalid UTF-8 string");
  }

  return jval;
}


iotjs_jval_t iotjs_jval_get_string_size(const iotjs_string_t* str) {
  iotjs_jval_t str_val = iotjs_jval_create_string(str);

  jerry_size_t size = jerry_get_string_size(str_val);
  iotjs_jval_t jval = iotjs_jval_create_number(size);

  jerry_release_value(str_val);

  return jval;
}


iotjs_jval_t iotjs_jval_create_string_raw(const char* data) {
  return jerry_create_string((const jerry_char_t*)data);
}


iotjs_jval_t iotjs_jval_create_object() {
  return jerry_create_object();
}


iotjs_jval_t iotjs_jval_create_array(uint32_t len) {
  return jerry_create_array(len);
}


iotjs_jval_t iotjs_jval_create_byte_array(uint32_t len, const char* data) {
  IOTJS_ASSERT(data != NULL);

  iotjs_jval_t jval = jerry_create_array(len);

  for (uint32_t i = 0; i < len; i++) {
    jerry_value_t val = jerry_create_number((double)data[i]);
    jerry_set_property_by_index(jval, i, val);
    jerry_release_value(val);
  }

  return jval;
}

jerry_value_t iotjs_jval_dummy_function(const jerry_value_t function_obj,
                                        const jerry_value_t this_val,
                                        const jerry_value_t args_p[],
                                        const jerry_length_t args_count) {
  return this_val;
}

iotjs_jval_t iotjs_jval_create_function(JHandlerType handler) {
  iotjs_jval_t jval = jerry_create_external_function(handler);

  IOTJS_ASSERT(jerry_value_is_constructor(jval));

  return jval;
}


iotjs_jval_t iotjs_jval_create_error(const char* msg) {
  return iotjs_jval_create_error_type(IOTJS_ERROR_COMMON, msg);
}


iotjs_jval_t iotjs_jval_create_error_type(iotjs_error_t type, const char* msg) {
  iotjs_jval_t jval;

  const jerry_char_t* jmsg = (const jerry_char_t*)(msg);
  jval = jerry_create_error((jerry_error_t)type, jmsg);
  jerry_value_clear_error_flag(&jval);

  return jval;
}


static iotjs_jval_t iotjs_jval_create_raw(jerry_value_t val) {
  return val;
}


iotjs_jval_t iotjs_jval_get_global_object() {
  return jglobal;
}


#define TYPE_CHECKER_BODY(jval_type)                 \
  bool iotjs_jval_is_##jval_type(iotjs_jval_t val) { \
    return jerry_value_is_##jval_type(val);          \
  }

FOR_EACH_JVAL_TYPES(TYPE_CHECKER_BODY)

#undef TYPE_CHECKER_BODY


bool iotjs_jval_as_boolean(iotjs_jval_t jval) {
  IOTJS_ASSERT(iotjs_jval_is_boolean(jval));
  return jerry_get_boolean_value(jval);
}


double iotjs_jval_as_number(iotjs_jval_t jval) {
  IOTJS_ASSERT(iotjs_jval_is_number(jval));
  return jerry_get_number_value(jval);
}


iotjs_string_t iotjs_jval_as_string(iotjs_jval_t jval) {
  IOTJS_ASSERT(iotjs_jval_is_string(jval));

  jerry_size_t size = jerry_get_string_size(jval);

  if (size == 0)
    return iotjs_string_create();

  char* buffer = iotjs_buffer_allocate(size + 1);
  jerry_char_t* jerry_buffer = (jerry_char_t*)(buffer);

  size_t check = jerry_string_to_char_buffer(jval, jerry_buffer, size);

  IOTJS_ASSERT(check == size);
  buffer[size] = '\0';

  iotjs_string_t res = iotjs_string_create_with_buffer(buffer, size);

  return res;
}


iotjs_jval_t iotjs_jval_as_object(iotjs_jval_t jval) {
  IOTJS_ASSERT(jerry_value_is_object(jval));
  return jval;
}


iotjs_jval_t iotjs_jval_as_array(iotjs_jval_t jval) {
  IOTJS_ASSERT(jerry_value_is_array(jval));
  return jval;
}


iotjs_jval_t iotjs_jval_as_function(iotjs_jval_t jval) {
  IOTJS_ASSERT(jerry_value_is_function(jval));
  return jval;
}


bool iotjs_jval_set_prototype(const iotjs_jval_t jobj, iotjs_jval_t jproto) {
  jerry_value_t ret = jerry_set_prototype(jobj, jproto);
  bool error_found = jerry_value_has_error_flag(ret);
  jerry_release_value(ret);

  return !error_found;
}


void iotjs_jval_set_method(iotjs_jval_t jobj, const char* name,
                           iotjs_native_handler_t handler) {
  IOTJS_ASSERT(iotjs_jval_is_object(jobj));

  iotjs_jval_t jfunc = iotjs_jval_create_function_with_dispatch(handler);
  iotjs_jval_set_property_jval(jobj, name, jfunc);
  jerry_release_value(jfunc);
}


void iotjs_jval_set_property_jval(iotjs_jval_t jobj, const char* name,
                                  iotjs_jval_t value) {
  IOTJS_ASSERT(iotjs_jval_is_object(jobj));

  jerry_value_t prop_name = jerry_create_string((const jerry_char_t*)(name));
  jerry_value_t ret_val = jerry_set_property(jobj, prop_name, value);
  jerry_release_value(prop_name);

  IOTJS_ASSERT(!jerry_value_has_error_flag(ret_val));
  jerry_release_value(ret_val);
}


void iotjs_jval_set_property_null(iotjs_jval_t jobj, const char* name) {
  iotjs_jval_set_property_jval(jobj, name, jerry_create_null());
}


void iotjs_jval_set_property_undefined(iotjs_jval_t jobj, const char* name) {
  iotjs_jval_set_property_jval(jobj, name, jerry_create_undefined());
}


void iotjs_jval_set_property_boolean(iotjs_jval_t jobj, const char* name,
                                     bool v) {
  iotjs_jval_set_property_jval(jobj, name, jerry_create_boolean(v));
}


void iotjs_jval_set_property_number(iotjs_jval_t jobj, const char* name,
                                    double v) {
  iotjs_jval_t jval = iotjs_jval_create_number(v);
  iotjs_jval_set_property_jval(jobj, name, jval);
  jerry_release_value(jval);
}


void iotjs_jval_set_property_string(iotjs_jval_t jobj, const char* name,
                                    const iotjs_string_t* v) {
  iotjs_jval_t jval = iotjs_jval_create_string(v);
  iotjs_jval_set_property_jval(jobj, name, jval);
  jerry_release_value(jval);
}


void iotjs_jval_set_property_string_raw(iotjs_jval_t jobj, const char* name,
                                        const char* v) {
  iotjs_jval_t jval = iotjs_jval_create_string_raw(v);
  iotjs_jval_set_property_jval(jobj, name, jval);
  jerry_release_value(jval);
}


iotjs_jval_t iotjs_jval_get_property(iotjs_jval_t jobj, const char* name) {
  IOTJS_ASSERT(iotjs_jval_is_object(jobj));

  jerry_value_t prop_name = jerry_create_string((const jerry_char_t*)(name));
  jerry_value_t res = jerry_get_property(jobj, prop_name);
  jerry_release_value(prop_name);

  if (jerry_value_has_error_flag(res)) {
    jerry_release_value(res);
    return jerry_acquire_value(jerry_create_undefined());
  }

  return iotjs_jval_create_raw(res);
}


uintptr_t iotjs_jval_get_object_native_handle(iotjs_jval_t jobj) {
  IOTJS_ASSERT(iotjs_jval_is_object(jobj));

  uintptr_t ptr = 0x0;
  JNativeInfoType* out_info;
  jerry_get_object_native_pointer(jobj, (void**)&ptr, &out_info);

  return ptr;
}


uintptr_t iotjs_jval_get_object_from_jhandler(iotjs_jhandler_t* jhandler,
                                              JNativeInfoType* native_info) {
  const iotjs_jval_t jval = JHANDLER_GET_THIS(object);

  if (!jerry_value_is_object(jval)) {
    return 0;
  }

  uintptr_t ptr = 0;
  JNativeInfoType* out_native_info;

  if (jerry_get_object_native_pointer(jval, (void**)&ptr, &out_native_info)) {
    if (ptr && out_native_info == native_info) {
      return ptr;
    }
  }

  JHANDLER_THROW(COMMON, "Unsafe access");

  return 0;
}


uintptr_t iotjs_jval_get_arg_obj_from_jhandler(iotjs_jhandler_t* jhandler,
                                               uint16_t index,
                                               JNativeInfoType* native_info) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_jhandler_t, jhandler);

  if (index >= _this->jargc) {
    return 0;
  }

  const iotjs_jval_t jobj = _this->jargv[index];

  if (!jerry_value_is_object(jobj)) {
    return 0;
  }

  uintptr_t ptr = 0;
  JNativeInfoType* out_native_info;

  if (jerry_get_object_native_pointer(jobj, (void**)&ptr, &out_native_info)) {
    if (ptr && out_native_info == native_info) {
      return ptr;
    }
  }

  JHANDLER_THROW(COMMON, "Unsafe access");

  return 0;
}


void iotjs_jval_set_property_by_index(iotjs_jval_t jarr, uint32_t idx,
                                      iotjs_jval_t jval) {
  IOTJS_ASSERT(iotjs_jval_is_object(jarr));

  jerry_value_t ret_val = jerry_set_property_by_index(jarr, idx, jval);
  IOTJS_ASSERT(!jerry_value_has_error_flag(ret_val));
  jerry_release_value(ret_val);
}


iotjs_jval_t iotjs_jval_get_property_by_index(iotjs_jval_t jarr, uint32_t idx) {
  IOTJS_ASSERT(iotjs_jval_is_object(jarr));

  jerry_value_t res = jerry_get_property_by_index(jarr, idx);

  if (jerry_value_has_error_flag(res)) {
    jerry_release_value(res);
    return jerry_create_undefined();
  }

  return res;
}


#ifndef NDEBUG
static iotjs_jval_t iotjs_jargs_get(const iotjs_jargs_t* jargs,
                                    uint16_t index) {
  const IOTJS_VALIDATED_STRUCT_METHOD(iotjs_jargs_t, jargs);

  IOTJS_ASSERT(index < _this->argc);
  return _this->argv[index];
}
#endif


iotjs_jval_t iotjs_jhelper_call(iotjs_jval_t jfunc, iotjs_jval_t jthis,
                                const iotjs_jargs_t* jargs, bool* throws) {
  IOTJS_ASSERT(iotjs_jval_is_object(jfunc));

  jerry_value_t* jargv_ = NULL;
  jerry_length_t jargc_ = iotjs_jargs_length(jargs);

#ifdef NDEBUG
  jargv_ = (jerry_value_t*)jargs->unsafe.argv;
#else
  if (jargc_ > 0) {
    unsigned buffer_size = sizeof(jerry_value_t) * jargc_;
    jargv_ = (jerry_value_t*)iotjs_buffer_allocate(buffer_size);
    for (unsigned i = 0; i < jargc_; ++i) {
      jargv_[i] = iotjs_jargs_get(jargs, i);
    }
  }
#endif

  jerry_value_t res = jerry_call_function(jfunc, jthis, jargv_, jargc_);

#ifndef NDEBUG
  if (jargv_) {
    iotjs_buffer_release((char*)jargv_);
  }
#endif

  *throws = jerry_value_has_error_flag(res);

  jerry_value_clear_error_flag(&res);

  return iotjs_jval_create_raw(res);
}


iotjs_jval_t iotjs_jhelper_call_ok(iotjs_jval_t jfunc, iotjs_jval_t jthis,
                                   const iotjs_jargs_t* jargs) {
  bool throws;
  iotjs_jval_t jres = iotjs_jhelper_call(jfunc, jthis, jargs, &throws);
  IOTJS_ASSERT(!throws);
  return jres;
}


iotjs_jval_t iotjs_jhelper_eval(const char* name, size_t name_len,
                                const uint8_t* data, size_t size,
                                bool strict_mode, bool* throws) {
  jerry_value_t res =
      jerry_parse_named_resource((const jerry_char_t*)name, name_len,
                                 (const jerry_char_t*)data, size, strict_mode);

  *throws = jerry_value_has_error_flag(res);

  if (!*throws) {
    jerry_value_t func = res;
    res = jerry_run(func);
    jerry_release_value(func);

    *throws = jerry_value_has_error_flag(res);
  }

  jerry_value_clear_error_flag(&res);

  return iotjs_jval_create_raw(res);
}


#ifdef ENABLE_SNAPSHOT
iotjs_jval_t iotjs_jhelper_exec_snapshot(const void* snapshot_p,
                                         size_t snapshot_size, bool* throws) {
  jerry_value_t res = jerry_exec_snapshot(snapshot_p, snapshot_size, false);
  /* the snapshot buffer can be referenced
   * until jerry_cleanup is not called */

  *throws = jerry_value_has_error_flag(res);

  jerry_value_clear_error_flag(&res);

  return iotjs_jval_create_raw(res);
}
#endif


jerry_value_t vm_exec_stop_callback(void* user_p) {
  State* state_p = (State*)user_p;

  if (*state_p != kExiting) {
    return jerry_create_undefined();
  }

  return jerry_create_string((const jerry_char_t*)"Abort script");
}


iotjs_jargs_t iotjs_jargs_create(uint16_t capacity) {
  iotjs_jargs_t jargs;
  IOTJS_VALIDATED_STRUCT_CONSTRUCTOR(iotjs_jargs_t, &jargs);

  _this->capacity = capacity;
  _this->argc = 0;
  if (capacity > 0) {
    unsigned buffer_size = sizeof(iotjs_jval_t) * capacity;
    _this->argv = (iotjs_jval_t*)iotjs_buffer_allocate(buffer_size);
  } else {
    return jargs_empty;
  }

  return jargs;
}


static void iotjs_jargs_initialize_empty(iotjs_jargs_t* jargs) {
  IOTJS_VALIDATED_STRUCT_CONSTRUCTOR(iotjs_jargs_t, jargs);
  _this->capacity = 0;
  _this->argc = 0;
  _this->argv = NULL;
}


const iotjs_jargs_t* iotjs_jargs_get_empty() {
  return &jargs_empty;
}


void iotjs_jargs_destroy(iotjs_jargs_t* jargs) {
  IOTJS_VALIDATED_STRUCT_DESTRUCTOR(iotjs_jargs_t, jargs);

  IOTJS_ASSERT(_this->argv == NULL || _this->argc > 0);
  IOTJS_ASSERT(_this->argc <= _this->capacity);

  if (_this->capacity > 0) {
    for (unsigned i = 0; i < _this->argc; ++i) {
      jerry_release_value(_this->argv[i]);
    }
    iotjs_buffer_release((char*)_this->argv);
  } else {
    IOTJS_ASSERT(_this->argv == NULL);
  }
}


uint16_t iotjs_jargs_length(const iotjs_jargs_t* jargs) {
  const IOTJS_VALIDATED_STRUCT_METHOD(iotjs_jargs_t, jargs);
  return _this->argc;
}


void iotjs_jargs_append_jval(iotjs_jargs_t* jargs, iotjs_jval_t x) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_jargs_t, jargs);
  IOTJS_ASSERT(_this->argc < _this->capacity);
  _this->argv[_this->argc++] = jerry_acquire_value(x);
}


void iotjs_jargs_append_undefined(iotjs_jargs_t* jargs) {
  IOTJS_VALIDATABLE_STRUCT_METHOD_VALIDATE(iotjs_jargs_t, jargs);
  iotjs_jargs_append_jval(jargs, jerry_create_undefined());
}


void iotjs_jargs_append_null(iotjs_jargs_t* jargs) {
  IOTJS_VALIDATABLE_STRUCT_METHOD_VALIDATE(iotjs_jargs_t, jargs);
  iotjs_jargs_append_jval(jargs, jerry_create_null());
}


void iotjs_jargs_append_bool(iotjs_jargs_t* jargs, bool x) {
  IOTJS_VALIDATABLE_STRUCT_METHOD_VALIDATE(iotjs_jargs_t, jargs);
  iotjs_jargs_append_jval(jargs, jerry_create_boolean(x));
}


void iotjs_jargs_append_number(iotjs_jargs_t* jargs, double x) {
  IOTJS_VALIDATABLE_STRUCT_METHOD_VALIDATE(iotjs_jargs_t, jargs);
  iotjs_jval_t jval = iotjs_jval_create_number(x);
  iotjs_jargs_append_jval(jargs, jval);
  jerry_release_value(jval);
}


void iotjs_jargs_append_string(iotjs_jargs_t* jargs, const iotjs_string_t* x) {
  IOTJS_VALIDATABLE_STRUCT_METHOD_VALIDATE(iotjs_jargs_t, jargs);
  iotjs_jval_t jval = iotjs_jval_create_string(x);
  iotjs_jargs_append_jval(jargs, jval);
  jerry_release_value(jval);
}


void iotjs_jargs_append_error(iotjs_jargs_t* jargs, const char* msg) {
  IOTJS_VALIDATABLE_STRUCT_METHOD_VALIDATE(iotjs_jargs_t, jargs);
  iotjs_jval_t error = iotjs_jval_create_error(msg);
  iotjs_jargs_append_jval(jargs, error);
  jerry_release_value(error);
}


void iotjs_jargs_append_string_raw(iotjs_jargs_t* jargs, const char* x) {
  IOTJS_VALIDATABLE_STRUCT_METHOD_VALIDATE(iotjs_jargs_t, jargs);
  iotjs_jval_t jval = iotjs_jval_create_string_raw(x);
  iotjs_jargs_append_jval(jargs, jval);
  jerry_release_value(jval);
}


void iotjs_jargs_replace(iotjs_jargs_t* jargs, uint16_t index, iotjs_jval_t x) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_jargs_t, jargs);

  IOTJS_ASSERT(index < _this->argc);

  jerry_release_value(_this->argv[index]);
  _this->argv[index] = jerry_acquire_value(x);
}


void iotjs_jhandler_initialize(iotjs_jhandler_t* jhandler,
                               const jerry_value_t jfunc,
                               const jerry_value_t jthis,
                               const jerry_value_t jargv[],
                               const uint16_t jargc) {
  IOTJS_VALIDATED_STRUCT_CONSTRUCTOR(iotjs_jhandler_t, jhandler);

  _this->jfunc = iotjs_jval_create_raw(jfunc);
  _this->jthis = iotjs_jval_create_raw(jthis);
  _this->jret = jerry_acquire_value(jerry_create_undefined());
#ifdef NDEBUG
  _this->jargv = (iotjs_jval_t*)jargv;
#else
  if (jargc > 0) {
    unsigned buffer_size = sizeof(iotjs_jval_t) * jargc;
    _this->jargv = (iotjs_jval_t*)iotjs_buffer_allocate(buffer_size);
    for (int i = 0; i < jargc; ++i) {
      _this->jargv[i] = iotjs_jval_create_raw(jargv[i]);
    }
  } else {
    _this->jargv = NULL;
  }
  _this->finished = false;
#endif

  _this->jargc = jargc;
}


void iotjs_jhandler_destroy(iotjs_jhandler_t* jhandler) {
#ifndef NDEBUG
  IOTJS_VALIDATED_STRUCT_DESTRUCTOR(iotjs_jhandler_t, jhandler);
  if (_this->jargc > 0) {
    iotjs_buffer_release((char*)(_this->jargv));
  } else {
    IOTJS_ASSERT(_this->jargv == NULL);
  }
#endif
}


iotjs_jval_t iotjs_jhandler_get_function(iotjs_jhandler_t* jhandler) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_jhandler_t, jhandler);
  return _this->jfunc;
}


iotjs_jval_t iotjs_jhandler_get_this(iotjs_jhandler_t* jhandler) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_jhandler_t, jhandler);
  return _this->jthis;
}


iotjs_jval_t iotjs_jhandler_get_arg(iotjs_jhandler_t* jhandler,
                                    uint16_t index) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_jhandler_t, jhandler);
  IOTJS_ASSERT(index < _this->jargc);
  return _this->jargv[index];
}


uint16_t iotjs_jhandler_get_arg_length(iotjs_jhandler_t* jhandler) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_jhandler_t, jhandler);
  return _this->jargc;
}


void iotjs_jhandler_return_jval(iotjs_jhandler_t* jhandler,
                                iotjs_jval_t ret_value) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_jhandler_t, jhandler);

#ifndef NDEBUG
  IOTJS_ASSERT(_this->finished == false);
#endif

  jerry_release_value(_this->jret);
  _this->jret = jerry_acquire_value(ret_value);
#ifndef NDEBUG
  _this->finished = true;
#endif
}


void iotjs_jhandler_return_undefined(iotjs_jhandler_t* jhandler) {
  IOTJS_VALIDATABLE_STRUCT_METHOD_VALIDATE(iotjs_jhandler_t, jhandler);
  iotjs_jhandler_return_jval(jhandler, jerry_create_undefined());
}


void iotjs_jhandler_return_null(iotjs_jhandler_t* jhandler) {
  IOTJS_VALIDATABLE_STRUCT_METHOD_VALIDATE(iotjs_jhandler_t, jhandler);
  iotjs_jhandler_return_jval(jhandler, jerry_create_null());
}


void iotjs_jhandler_return_boolean(iotjs_jhandler_t* jhandler, bool ret) {
  IOTJS_VALIDATABLE_STRUCT_METHOD_VALIDATE(iotjs_jhandler_t, jhandler);
  iotjs_jhandler_return_jval(jhandler, jerry_create_boolean(ret));
}


void iotjs_jhandler_return_number(iotjs_jhandler_t* jhandler, double ret) {
  IOTJS_VALIDATABLE_STRUCT_METHOD_VALIDATE(iotjs_jhandler_t, jhandler);
  iotjs_jval_t jval = iotjs_jval_create_number(ret);
  iotjs_jhandler_return_jval(jhandler, jval);
  jerry_release_value(jval);
}


void iotjs_jhandler_return_string(iotjs_jhandler_t* jhandler,
                                  const iotjs_string_t* ret) {
  IOTJS_VALIDATABLE_STRUCT_METHOD_VALIDATE(iotjs_jhandler_t, jhandler);
  iotjs_jval_t jval = iotjs_jval_create_string(ret);
  iotjs_jhandler_return_jval(jhandler, jval);
  jerry_release_value(jval);
}


void iotjs_jhandler_return_string_raw(iotjs_jhandler_t* jhandler,
                                      const char* ret) {
  IOTJS_VALIDATABLE_STRUCT_METHOD_VALIDATE(iotjs_jhandler_t, jhandler);
  iotjs_jval_t jval = iotjs_jval_create_string_raw(ret);
  iotjs_jhandler_return_jval(jhandler, jval);
  jerry_release_value(jval);
}


void iotjs_jhandler_throw(iotjs_jhandler_t* jhandler, iotjs_jval_t err) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_jhandler_t, jhandler);
#ifndef NDEBUG
  IOTJS_ASSERT(_this->finished == false);
#endif

  jerry_release_value(_this->jret);
  _this->jret = jerry_acquire_value(err);
  jerry_value_set_error_flag(&_this->jret);

#ifndef NDEBUG
  _this->finished = true;
#endif
}


void iotjs_jhandler_error(iotjs_jhandler_t* jhandler, const char* func_name) {
  char buffer[64];
  snprintf(buffer, 63, "Internal error (%s)", func_name);
  JHANDLER_THROW(COMMON, buffer)
}


static jerry_value_t iotjs_native_dispatch_function(
    const jerry_value_t jfunc, const jerry_value_t jthis,
    const jerry_value_t jargv[], const JRawLengthType jargc) {
  uintptr_t target_function_ptr = 0x0;
  JNativeInfoType* out_info;

  if (!jerry_get_object_native_pointer(jfunc, (void**)&target_function_ptr,
                                       &out_info)) {
    const jerry_char_t* jmsg = (const jerry_char_t*)("Internal dispatch error");
    return jerry_create_error((jerry_error_t)IOTJS_ERROR_COMMON, jmsg);
  }

  IOTJS_ASSERT(target_function_ptr != 0x0);

  iotjs_jhandler_t jhandler;
  iotjs_jhandler_initialize(&jhandler, jfunc, jthis, jargv, jargc);

  ((iotjs_native_handler_t)target_function_ptr)(&jhandler);

  jerry_value_t ret_val = jhandler.unsafe.jret;
  iotjs_jhandler_destroy(&jhandler);
  return ret_val;
}


iotjs_jval_t iotjs_jval_create_function_with_dispatch(
    iotjs_native_handler_t handler) {
  iotjs_jval_t jfunc =
      iotjs_jval_create_function(iotjs_native_dispatch_function);
  jerry_set_object_native_pointer(jfunc, handler, NULL);
  return jfunc;
}


void iotjs_binding_initialize() {
  jglobal = jerry_get_global_object();

  IOTJS_ASSERT(iotjs_jval_is_object(jglobal));

  iotjs_jargs_initialize_empty(&jargs_empty);

#ifdef NDEBUG
  assert(sizeof(iotjs_jval_t) == sizeof(jerry_value_t));
#endif
}


void iotjs_binding_finalize() {
  jerry_release_value(jglobal);
  iotjs_jargs_destroy(&jargs_empty);
}
