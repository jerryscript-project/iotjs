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
#include "iotjs_js.h"

#include <string.h>


static iotjs_jargs_t jargs_empty;


jerry_value_t iotjs_jval_create_string(const iotjs_string_t* v) {
  jerry_value_t jval;

  const jerry_char_t* data = (const jerry_char_t*)(iotjs_string_data(v));
  jerry_size_t size = iotjs_string_size(v);

  if (jerry_is_valid_utf8_string(data, size)) {
    jval = jerry_create_string_sz_from_utf8(data, size);
  } else {
    jval = JS_CREATE_ERROR(TYPE, "Invalid UTF-8 string");
  }

  return jval;
}


jerry_value_t iotjs_jval_get_string_size(const iotjs_string_t* str) {
  jerry_value_t str_val = iotjs_jval_create_string(str);

  jerry_size_t size = jerry_get_string_size(str_val);
  jerry_value_t jval = jerry_create_number(size);

  jerry_release_value(str_val);

  return jval;
}


jerry_value_t iotjs_jval_create_byte_array(uint32_t len, const char* data) {
  IOTJS_ASSERT(data != NULL);

  jerry_value_t jval = jerry_create_array(len);

  for (uint32_t i = 0; i < len; i++) {
    jerry_value_t val = jerry_create_number((double)data[i]);
    jerry_set_property_by_index(jval, i, val);
    jerry_release_value(val);
  }

  return jval;
}


jerry_value_t iotjs_jval_create_function(jerry_external_handler_t handler) {
  jerry_value_t jval = jerry_create_external_function(handler);

  IOTJS_ASSERT(jerry_value_is_constructor(jval));

  return jval;
}


jerry_value_t iotjs_jval_create_error_without_error_flag(const char* msg) {
  jerry_value_t jval =
      jerry_create_error(JERRY_ERROR_COMMON, (const jerry_char_t*)(msg));
  jerry_value_clear_error_flag(&jval);

  return jval;
}


bool iotjs_jval_as_boolean(jerry_value_t jval) {
  IOTJS_ASSERT(jerry_value_is_boolean(jval));
  return jerry_get_boolean_value(jval);
}


double iotjs_jval_as_number(jerry_value_t jval) {
  IOTJS_ASSERT(jerry_value_is_number(jval));
  return jerry_get_number_value(jval);
}


iotjs_string_t iotjs_jval_as_string(jerry_value_t jval) {
  IOTJS_ASSERT(jerry_value_is_string(jval));

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


jerry_value_t iotjs_jval_as_object(jerry_value_t jval) {
  IOTJS_ASSERT(jerry_value_is_object(jval));
  return jval;
}


jerry_value_t iotjs_jval_as_array(jerry_value_t jval) {
  IOTJS_ASSERT(jerry_value_is_array(jval));
  return jval;
}


jerry_value_t iotjs_jval_as_function(jerry_value_t jval) {
  IOTJS_ASSERT(jerry_value_is_function(jval));
  return jval;
}


bool iotjs_jval_set_prototype(const jerry_value_t jobj, jerry_value_t jproto) {
  jerry_value_t ret = jerry_set_prototype(jobj, jproto);
  bool error_found = jerry_value_has_error_flag(ret);
  jerry_release_value(ret);

  return !error_found;
}


void iotjs_jval_set_method(jerry_value_t jobj, const char* name,
                           jerry_external_handler_t handler) {
  IOTJS_ASSERT(jerry_value_is_object(jobj));

  jerry_value_t jfunc = jerry_create_external_function(handler);
  iotjs_jval_set_property_jval(jobj, name, jfunc);
  jerry_release_value(jfunc);
}


void iotjs_jval_set_property_jval(jerry_value_t jobj, const char* name,
                                  jerry_value_t value) {
  IOTJS_ASSERT(jerry_value_is_object(jobj));

  jerry_value_t prop_name = jerry_create_string((const jerry_char_t*)(name));
  jerry_value_t ret_val = jerry_set_property(jobj, prop_name, value);
  jerry_release_value(prop_name);

  IOTJS_ASSERT(!jerry_value_has_error_flag(ret_val));
  jerry_release_value(ret_val);
}


void iotjs_jval_set_property_null(jerry_value_t jobj, const char* name) {
  iotjs_jval_set_property_jval(jobj, name, jerry_create_null());
}


void iotjs_jval_set_property_undefined(jerry_value_t jobj, const char* name) {
  iotjs_jval_set_property_jval(jobj, name, jerry_create_undefined());
}


void iotjs_jval_set_property_boolean(jerry_value_t jobj, const char* name,
                                     bool v) {
  iotjs_jval_set_property_jval(jobj, name, jerry_create_boolean(v));
}


void iotjs_jval_set_property_number(jerry_value_t jobj, const char* name,
                                    double v) {
  jerry_value_t jval = jerry_create_number(v);
  iotjs_jval_set_property_jval(jobj, name, jval);
  jerry_release_value(jval);
}


void iotjs_jval_set_property_string(jerry_value_t jobj, const char* name,
                                    const iotjs_string_t* v) {
  jerry_value_t jval = iotjs_jval_create_string(v);
  iotjs_jval_set_property_jval(jobj, name, jval);
  jerry_release_value(jval);
}


void iotjs_jval_set_property_string_raw(jerry_value_t jobj, const char* name,
                                        const char* v) {
  jerry_value_t jval = jerry_create_string((const jerry_char_t*)v);
  iotjs_jval_set_property_jval(jobj, name, jval);
  jerry_release_value(jval);
}


jerry_value_t iotjs_jval_get_property(jerry_value_t jobj, const char* name) {
  IOTJS_ASSERT(jerry_value_is_object(jobj));

  jerry_value_t prop_name = jerry_create_string((const jerry_char_t*)(name));
  jerry_value_t res = jerry_get_property(jobj, prop_name);
  jerry_release_value(prop_name);

  if (jerry_value_has_error_flag(res)) {
    jerry_release_value(res);
    return jerry_create_undefined();
  }

  return res;
}


uintptr_t iotjs_jval_get_object_native_handle(jerry_value_t jobj) {
  IOTJS_ASSERT(jerry_value_is_object(jobj));

  uintptr_t ptr = 0x0;
  JNativeInfoType* out_info;
  jerry_get_object_native_pointer(jobj, (void**)&ptr, &out_info);

  return ptr;
}


void iotjs_jval_set_property_by_index(jerry_value_t jarr, uint32_t idx,
                                      jerry_value_t jval) {
  IOTJS_ASSERT(jerry_value_is_object(jarr));

  jerry_value_t ret_val = jerry_set_property_by_index(jarr, idx, jval);
  IOTJS_ASSERT(!jerry_value_has_error_flag(ret_val));
  jerry_release_value(ret_val);
}


jerry_value_t iotjs_jval_get_property_by_index(jerry_value_t jarr,
                                               uint32_t idx) {
  IOTJS_ASSERT(jerry_value_is_object(jarr));

  jerry_value_t res = jerry_get_property_by_index(jarr, idx);

  if (jerry_value_has_error_flag(res)) {
    jerry_release_value(res);
    return jerry_create_undefined();
  }

  return res;
}


jerry_value_t iotjs_jhelper_call(jerry_value_t jfunc, jerry_value_t jthis,
                                 const iotjs_jargs_t* jargs) {
  IOTJS_ASSERT(jerry_value_is_object(jfunc));

  jerry_length_t jargc_ = iotjs_jargs_length(jargs);

  jerry_value_t jres = jerry_call_function(jfunc, jthis, jargs->argv, jargc_);

  return jres;
}


jerry_value_t iotjs_jhelper_call_ok(jerry_value_t jfunc, jerry_value_t jthis,
                                    const iotjs_jargs_t* jargs) {
  jerry_value_t jres = iotjs_jhelper_call(jfunc, jthis, jargs);
  IOTJS_ASSERT(!jerry_value_has_error_flag(jres));
  return jres;
}


jerry_value_t iotjs_jhelper_eval(const char* name, size_t name_len,
                                 const uint8_t* data, size_t size,
                                 bool strict_mode) {
  jerry_value_t jres =
      jerry_parse_named_resource((const jerry_char_t*)name, name_len,
                                 (const jerry_char_t*)data, size, strict_mode);

  if (!jerry_value_has_error_flag(jres)) {
    jerry_value_t func = jres;
    jres = jerry_run(func);
    jerry_release_value(func);
  }

  return jres;
}


jerry_value_t vm_exec_stop_callback(void* user_p) {
  State* state_p = (State*)user_p;

  if (*state_p != kExiting) {
    return jerry_create_undefined();
  }

  return jerry_create_string((const jerry_char_t*)"Abort script");
}


iotjs_jargs_t iotjs_jargs_create(uint16_t capacity) {
  if (capacity == 0) {
    return jargs_empty;
  }

  iotjs_jargs_t jargs;

  jargs.capacity = capacity;
  jargs.argc = 0;
  unsigned buffer_size = sizeof(jerry_value_t) * capacity;
  jargs.argv = (jerry_value_t*)iotjs_buffer_allocate(buffer_size);

  return jargs;
}


const iotjs_jargs_t* iotjs_jargs_get_empty() {
  return &jargs_empty;
}


void iotjs_jargs_destroy(iotjs_jargs_t* jargs) {
  IOTJS_ASSERT(jargs && jargs->argc <= jargs->capacity);

  if (jargs->capacity > 0) {
    IOTJS_ASSERT(jargs->argv);
    for (unsigned i = 0; i < jargs->argc; ++i) {
      jerry_release_value(jargs->argv[i]);
    }
    iotjs_buffer_release((char*)jargs->argv);
  } else {
    IOTJS_ASSERT(jargs->argv == NULL);
  }
}


uint16_t iotjs_jargs_length(const iotjs_jargs_t* jargs) {
  IOTJS_ASSERT(jargs);
  return jargs->argc;
}


void iotjs_jargs_append_jval(iotjs_jargs_t* jargs, jerry_value_t x) {
  IOTJS_ASSERT(jargs && jargs->argc < jargs->capacity);
  IOTJS_ASSERT(jargs->argv);
  jargs->argv[jargs->argc++] = jerry_acquire_value(x);
}


void iotjs_jargs_append_undefined(iotjs_jargs_t* jargs) {
  iotjs_jargs_append_jval(jargs, jerry_create_undefined());
}


void iotjs_jargs_append_null(iotjs_jargs_t* jargs) {
  iotjs_jargs_append_jval(jargs, jerry_create_null());
}


void iotjs_jargs_append_bool(iotjs_jargs_t* jargs, bool x) {
  iotjs_jargs_append_jval(jargs, jerry_create_boolean(x));
}


void iotjs_jargs_append_number(iotjs_jargs_t* jargs, double x) {
  jerry_value_t jval = jerry_create_number(x);
  iotjs_jargs_append_jval(jargs, jval);
  jerry_release_value(jval);
}


void iotjs_jargs_append_string(iotjs_jargs_t* jargs, const iotjs_string_t* x) {
  jerry_value_t jval = iotjs_jval_create_string(x);
  iotjs_jargs_append_jval(jargs, jval);
  jerry_release_value(jval);
}


void iotjs_jargs_append_error(iotjs_jargs_t* jargs, const char* msg) {
  jerry_value_t error = iotjs_jval_create_error_without_error_flag(msg);
  iotjs_jargs_append_jval(jargs, error);
  jerry_release_value(error);
}


void iotjs_jargs_append_string_raw(iotjs_jargs_t* jargs, const char* x) {
  jerry_value_t jval = jerry_create_string((const jerry_char_t*)x);
  iotjs_jargs_append_jval(jargs, jval);
  jerry_release_value(jval);
}


void iotjs_jargs_replace(iotjs_jargs_t* jargs, uint16_t index,
                         jerry_value_t x) {
  IOTJS_ASSERT(jargs && index < jargs->argc);
  IOTJS_ASSERT(jargs->argv);

  jerry_release_value(jargs->argv[index]);
  jargs->argv[index] = jerry_acquire_value(x);
}
