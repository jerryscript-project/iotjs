/* Copyright 2019-present Samsung Electronics Co., Ltd. and other contributors
 * Copyright 2018-present Rokid Co., Ltd. and other contributors
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

#include "jerryscript-ext/handle-scope.h"
#include "jerryscript.h"
#include <stdlib.h>
#include "internal/node_api_internal.h"
#include "modules/iotjs_module_buffer.h"

#include <math.h>

/* Feature missing error messages */
const char* const napi_err_no_dataview =
    "DataView is not supported by this build.";
const char* const napi_err_no_promise =
    "Promise is not supported by this build.";
const char* const napi_err_no_symbol =
    "Symbols are not supported by this build.";
const char* const napi_err_no_typedarray =
    "TypedArray is not supported by this build.";
const char* const napi_err_invalid_deferred =
    "Invalid deferred object. Please refer to the documentation.";

static void iotjs_napi_buffer_external_free_cb(void* native_p) {
  iotjs_buffer_external_info_t* info = (iotjs_buffer_external_info_t*)native_p;

  napi_env env = info->env;
  void* external_data = info->external_data;
  void* finalize_hint = info->finalize_hint;
  napi_finalize finalize_cb = info->finalize_cb;
  if (finalize_cb != NULL) {
    finalize_cb(env, external_data, finalize_hint);
  }

  IOTJS_RELEASE(info);
}

napi_status napi_assign_bool(bool value, bool* result) {
  NAPI_ASSIGN(result, value);
  NAPI_RETURN(napi_ok);
}

napi_status napi_assign_nvalue(jerry_value_t jvalue, napi_value* nvalue) {
  NAPI_ASSIGN(nvalue, AS_NAPI_VALUE(jvalue));
  NAPI_RETURN(napi_ok);
}

napi_status napi_create_array(napi_env env, napi_value* result) {
  NAPI_TRY_ENV(env);
  JERRYX_CREATE(jval, jerry_create_array(0));
  return napi_assign_nvalue(jval, result);
}

napi_status napi_create_array_with_length(napi_env env, size_t length,
                                          napi_value* result) {
  NAPI_TRY_ENV(env);
  JERRYX_CREATE(jval, jerry_create_array(length));
  return napi_assign_nvalue(jval, result);
}

napi_status napi_create_arraybuffer(napi_env env, size_t byte_length,
                                    void** data, napi_value* result) {
  NAPI_TRY_ENV(env);

  if (!jerry_is_feature_enabled(JERRY_FEATURE_TYPEDARRAY)) {
    NAPI_ASSIGN(data, NULL);
    NAPI_ASSIGN(result, NULL);
    NAPI_RETURN_WITH_MSG(napi_generic_failure, napi_err_no_typedarray);
  }

  JERRYX_CREATE(jval, jerry_create_arraybuffer(byte_length));
  uint8_t* data_ptr = jerry_get_arraybuffer_pointer(jval);
  NAPI_ASSIGN(data, data_ptr);
  NAPI_ASSIGN(result, AS_NAPI_VALUE(jval));
  NAPI_RETURN(napi_ok);
}

static void iotjs_napi_arraybuffer_external_free_cb(void* native_p) {
  IOTJS_UNUSED(native_p);
}

napi_status napi_create_external_arraybuffer(napi_env env, void* external_data,
                                             size_t byte_length,
                                             napi_finalize finalize_cb,
                                             void* finalize_hint,
                                             napi_value* result) {
  NAPI_TRY_ENV(env);
  NAPI_WEAK_ASSERT_WITH_MSG(napi_invalid_arg, external_data != NULL,
                            "External data pointer could not be NULL.");
  NAPI_WEAK_ASSERT_WITH_MSG(napi_invalid_arg, byte_length != 0,
                            "External data byte length could not be 0.");

  if (!jerry_is_feature_enabled(JERRY_FEATURE_TYPEDARRAY)) {
    NAPI_ASSIGN(result, NULL);
    NAPI_RETURN_WITH_MSG(napi_generic_failure, napi_err_no_typedarray);
  }

  JERRYX_CREATE(jval_arrbuf, jerry_create_arraybuffer_external(
                                 byte_length, external_data,
                                 iotjs_napi_arraybuffer_external_free_cb));

  iotjs_object_info_t* info =
      iotjs_get_object_native_info(jval_arrbuf, sizeof(iotjs_object_info_t));
  info->env = env;
  info->native_object = external_data;
  info->finalize_hint = finalize_hint;
  info->finalize_cb = finalize_cb;

  NAPI_ASSIGN(result, AS_NAPI_VALUE(jval_arrbuf));
  NAPI_RETURN(napi_ok);
}

napi_status napi_create_typedarray(napi_env env, napi_typedarray_type type,
                                   size_t length, napi_value arraybuffer,
                                   size_t byte_offset, napi_value* result) {
  NAPI_TRY_ENV(env);

  if (!jerry_is_feature_enabled(JERRY_FEATURE_TYPEDARRAY)) {
    NAPI_ASSIGN(result, NULL);
    NAPI_RETURN_WITH_MSG(napi_generic_failure, napi_err_no_typedarray);
  }

  jerry_typedarray_type_t jtype;
#define CASE_NAPI_TYPEDARRAY_TYPE(type_name, jtype_name) \
  case napi_##type_name##_array:                         \
    jtype = JERRY_TYPEDARRAY_##jtype_name;               \
    break;

  switch (type) {
    CASE_NAPI_TYPEDARRAY_TYPE(int8, INT8);
    CASE_NAPI_TYPEDARRAY_TYPE(uint8, UINT8);
    CASE_NAPI_TYPEDARRAY_TYPE(uint8_clamped, UINT8CLAMPED);
    CASE_NAPI_TYPEDARRAY_TYPE(int16, INT16);
    CASE_NAPI_TYPEDARRAY_TYPE(uint16, UINT16);
    CASE_NAPI_TYPEDARRAY_TYPE(int32, INT32);
    CASE_NAPI_TYPEDARRAY_TYPE(uint32, UINT32);
    CASE_NAPI_TYPEDARRAY_TYPE(float32, FLOAT32);
    CASE_NAPI_TYPEDARRAY_TYPE(float64, FLOAT64);
    default:
      jtype = JERRY_TYPEDARRAY_INVALID;
  }
#undef CASE_NAPI_TYPEDARRAY_TYPE

  jerry_value_t jval_arraybuffer = AS_JERRY_VALUE(arraybuffer);
  JERRYX_CREATE(jval,
                jerry_create_typedarray_for_arraybuffer_sz(jtype,
                                                           jval_arraybuffer,
                                                           byte_offset,
                                                           length));
  NAPI_ASSIGN(result, AS_NAPI_VALUE(jval));
  NAPI_RETURN(napi_ok);
}

napi_status napi_create_buffer(napi_env env, size_t size, void** data,
                               napi_value* result) {
  NAPI_TRY_ENV(env);
  JERRYX_CREATE(jval_buf, iotjs_bufferwrap_create_buffer(size));
  iotjs_bufferwrap_t* buf_wrap = iotjs_bufferwrap_from_jbuffer(jval_buf);

  NAPI_ASSIGN(data, buf_wrap->buffer);

  return napi_assign_nvalue(jval_buf, result);
}

napi_status napi_create_buffer_copy(napi_env env, size_t size, const void* data,
                                    void** result_data, napi_value* result) {
  NAPI_TRY_ENV(env);
  JERRYX_CREATE(jval_buf, iotjs_bufferwrap_create_buffer(size));
  iotjs_bufferwrap_t* buf_wrap = iotjs_bufferwrap_from_jbuffer(jval_buf);

  iotjs_bufferwrap_copy(buf_wrap, (char*)data, size);

  NAPI_ASSIGN(result_data, buf_wrap->buffer);

  return napi_assign_nvalue(jval_buf, result);
}

napi_status napi_create_dataview(napi_env env, size_t byte_length,
                                 napi_value arraybuffer, size_t byte_offset,
                                 napi_value* result) {
  NAPI_TRY_ENV(env);

  if (!jerry_is_feature_enabled(JERRY_FEATURE_DATAVIEW)) {
    NAPI_ASSIGN(result, NULL);
    NAPI_RETURN_WITH_MSG(napi_generic_failure, napi_err_no_dataview);
  }

  NAPI_WEAK_ASSERT_WITH_MSG(napi_invalid_arg, byte_length != 0,
                            "External data byte length could not be 0.");
  jerry_value_t jval_arraybuffer = AS_JERRY_VALUE(arraybuffer);

  NAPI_WEAK_ASSERT_WITH_MSG(napi_invalid_arg,
                            jerry_value_is_arraybuffer(jval_arraybuffer),
                            "Argument must be a valid ArrayBuffer object.");

  JERRYX_CREATE(jval_dv, jerry_create_dataview(jval_arraybuffer, byte_offset,
                                               byte_length));
  NAPI_ASSIGN(result, AS_NAPI_VALUE(jval_dv));
  NAPI_RETURN(napi_ok);
}

static void napi_external_destroy(iotjs_object_info_t* info) {
  if (info->finalize_cb != NULL) {
    info->finalize_cb(info->env, info->native_object, info->finalize_hint);
  }

  IOTJS_RELEASE(info);
}

static const jerry_object_native_info_t napi_external_native_info = {
  .free_cb = (jerry_object_native_free_callback_t)napi_external_destroy
};

napi_status napi_create_external(napi_env env, void* data,
                                 napi_finalize finalize_cb, void* finalize_hint,
                                 napi_value* result) {
  NAPI_TRY_ENV(env);
  napi_value nval;
  NAPI_INTERNAL_CALL(napi_create_object(env, &nval));
  iotjs_object_info_t* info =
      (iotjs_object_info_t*)iotjs_buffer_allocate(sizeof(iotjs_object_info_t));
  info->native_object = data;
  info->finalize_cb = finalize_cb;
  info->finalize_hint = finalize_hint;

  jerry_set_object_native_pointer(AS_JERRY_VALUE(nval), info,
                                  &napi_external_native_info);

  NAPI_ASSIGN(result, nval);
  NAPI_RETURN(napi_ok);
}

napi_status napi_create_external_buffer(napi_env env, size_t length, void* data,
                                        napi_finalize finalize_cb,
                                        void* finalize_hint,
                                        napi_value* result) {
  NAPI_TRY_ENV(env);
  char* nval = NULL;
  napi_value res;
  NAPI_INTERNAL_CALL(
      napi_create_buffer_copy(env, length, data, (void**)&nval, &res));

  jerry_value_t jbuffer = AS_JERRY_VALUE(res);
  iotjs_bufferwrap_t* bufferwrap = iotjs_bufferwrap_from_jbuffer(jbuffer);
  iotjs_buffer_external_info_t* info =
      IOTJS_ALLOC(iotjs_buffer_external_info_t);

  info->env = env;
  info->external_data = data;
  info->finalize_hint = finalize_hint;
  info->finalize_cb = finalize_cb;

  iotjs_bufferwrap_set_external_callback(bufferwrap,
                                         iotjs_napi_buffer_external_free_cb,
                                         info);

  NAPI_ASSIGN(result, res);
  NAPI_RETURN(napi_ok);
}

napi_status napi_create_object(napi_env env, napi_value* result) {
  NAPI_TRY_ENV(env);
  JERRYX_CREATE(jval, jerry_create_object());
  return napi_assign_nvalue(jval, result);
}

static napi_status napi_create_error_helper(jerry_error_t jerry_error_type,
                                            napi_env env, napi_value code,
                                            napi_value msg,
                                            napi_value* result) {
  NAPI_TRY_ENV(env);

  jerry_value_t jval_code = AS_JERRY_VALUE(code);
  jerry_value_t jval_msg = AS_JERRY_VALUE(msg);

  NAPI_TRY_TYPE(string, jval_msg);

  jerry_size_t msg_size = jerry_get_utf8_string_size(jval_msg);
  jerry_char_t* raw_msg = IOTJS_CALLOC(msg_size + 1, jerry_char_t);
  jerry_size_t written_size =
      jerry_string_to_utf8_char_buffer(jval_msg, raw_msg, msg_size);
  NAPI_WEAK_ASSERT(napi_invalid_arg, written_size == msg_size);
  raw_msg[msg_size] = '\0';

  jerry_value_t jval_error = jerry_create_error(jerry_error_type, raw_msg);

  IOTJS_RELEASE(raw_msg);

  /** code has to be an JS string type, thus it can not be an number 0 */
  if (code != NULL) {
    NAPI_TRY_TYPE(string, jval_code);
    jval_error = jerry_get_value_from_error(jval_error, true);
    iotjs_jval_set_property_jval(jval_error, IOTJS_MAGIC_STRING_CODE,
                                 jval_code);
  }

  jerryx_create_handle(jval_error);
  NAPI_ASSIGN(result, AS_NAPI_VALUE(jval_error));

  NAPI_RETURN(napi_ok);
}

napi_status napi_create_error(napi_env env, napi_value code, napi_value msg,
                              napi_value* result) {
  return napi_create_error_helper(JERRY_ERROR_COMMON, env, code, msg, result);
}

napi_status napi_create_range_error(napi_env env, napi_value code,
                                    napi_value msg, napi_value* result) {
  return napi_create_error_helper(JERRY_ERROR_RANGE, env, code, msg, result);
}

napi_status napi_create_type_error(napi_env env, napi_value code,
                                   napi_value msg, napi_value* result) {
  return napi_create_error_helper(JERRY_ERROR_TYPE, env, code, msg, result);
}

static napi_status napi_number_convert_from_c_type_helper(napi_env env,
                                                          double value,
                                                          napi_value* result) {
  NAPI_TRY_ENV(env);
  JERRYX_CREATE(jval, jerry_create_number(value));
  return napi_assign_nvalue(jval, result);
}

#define DEF_NAPI_NUMBER_CONVERT_FROM_C_TYPE(type, name)                        \
  napi_status napi_create_##name(napi_env env, type value,                     \
                                 napi_value* result) {                         \
    return napi_number_convert_from_c_type_helper(env, (double)value, result); \
  }

DEF_NAPI_NUMBER_CONVERT_FROM_C_TYPE(int32_t, int32);
DEF_NAPI_NUMBER_CONVERT_FROM_C_TYPE(uint32_t, uint32);
DEF_NAPI_NUMBER_CONVERT_FROM_C_TYPE(int64_t, int64);
DEF_NAPI_NUMBER_CONVERT_FROM_C_TYPE(double, double);
#undef DEF_NAPI_NUMBER_CONVERT_FROM_C_TYPE

napi_status napi_get_value_double(napi_env env, napi_value value,
                                  double* result) {
  NAPI_TRY_ENV(env);
  jerry_value_t jval = AS_JERRY_VALUE(value);
  NAPI_TRY_TYPE(number, jval);
  NAPI_ASSIGN(result, jerry_get_number_value(jval));
  NAPI_RETURN(napi_ok);
}

#define DEF_NAPI_NUMBER_CONVERT_FROM_NVALUE(type, name)             \
  napi_status napi_get_value_##name(napi_env env, napi_value value, \
                                    type* result) {                 \
    NAPI_TRY_ENV(env);                                              \
    jerry_value_t jval = AS_JERRY_VALUE(value);                     \
    NAPI_TRY_TYPE(number, jval);                                    \
    double num = jerry_get_number_value(jval);                      \
    if (isinf(num) || isnan(num)) {                                 \
      num = 0;                                                      \
    }                                                               \
    NAPI_ASSIGN(result, num);                                       \
    NAPI_RETURN(napi_ok);                                           \
  }

DEF_NAPI_NUMBER_CONVERT_FROM_NVALUE(int32_t, int32);
DEF_NAPI_NUMBER_CONVERT_FROM_NVALUE(int64_t, int64);
DEF_NAPI_NUMBER_CONVERT_FROM_NVALUE(uint32_t, uint32);
#undef DEF_NAPI_NUMBER_CONVERT_FROM_NVALUE

napi_status napi_create_symbol(napi_env env, napi_value description,
                               napi_value* result) {
  NAPI_TRY_ENV(env);

  if (!jerry_is_feature_enabled(JERRY_FEATURE_SYMBOL)) {
    NAPI_ASSIGN(result, NULL);
    NAPI_RETURN_WITH_MSG(napi_generic_failure, napi_err_no_symbol);
  }

  JERRYX_CREATE(jval, jerry_create_symbol(AS_JERRY_VALUE(description)));
  NAPI_ASSIGN(result, AS_NAPI_VALUE(jval));
  NAPI_RETURN(napi_ok);
}

napi_status napi_create_string_utf8(napi_env env, const char* str,
                                    size_t length, napi_value* result) {
  NAPI_TRY_ENV(env);
  if (length == NAPI_AUTO_LENGTH) {
    length = strlen(str);
  }
  JERRYX_CREATE(jval,
                jerry_create_string_sz_from_utf8((jerry_char_t*)str, length));
  return napi_assign_nvalue(jval, result);
}

napi_status napi_get_array_length(napi_env env, napi_value value,
                                  uint32_t* result) {
  NAPI_TRY_ENV(env);
  jerry_value_t jval = AS_JERRY_VALUE(value);
  NAPI_ASSIGN(result, jerry_get_array_length(jval));
  NAPI_RETURN(napi_ok);
}

napi_status napi_get_arraybuffer_info(napi_env env, napi_value arraybuffer,
                                      void** data, size_t* byte_length) {
  NAPI_TRY_ENV(env);

  if (!jerry_is_feature_enabled(JERRY_FEATURE_TYPEDARRAY)) {
    NAPI_ASSIGN(byte_length, 0);
    NAPI_ASSIGN(data, NULL);
    NAPI_RETURN_WITH_MSG(napi_generic_failure, napi_err_no_typedarray);
  }

  jerry_value_t jval = AS_JERRY_VALUE(arraybuffer);
  jerry_length_t len = jerry_get_arraybuffer_byte_length(jval);

  /**
   * WARNING: if the arraybuffer is managed by js engine,
   * write beyond address limit may lead to an unpredictable result.
   */
  uint8_t* ptr = jerry_get_arraybuffer_pointer(jval);

  NAPI_ASSIGN(byte_length, len);
  NAPI_ASSIGN(data, ptr);
  NAPI_RETURN(napi_ok);
}

napi_status napi_get_buffer_info(napi_env env, napi_value value, void** data,
                                 size_t* length) {
  NAPI_TRY_ENV(env);
  jerry_value_t jval = AS_JERRY_VALUE(value);
  iotjs_bufferwrap_t* buf_wrap = iotjs_bufferwrap_from_jbuffer(jval);
  NAPI_ASSIGN(data, buf_wrap->buffer);
  NAPI_ASSIGN(length, iotjs_bufferwrap_length(buf_wrap));

  NAPI_RETURN(napi_ok);
}

napi_status napi_get_prototype(napi_env env, napi_value object,
                               napi_value* result) {
  NAPI_TRY_ENV(env);
  jerry_value_t jval = AS_JERRY_VALUE(object);
  JERRYX_CREATE(jval_proto, jerry_get_prototype(jval));
  return napi_assign_nvalue(jval_proto, result);
}

napi_status napi_get_value_bool(napi_env env, napi_value value, bool* result) {
  NAPI_TRY_ENV(env);
  jerry_value_t jval = AS_JERRY_VALUE(value);
  NAPI_TRY_TYPE(boolean, jval);
  return napi_assign_bool(jerry_get_boolean_value(jval), result);
}

napi_status napi_get_boolean(napi_env env, bool value, napi_value* result) {
  NAPI_TRY_ENV(env);
  JERRYX_CREATE(jval, jerry_create_boolean(value));
  return napi_assign_nvalue(jval, result);
}

napi_status napi_get_dataview_info(napi_env env, napi_value dataview,
                                   size_t* byte_length, void** data,
                                   napi_value* arraybuffer,
                                   size_t* byte_offset) {
  NAPI_TRY_ENV(env);

  if (!jerry_is_feature_enabled(JERRY_FEATURE_DATAVIEW)) {
    NAPI_ASSIGN(byte_length, 0);
    NAPI_ASSIGN(data, NULL);
    NAPI_ASSIGN(arraybuffer, AS_NAPI_VALUE(jerry_create_undefined()));
    NAPI_ASSIGN(byte_offset, 0);
    NAPI_RETURN_WITH_MSG(napi_generic_failure, napi_err_no_dataview);
  }

  jerry_value_t jval = AS_JERRY_VALUE(dataview);
  NAPI_WEAK_ASSERT_WITH_MSG(napi_invalid_arg, jerry_value_is_dataview(jval),
                            "Argument must be a valid DataView object.");

  JERRYX_CREATE(jval_arraybuffer,
                jerry_get_dataview_buffer(jval, (jerry_length_t*)byte_offset,
                                          (jerry_length_t*)byte_length));
  uint8_t* ptr = jerry_get_arraybuffer_pointer(jval_arraybuffer);

  NAPI_ASSIGN(arraybuffer, AS_NAPI_VALUE(jval_arraybuffer));
  NAPI_ASSIGN(data, ptr);
  NAPI_RETURN(napi_ok);
}

napi_status napi_get_typedarray_info(napi_env env, napi_value typedarray,
                                     napi_typedarray_type* type, size_t* length,
                                     void** data, napi_value* arraybuffer,
                                     size_t* byte_offset) {
  NAPI_TRY_ENV(env);

  if (!jerry_is_feature_enabled(JERRY_FEATURE_TYPEDARRAY)) {
    NAPI_ASSIGN(type, napi_int8_array);
    NAPI_ASSIGN(length, 0);
    NAPI_ASSIGN(data, NULL);
    NAPI_ASSIGN(arraybuffer, AS_NAPI_VALUE(jerry_create_undefined()));
    NAPI_ASSIGN(byte_offset, 0);
    NAPI_RETURN_WITH_MSG(napi_generic_failure, napi_err_no_typedarray);
  }

  jerry_value_t jval = AS_JERRY_VALUE(typedarray);
  jerry_typedarray_type_t jtype = jerry_get_typedarray_type(jval);

  napi_typedarray_type ntype;
#define CASE_JERRY_TYPEDARRAY_TYPE(jtype_name, type_name) \
  case JERRY_TYPEDARRAY_##jtype_name: {                   \
    ntype = napi_##type_name##_array;                     \
    break;                                                \
  }

  switch (jtype) {
    CASE_JERRY_TYPEDARRAY_TYPE(INT8, int8);
    CASE_JERRY_TYPEDARRAY_TYPE(UINT8, uint8);
    CASE_JERRY_TYPEDARRAY_TYPE(UINT8CLAMPED, uint8_clamped);
    CASE_JERRY_TYPEDARRAY_TYPE(INT16, int16);
    CASE_JERRY_TYPEDARRAY_TYPE(UINT16, uint16);
    CASE_JERRY_TYPEDARRAY_TYPE(INT32, int32);
    CASE_JERRY_TYPEDARRAY_TYPE(UINT32, uint32);
    CASE_JERRY_TYPEDARRAY_TYPE(FLOAT32, float32);
    default: {
      IOTJS_ASSERT(jtype == JERRY_TYPEDARRAY_FLOAT64);
      ntype = napi_float64_array;
      break;
    }
  }
#undef CASE_JERRY_TYPEDARRAY_TYPE

  jerry_length_t jlength = jerry_get_typedarray_length(jval);
  jerry_length_t jbyte_offset;
  jerry_length_t jbyte_length;
  JERRYX_CREATE(jval_arraybuffer,
                jerry_get_typedarray_buffer(jval, &jbyte_offset,
                                            &jbyte_length));

  /**
   * WARNING: if the arraybuffer is managed by js engine,
   * write beyond address limit may lead to an unpredictable result.
   */
  uint8_t* ptr = jerry_get_arraybuffer_pointer(jval);

  NAPI_ASSIGN(type, ntype);
  NAPI_ASSIGN(length, jlength);
  NAPI_ASSIGN(data, ptr);
  NAPI_ASSIGN(arraybuffer, AS_NAPI_VALUE(jval_arraybuffer));
  NAPI_ASSIGN(byte_offset, jbyte_offset);
  NAPI_RETURN(napi_ok);
}

napi_status napi_get_value_external(napi_env env, napi_value value,
                                    void** result) {
  NAPI_TRY_ENV(env);
  jerry_value_t jval = AS_JERRY_VALUE(value);
  iotjs_object_info_t* info = NULL;
  if (!jerry_get_object_native_pointer(jval, (void**)&info,
                                       &napi_external_native_info)) {
    NAPI_ASSIGN(result, NULL);
    NAPI_RETURN_WITH_MSG(napi_invalid_arg,
                         "Argument must be type of 'napi_external'.");
  }

  NAPI_ASSIGN(result, info->native_object);
  NAPI_RETURN(napi_ok);
}

napi_status napi_get_value_string_utf8(napi_env env, napi_value value,
                                       char* buf, size_t bufsize,
                                       size_t* result) {
  NAPI_TRY_ENV(env);
  jerry_value_t jval = AS_JERRY_VALUE(value);
  NAPI_TRY_TYPE(string, jval);

  size_t str_size = jerry_get_utf8_string_size(jval);
  if (buf == NULL) {
    /* null terminator is excluded */
    NAPI_ASSIGN(result, str_size);
    NAPI_RETURN(napi_ok);
  }

  jerry_size_t written_size =
      jerry_string_to_utf8_char_buffer(jval, (jerry_char_t*)buf, bufsize);
  NAPI_WEAK_ASSERT_WITH_MSG(napi_generic_failure,
                            str_size == 0 || (bufsize > 0 && written_size != 0),
                            "Insufficient buffer not supported yet.");
  /* expects one more byte to write null terminator  */
  if (bufsize > written_size) {
    buf[written_size] = '\0';
  }
  NAPI_ASSIGN(result, written_size);
  NAPI_RETURN(napi_ok);
}

napi_status napi_get_global(napi_env env, napi_value* result) {
  NAPI_TRY_ENV(env);
  JERRYX_CREATE(jval, jerry_get_global_object());
  return napi_assign_nvalue(jval, result);
}

napi_status napi_get_null(napi_env env, napi_value* result) {
  NAPI_TRY_ENV(env);
  JERRYX_CREATE(jval, jerry_create_null());
  return napi_assign_nvalue(jval, result);
}

napi_status napi_get_undefined(napi_env env, napi_value* result) {
  NAPI_TRY_ENV(env);
  JERRYX_CREATE(jval, jerry_create_undefined());
  return napi_assign_nvalue(jval, result);
}

napi_status napi_coerce_to_bool(napi_env env, napi_value value,
                                napi_value* result) {
  NAPI_TRY_ENV(env);
  jerry_value_t jval = AS_JERRY_VALUE(value);
  bool res = jerry_value_to_boolean(jval);
  JERRYX_CREATE(jval_result, jerry_create_boolean(res));
  NAPI_ASSIGN(result, AS_NAPI_VALUE(jval_result));
  NAPI_RETURN(napi_ok);
}

#define DEF_NAPI_COERCE_TO(type, alias)                             \
  napi_status napi_coerce_to_##type(napi_env env, napi_value value, \
                                    napi_value* result) {           \
    NAPI_TRY_ENV(env);                                              \
    jerry_value_t jval = AS_JERRY_VALUE(value);                     \
    JERRYX_CREATE(jval_result, jerry_value_to_##alias(jval));       \
    return napi_assign_nvalue(jval_result, result);                 \
  }

DEF_NAPI_COERCE_TO(number, number);
DEF_NAPI_COERCE_TO(object, object);
DEF_NAPI_COERCE_TO(string, string);

napi_status napi_typeof(napi_env env, napi_value value,
                        napi_valuetype* result) {
  NAPI_TRY_ENV(env);
  jerry_value_t jval = AS_JERRY_VALUE(value);
  jerry_type_t type = jerry_value_get_type(jval);

  switch (type) {
    case JERRY_TYPE_UNDEFINED: {
      NAPI_ASSIGN(result, napi_undefined);
      break;
    }
    case JERRY_TYPE_NULL: {
      NAPI_ASSIGN(result, napi_null);
      break;
    }
    case JERRY_TYPE_BOOLEAN: {
      NAPI_ASSIGN(result, napi_boolean);
      break;
    }
    case JERRY_TYPE_NUMBER: {
      NAPI_ASSIGN(result, napi_number);
      break;
    }
    case JERRY_TYPE_STRING: {
      NAPI_ASSIGN(result, napi_string);
      break;
    }
    case JERRY_TYPE_SYMBOL: {
      NAPI_ASSIGN(result, napi_symbol);
      break;
    }
    case JERRY_TYPE_OBJECT: {
      if (jerry_get_object_native_pointer(jval, NULL,
                                          &napi_external_native_info)) {
        NAPI_ASSIGN(result, napi_external);
      } else {
        NAPI_ASSIGN(result, napi_object);
      }
      break;
    }
    case JERRY_TYPE_FUNCTION: {
      NAPI_ASSIGN(result, napi_function);
      break;
    }
    default:
      NAPI_RETURN(napi_invalid_arg);
  }

  NAPI_RETURN(napi_ok);
}

#define DEF_NAPI_VALUE_IS(type)                                              \
  napi_status napi_is_##type(napi_env env, napi_value value, bool* result) { \
    NAPI_TRY_ENV(env);                                                       \
    return napi_assign_bool(jerry_value_is_##type(AS_JERRY_VALUE(value)),    \
                            result);                                         \
  }

DEF_NAPI_VALUE_IS(array);
DEF_NAPI_VALUE_IS(arraybuffer);
DEF_NAPI_VALUE_IS(dataview);
DEF_NAPI_VALUE_IS(typedarray);

napi_status napi_is_buffer(napi_env env, napi_value value, bool* result) {
  NAPI_TRY_ENV(env);
  jerry_value_t jval_global = jerry_get_global_object();
  jerry_value_t jval_buffer =
      iotjs_jval_get_property(jval_global, IOTJS_MAGIC_STRING_BUFFER);

  napi_status status =
      napi_instanceof(env, value, AS_NAPI_VALUE(jval_buffer), result);

  jerry_release_value(jval_buffer);
  jerry_release_value(jval_global);

  return status;
}

napi_status napi_is_error(napi_env env, napi_value value, bool* result) {
  NAPI_TRY_ENV(env);
  jerry_value_t jval_global = jerry_get_global_object();
  jerry_value_t jval_error =
      iotjs_jval_get_property(jval_global, IOTJS_MAGIC_STRING_ERROR);

  napi_status status =
      napi_instanceof(env, value, AS_NAPI_VALUE(jval_error), result);

  jerry_release_value(jval_error);
  jerry_release_value(jval_global);

  return status;
}

napi_status napi_instanceof(napi_env env, napi_value object,
                            napi_value constructor, bool* result) {
  NAPI_TRY_ENV(env);
  jerry_value_t jval_object = AS_JERRY_VALUE(object);
  jerry_value_t jval_cons = AS_JERRY_VALUE(constructor);

  jerry_value_t is_instance =
      jerry_binary_operation(JERRY_BIN_OP_INSTANCEOF, jval_object, jval_cons);
  if (jerry_value_is_error(is_instance)) {
    jerry_release_value(is_instance);
    NAPI_ASSIGN(result, false);
  } else {
    NAPI_ASSIGN(result, jerry_get_boolean_value(is_instance));
  }

  NAPI_RETURN(napi_ok);
}

napi_status napi_strict_equals(napi_env env, napi_value lhs, napi_value rhs,
                               bool* result) {
  NAPI_TRY_ENV(env);
  jerry_value_t jval_lhs = AS_JERRY_VALUE(lhs);
  jerry_value_t jval_rhs = AS_JERRY_VALUE(rhs);

  jerry_value_t is_equal =
      jerry_binary_operation(JERRY_BIN_OP_STRICT_EQUAL, jval_lhs, jval_rhs);
  if (jerry_value_is_error(is_equal)) {
    jerry_release_value(is_equal);
    NAPI_RETURN(napi_generic_failure);
  }

  return napi_assign_bool(jerry_get_boolean_value(is_equal), result);
}

napi_status napi_create_promise(napi_env env, napi_deferred* deferred,
                                napi_value* promise) {
  NAPI_TRY_ENV(env);
  if (!jerry_is_feature_enabled(JERRY_FEATURE_PROMISE)) {
    NAPI_ASSIGN(promise, NULL);
    NAPI_RETURN_WITH_MSG(napi_generic_failure, napi_err_no_promise);
  }

  if (deferred == NULL) {
    NAPI_ASSIGN(promise, NULL);
    NAPI_RETURN_WITH_MSG(napi_generic_failure, napi_err_invalid_deferred);
  }

  jerry_value_t jpromise = jerry_create_promise();
  napi_assign_nvalue(jpromise, promise);
  *deferred = malloc(sizeof(napi_value*));
  memcpy(*deferred, promise, sizeof(napi_value*));
  NAPI_RETURN(napi_ok);
}

napi_status napi_resolve_deferred(napi_env env, napi_deferred deferred,
                                  napi_value resolution) {
  NAPI_TRY_ENV(env);
  if (!jerry_is_feature_enabled(JERRY_FEATURE_PROMISE)) {
    NAPI_RETURN_WITH_MSG(napi_generic_failure, napi_err_no_promise);
  }

  if (deferred == NULL) {
    NAPI_RETURN_WITH_MSG(napi_generic_failure, napi_err_invalid_deferred);
  }

  jerry_value_t promise = AS_JERRY_VALUE(*((napi_value*)deferred));
  jerry_value_t res =
      jerry_resolve_or_reject_promise(promise, AS_JERRY_VALUE(resolution),
                                      true);
  jerry_release_value(promise);
  free(deferred);
  if (jerry_value_is_error(res)) {
    NAPI_INTERNAL_CALL(napi_throw(env, AS_NAPI_VALUE(res)));
    NAPI_RETURN(napi_pending_exception);
  }
  NAPI_RETURN(napi_ok);
}

napi_status napi_reject_deferred(napi_env env, napi_deferred deferred,
                                 napi_value rejection) {
  NAPI_TRY_ENV(env);
  if (!jerry_is_feature_enabled(JERRY_FEATURE_PROMISE)) {
    NAPI_RETURN_WITH_MSG(napi_generic_failure, napi_err_no_promise);
  }

  if (deferred == NULL) {
    NAPI_RETURN_WITH_MSG(napi_generic_failure, napi_err_invalid_deferred);
  }

  jerry_value_t promise = AS_JERRY_VALUE(*((napi_value*)deferred));
  jerry_value_t res =
      jerry_resolve_or_reject_promise(promise, AS_JERRY_VALUE(rejection),
                                      false);
  jerry_release_value(promise);
  free(deferred);
  if (jerry_value_is_error(res)) {
    NAPI_INTERNAL_CALL(napi_throw(env, AS_NAPI_VALUE(res)));
    NAPI_RETURN(napi_pending_exception);
  }

  NAPI_RETURN(napi_ok);
}

napi_status napi_is_promise(napi_env env, napi_value promise,
                            bool* is_promise) {
  NAPI_TRY_ENV(env);
  if (!jerry_is_feature_enabled(JERRY_FEATURE_PROMISE)) {
    NAPI_RETURN_WITH_MSG(napi_generic_failure, napi_err_no_promise);
  }

  *is_promise = jerry_value_is_promise(AS_JERRY_VALUE(promise));
  NAPI_RETURN(napi_ok);
}
