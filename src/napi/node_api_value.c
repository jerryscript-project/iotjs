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
#include "internal/node_api_internal.h"
#include "modules/iotjs_module_buffer.h"

#include <math.h>

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

napi_status napi_create_external(napi_env env, void* data,
                                 napi_finalize finalize_cb, void* finalize_hint,
                                 napi_value* result) {
  NAPI_TRY_ENV(env);
  napi_value nval;
  NAPI_INTERNAL_CALL(napi_create_object(env, &nval));
  iotjs_object_info_t* info =
      iotjs_get_object_native_info(AS_JERRY_VALUE(nval),
                                   sizeof(iotjs_object_info_t));
  info->native_object = data;
  info->finalize_cb = finalize_cb;
  info->finalize_hint = finalize_hint;

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
  jerry_char_t raw_msg[msg_size + 1];
  jerry_size_t written_size =
      jerry_string_to_utf8_char_buffer(jval_msg, raw_msg, msg_size);
  NAPI_WEAK_ASSERT(napi_invalid_arg, written_size == msg_size);
  raw_msg[msg_size] = '\0';

  jerry_value_t jval_error = jerry_create_error(jerry_error_type, raw_msg);

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
    NAPI_RETURN(napi_generic_failure,
                "Symbols are not supported by this build.");
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

napi_status napi_get_value_external(napi_env env, napi_value value,
                                    void** result) {
  NAPI_TRY_ENV(env);
  jerry_value_t jval = AS_JERRY_VALUE(value);
  iotjs_object_info_t* info =
      iotjs_get_object_native_info(jval, sizeof(iotjs_object_info_t));
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
  NAPI_WEAK_ASSERT(napi_generic_failure,
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
      void* ptr = NULL;
      JNativeInfoType* out_info;
      bool has_p =
          jerry_get_object_native_pointer(jval, (void**)&ptr, &out_info);
      if (has_p && !iotjs_jbuffer_get_bufferwrap_ptr(jval)) {
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
      NAPI_RETURN(napi_invalid_arg, NULL);
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
