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

#include <stdlib.h>

napi_status napi_get_property_names(napi_env env, napi_value object,
                                    napi_value* result) {
  NAPI_TRY_ENV(env);
  jerry_value_t jval = AS_JERRY_VALUE(object);
  NAPI_TRY_TYPE(object, jval);

  jerry_value_t jval_keys = jerry_get_object_keys(jval);
  jerryx_create_handle(jval_keys);
  if (jerry_value_is_error(jval_keys)) {
    jerry_release_value(jval_keys);
    NAPI_RETURN(napi_invalid_arg);
  }

  return napi_assign_nvalue(jval_keys, result);
}

napi_status napi_set_property(napi_env env, napi_value object, napi_value key,
                              napi_value value) {
  NAPI_TRY_ENV(env);
  jerry_value_t jval_object = AS_JERRY_VALUE(object);
  jerry_value_t jval_key = AS_JERRY_VALUE(key);
  jerry_value_t jval_val = AS_JERRY_VALUE(value);

  NAPI_TRY_TYPE(object, jval_object);
  NAPI_TRY_TYPE(string, jval_key);

  jerry_value_t ret = jerry_set_property(jval_object, jval_key, jval_val);
  if (jerry_value_is_error(ret)) {
    jerry_release_value(ret);
    NAPI_RETURN(napi_invalid_arg);
  }

  jerry_release_value(ret);
  NAPI_RETURN(napi_ok);
}

napi_status napi_get_property(napi_env env, napi_value object, napi_value key,
                              napi_value* result) {
  NAPI_TRY_ENV(env);
  jerry_value_t jval_object = AS_JERRY_VALUE(object);
  jerry_value_t jval_key = AS_JERRY_VALUE(key);

  NAPI_TRY_TYPE(object, jval_object);
  NAPI_TRY_TYPE(string, jval_key);

  jerry_value_t jval_ret = jerry_get_property(jval_object, jval_key);
  jerryx_create_handle(jval_ret);
  if (jerry_value_is_error(jval_ret)) {
    jerry_release_value(jval_ret);
    NAPI_RETURN(napi_invalid_arg);
  }

  return napi_assign_nvalue(jval_ret, result);
}

napi_status napi_has_property(napi_env env, napi_value object, napi_value key,
                              bool* result) {
  NAPI_TRY_ENV(env);
  jerry_value_t jval_object = AS_JERRY_VALUE(object);
  jerry_value_t jval_key = AS_JERRY_VALUE(key);

  NAPI_TRY_TYPE(object, jval_object);
  NAPI_TRY_TYPE(string, jval_key);
  jerry_value_t has_prop = jerry_has_property(jval_object, jval_key);
  NAPI_TRY_TYPE(boolean, has_prop);

  return napi_assign_bool(jerry_get_boolean_value(has_prop), result);
}

napi_status napi_delete_property(napi_env env, napi_value object,
                                 napi_value key, bool* result) {
  NAPI_TRY_ENV(env);
  jerry_value_t jval_object = AS_JERRY_VALUE(object);
  jerry_value_t jval_key = AS_JERRY_VALUE(key);

  NAPI_TRY_TYPE(object, jval_object);
  NAPI_TRY_TYPE(string, jval_key);

  return napi_assign_bool(jerry_delete_property(jval_object, jval_key), result);
}

napi_status napi_has_own_property(napi_env env, napi_value object,
                                  napi_value key, bool* result) {
  NAPI_TRY_ENV(env);
  jerry_value_t jval_object = AS_JERRY_VALUE(object);
  jerry_value_t jval_key = AS_JERRY_VALUE(key);

  NAPI_TRY_TYPE(object, jval_object);
  NAPI_TRY_TYPE(string, jval_key);
  jerry_value_t has_prop = jerry_has_own_property(jval_object, jval_key);
  NAPI_TRY_TYPE(boolean, has_prop);

  return napi_assign_bool(jerry_get_boolean_value(has_prop), result);
}

napi_status napi_set_named_property(napi_env env, napi_value object,
                                    const char* utf8Name, napi_value value) {
  NAPI_TRY_ENV(env);
  jerry_value_t jval_object = AS_JERRY_VALUE(object);
  NAPI_TRY_TYPE(object, jval_object);

  jerry_value_t jval_key =
      jerry_create_string_from_utf8((jerry_char_t*)utf8Name);
  napi_status status =
      napi_set_property(env, object, AS_NAPI_VALUE(jval_key), value);
  jerry_release_value(jval_key);
  return status;
}

napi_status napi_get_named_property(napi_env env, napi_value object,
                                    const char* utf8Name, napi_value* result) {
  NAPI_TRY_ENV(env);
  jerry_value_t jval_object = AS_JERRY_VALUE(object);
  NAPI_TRY_TYPE(object, jval_object);

  jerry_value_t jval_key =
      jerry_create_string_from_utf8((jerry_char_t*)utf8Name);
  napi_status status =
      napi_get_property(env, object, AS_NAPI_VALUE(jval_key), result);
  jerry_release_value(jval_key);
  return status;
}

napi_status napi_has_named_property(napi_env env, napi_value object,
                                    const char* utf8Name, bool* result) {
  NAPI_TRY_ENV(env);
  jerry_value_t jval_object = AS_JERRY_VALUE(object);
  NAPI_TRY_TYPE(object, jval_object);

  jerry_value_t jval_key =
      jerry_create_string_from_utf8((jerry_char_t*)utf8Name);
  napi_status status =
      napi_has_property(env, object, AS_NAPI_VALUE(jval_key), result);
  jerry_release_value(jval_key);
  return status;
}

napi_status napi_set_element(napi_env env, napi_value object, uint32_t index,
                             napi_value value) {
  NAPI_TRY_ENV(env);
  jerry_value_t jval_object = AS_JERRY_VALUE(object);
  jerry_value_t jval_val = AS_JERRY_VALUE(value);

  NAPI_TRY_TYPE(object, jval_object);

  jerry_value_t res = jerry_set_property_by_index(jval_object, index, jval_val);
  if (jerry_value_is_error(res)) {
    jerry_release_value(res);
    NAPI_RETURN(napi_invalid_arg);
  }

  jerry_release_value(res);
  NAPI_RETURN(napi_ok);
}

napi_status napi_get_element(napi_env env, napi_value object, uint32_t index,
                             napi_value* result) {
  NAPI_TRY_ENV(env);
  jerry_value_t jval_object = AS_JERRY_VALUE(object);

  NAPI_TRY_TYPE(object, jval_object);

  jerry_value_t jval_ret = jerry_get_property_by_index(jval_object, index);
  jerryx_create_handle(jval_ret);
  if (jerry_value_is_error(jval_ret)) {
    jerry_release_value(jval_ret);
    NAPI_RETURN(napi_invalid_arg);
  }
  return napi_assign_nvalue(jval_ret, result);
}

napi_status napi_has_element(napi_env env, napi_value object, uint32_t index,
                             bool* result) {
  NAPI_TRY_ENV(env);
  jerry_value_t jval_object = AS_JERRY_VALUE(object);
  NAPI_TRY_TYPE(object, jval_object);

  char idx_str[17];
  sprintf(idx_str, "%d", index);
  jerry_value_t prop_name = jerry_create_string((const jerry_char_t*)idx_str);
  jerry_value_t has_prop_val = jerry_has_own_property(jval_object, prop_name);
  jerry_release_value(prop_name);

  if (jerry_value_is_error(has_prop_val)) {
    jerry_release_value(has_prop_val);
    NAPI_RETURN(napi_generic_failure);
  }

  bool has_prop = jerry_get_boolean_value(has_prop_val);
  return napi_assign_bool(has_prop, result);
}

napi_status napi_delete_element(napi_env env, napi_value object, uint32_t index,
                                bool* result) {
  NAPI_TRY_ENV(env);
  jerry_value_t jval_object = AS_JERRY_VALUE(object);
  NAPI_TRY_TYPE(object, jval_object);

  return napi_assign_bool(jerry_delete_property_by_index(jval_object, index),
                          result);
}

napi_status iotjs_napi_prop_desc_to_jdesc(napi_env env,
                                          const napi_property_descriptor* ndesc,
                                          jerry_property_descriptor_t* jdesc) {
  napi_status status;

  if (ndesc->attributes & napi_configurable) {
    jdesc->is_configurable_defined = true;
    jdesc->is_configurable = true;
  }

  if (ndesc->attributes & napi_enumerable) {
    jdesc->is_enumerable_defined = true;
    jdesc->is_enumerable = true;
  }

  if (ndesc->attributes & napi_writable) {
    jdesc->is_writable_defined = true;
    jdesc->is_writable = true;
  }

  if (ndesc->value != NULL) {
    jdesc->is_value_defined = true;
    jdesc->value = AS_JERRY_VALUE(ndesc->value);
    NAPI_RETURN(napi_ok);
  }

  if (ndesc->method != NULL) {
    napi_value method;
    status = napi_create_function(env, "method", 6, ndesc->method, ndesc->data,
                                  &method);
    if (status != napi_ok)
      return status;
    jdesc->is_value_defined = true;
    jdesc->value = AS_JERRY_VALUE(method);
    NAPI_RETURN(napi_ok);
  }

  if (ndesc->getter != NULL) {
    napi_value getter;
    status = napi_create_function(env, "getter", 6, ndesc->getter, ndesc->data,
                                  &getter);
    if (status != napi_ok)
      return status;
    jdesc->is_get_defined = true;
    jdesc->getter = AS_JERRY_VALUE(getter);
    /** jerryscript asserts xor is_writable_defined and accessors */
    jdesc->is_writable_defined = false;
  }

  if (ndesc->setter != NULL) {
    napi_value setter;
    status = napi_create_function(env, "setter", 6, ndesc->setter, ndesc->data,
                                  &setter);
    if (status != napi_ok)
      return status;
    jdesc->is_set_defined = true;
    jdesc->setter = AS_JERRY_VALUE(setter);
    /** jerryscript asserts xor is_writable_defined and accessors */
    jdesc->is_writable_defined = false;
  }

  NAPI_RETURN(napi_ok);
}

napi_status napi_define_properties(napi_env env, napi_value object,
                                   size_t property_count,
                                   const napi_property_descriptor* properties) {
  NAPI_TRY_ENV(env);
  jerry_value_t jval_target = AS_JERRY_VALUE(object);
  NAPI_TRY_TYPE(object, jval_target);
  NAPI_WEAK_ASSERT(napi_invalid_arg, properties != NULL);

  napi_status status;
  jerry_property_descriptor_t prop_desc;
  for (size_t i = 0; i < property_count; ++i) {
    jerry_init_property_descriptor_fields(&prop_desc);
    napi_property_descriptor prop = properties[i];

    jerry_value_t jval_prop_name;
    if (prop.utf8name != NULL) {
      jval_prop_name =
          jerry_create_string_from_utf8((jerry_char_t*)prop.utf8name);
      jerryx_create_handle(jval_prop_name);
    } else if (prop.name != NULL) {
      jval_prop_name = AS_JERRY_VALUE(prop.name);
      NAPI_TRY_TYPE(string, jval_prop_name);
    } else {
      NAPI_RETURN(napi_invalid_arg);
    }

    status = iotjs_napi_prop_desc_to_jdesc(env, &prop, &prop_desc);
    if (status != napi_ok)
      return status;

    jerry_value_t return_value =
        jerry_define_own_property(jval_target, jval_prop_name, &prop_desc);
    if (jerry_value_is_error(return_value)) {
      NAPI_RETURN(napi_invalid_arg);
    }
    jerry_release_value(return_value);

    /**
     * We don't have to call `jerry_free_property_descriptor_fields`
     * since most napi values are managed by handle scopes.
     */
    // jerry_free_property_descriptor_fields(&prop_desc);
  }

  NAPI_RETURN(napi_ok);
}
