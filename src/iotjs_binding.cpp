/* Copyright 2015-2016 Samsung Electronics Co., Ltd.
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


namespace iotjs {

#define JVAL_IS_NULL(val_p) \
    jerry_value_is_null(*val_p)

#define JVAL_IS_UNDEFINED(val_p) \
    jerry_value_is_undefined(*val_p)

#define JVAL_IS_BOOLEAN(val_p) \
    jerry_value_is_boolean(*val_p)

#define JVAL_IS_NUMBER(val_p) \
    jerry_value_is_number(*val_p)

#define JVAL_IS_STRING(val_p) \
    jerry_value_is_string(*val_p)

#define JVAL_IS_OBJECT(val_p) \
    jerry_value_is_object(*val_p)

#define JVAL_IS_FUNCTION(val_p) \
    jerry_value_is_function(*val_p)

#define JVAL_IS_ARRAY(val_p) \
    jerry_value_is_array(*val_p)

#define JVAL_TO_BOOLEAN(val_p) \
    jerry_get_boolean_value(*val_p)

#define JVAL_TO_NUMBER(val_p) \
    jerry_get_number_value(*val_p)


JObject* JObject::_null = nullptr;
JObject* JObject::_undefined = nullptr;


JObject::JObject() {
  _obj_val = jerry_create_object();
  _unref_at_close = true;
}


JObject::JObject(const JObject& other) {
  _obj_val = other._obj_val;
  _unref_at_close = true;
  Ref();
}


JObject::JObject(bool v) {
  _obj_val = iotjs_jval_bool(v);
  _unref_at_close = false;
}


JObject::JObject(int v) {
  _obj_val = iotjs_jval_number(v);
  _unref_at_close = true;
}


JObject::JObject(double v) {
  _obj_val = iotjs_jval_number(v);
  _unref_at_close = true;
}


JObject::JObject(const char* v) {
  IOTJS_ASSERT(v != NULL);
  _obj_val = iotjs_jval_raw_string(v);
  _unref_at_close = true;
}


JObject::JObject(const iotjs_string_t& v) {
  _obj_val = iotjs_jval_string(&v);
  _unref_at_close = true;
}


JObject::JObject(uint32_t len, const char* data) {
  _obj_val = jerry_create_array(len);

  if (data != NULL) {
    for (uint32_t i = 0; i < len; i++) {
      jerry_value_t val = jerry_create_number((double)data[i]);
      jerry_set_property_by_index(_obj_val, i, val);
      jerry_release_value(val);
    }
  }

  _unref_at_close = true;
}


JObject::JObject(const JRawValueType val, bool need_unref) {
  _obj_val = val;
  _unref_at_close = need_unref;
}


JObject::JObject(JHandlerType handler) {
  _obj_val = jerry_create_external_function(handler);
  IOTJS_ASSERT(jerry_value_is_constructor(_obj_val));
  _unref_at_close = true;
}


JObject::~JObject() {
  if (_unref_at_close) {
    Unref();
  }
}


void JObject::init() {
  _null = new JObject(iotjs_jval_null(), false);
  _undefined = new JObject(iotjs_jval_undefined(), false);
}


void JObject::cleanup() {
  delete _null;
  delete _undefined;
}


JObject& JObject::Null() {
  return *_null;
}


JObject& JObject::Undefined() {
  return *_undefined;
}


JObject JObject::Global() {
  return JObject((JRawValueType) jerry_get_global_object(), true);
}


JObject CreateError(const char* message, jerry_error_t error) {
  JRawValueType error_value = jerry_create_error(error,
      reinterpret_cast<const jerry_char_t*>(message));
  jerry_value_clear_error_flag(&error_value);
  return JObject(error_value);
}


JObject JObject::Error(const char* message) {
  return CreateError(message, JERRY_ERROR_COMMON);
}


JObject JObject::EvalError(const char* message) {
  return CreateError(message, JERRY_ERROR_EVAL);
}


JObject JObject::RangeError(const char* message) {
  return CreateError(message, JERRY_ERROR_RANGE);
}


JObject JObject::ReferenceError(const char* message) {
  return CreateError(message, JERRY_ERROR_REFERENCE);
}


JObject JObject::SyntaxError(const char* message) {
  return CreateError(message, JERRY_ERROR_SYNTAX);
}


JObject JObject::TypeError(const char* message) {
  return CreateError(message, JERRY_ERROR_TYPE);
}


JObject JObject::URIError(const char* message) {
  return CreateError(message, JERRY_ERROR_URI);
}


JResult JObject::Eval(const iotjs_string_t& source,
                      bool strict_mode) {
  JRawValueType res = jerry_eval(
      reinterpret_cast<const jerry_char_t*>(iotjs_string_data(&source)),
      iotjs_string_size(&source),
      strict_mode);

  JResultType type = jerry_value_has_error_flag(res)
      ? JRESULT_EXCEPTION
      : JRESULT_OK;

  jerry_value_clear_error_flag(&res);

  return JResult(res, type);
}


void JObject::SetMethod(const char* name, JHandlerType handler) {
  IOTJS_ASSERT(IsObject());
  JObject method(jerry_create_external_function(handler));
  SetProperty(name, method);
}


void JObject::SetProperty(const char* name, const JObject& val) {
  IOTJS_ASSERT(IsObject());
  SetProperty(name, val.raw_value());
}


void JObject::SetProperty(const char* name, JRawValueType val) {
  IOTJS_ASSERT(IsObject());
  JRawValueType prop_name = jerry_create_string(
      reinterpret_cast<const jerry_char_t*>(name));
  JRawValueType ret_val = jerry_set_property(
      _obj_val,
      prop_name,
      val);
  jerry_release_value(prop_name);
  IOTJS_ASSERT(!jerry_value_has_error_flag(ret_val));
  jerry_release_value(ret_val);
}


void JObject::SetPropertyByIdx(uint32_t idx, const JObject& obj) {
  IOTJS_ASSERT(IsObject());
  SetPropertyByIdx(idx, obj.raw_value());
}


void JObject::SetPropertyByIdx(uint32_t idx, JRawValueType val) {
  IOTJS_ASSERT(IsObject());
  JRawValueType ret_val = jerry_set_property_by_index(
      _obj_val,
      idx,
      val);
  IOTJS_ASSERT(!jerry_value_has_error_flag(ret_val));
  jerry_release_value(ret_val);
}


JObject JObject::GetProperty(const char* name) {
  IOTJS_ASSERT(IsObject());
  JRawValueType prop_name = jerry_create_string(
      reinterpret_cast<const jerry_char_t*>(name));
  JRawValueType res = jerry_get_property(_obj_val, prop_name);
  jerry_release_value(prop_name);

  if (jerry_value_has_error_flag (res)) {
    jerry_release_value (res);
    return JObject::Undefined();
  }

  return JObject(res);
}


void JObject::Ref() {
  jerry_acquire_value(_obj_val);
}


void JObject::Unref() {
  jerry_release_value (_obj_val);
}


bool JObject::IsNull() {
  return JVAL_IS_NULL(&_obj_val);
}


bool JObject::IsUndefined() {
  return JVAL_IS_UNDEFINED(&_obj_val);
}


bool JObject::IsBoolean() {
  return JVAL_IS_BOOLEAN(&_obj_val);
}


bool JObject::IsNumber() {
  return JVAL_IS_NUMBER(&_obj_val);
}


bool JObject::IsString() {
  return JVAL_IS_STRING(&_obj_val);
}


bool JObject::IsObject() {
  return JVAL_IS_OBJECT(&_obj_val);
}


bool JObject::IsFunction() {
  return JVAL_IS_FUNCTION(&_obj_val);
}


bool JObject::IsArray() {
  return JVAL_IS_ARRAY(&_obj_val);
}


void JObject::SetNative(uintptr_t ptr, JFreeHandlerType free_handler) {
  IOTJS_ASSERT(IsObject());
  jerry_set_object_native_handle(_obj_val, ptr, free_handler);
}


uintptr_t JObject::GetNative() {
  IOTJS_ASSERT(IsObject());
  uintptr_t ptr;
  jerry_get_object_native_handle(_obj_val, &ptr);
  return ptr;
}


JResult JObject::Call(JObject& this_, iotjs_jargs_t& arg) {
  IOTJS_ASSERT(IsFunction());

  JRawValueType res;
  JRawValueType* val_args = NULL;
  uint16_t val_argv = 0;

  if (iotjs_jargs_length(&arg) > 0) {
    val_argv = iotjs_jargs_length(&arg);
    val_args = new JRawValueType[val_argv];
    for (int i = 0; i < val_argv; ++i) {
      val_args[i] = iotjs_jargs_get(&arg, i)->raw_value();
    }
  }

  res = jerry_call_function(_obj_val, this_.raw_value(), val_args, val_argv);

  if (val_args) {
    delete [] val_args;
  }

  JResultType type = jerry_value_has_error_flag(res)
      ? JRESULT_EXCEPTION
      : JRESULT_OK;

  jerry_value_clear_error_flag(&res);

  return JResult(res, type);
}


JObject JObject::CallOk(JObject& this_, iotjs_jargs_t& arg) {
  JResult jres = Call(this_, arg);
  IOTJS_ASSERT(jres.IsOk());
  return jres.value();
}


bool JObject::GetBoolean() {
  IOTJS_ASSERT(IsBoolean());
  return JVAL_TO_BOOLEAN(&_obj_val);
}


int32_t JObject::GetInt32() {
  return static_cast<int32_t>(GetNumber());
}


int64_t JObject::GetInt64() {
  return static_cast<int64_t>(GetNumber());
}


double JObject::GetNumber() {
  IOTJS_ASSERT(IsNumber());
  return JVAL_TO_NUMBER(&_obj_val);
}


iotjs_string_t JObject::GetString() {
  IOTJS_ASSERT(IsString());

  jerry_size_t size = jerry_get_string_size(_obj_val);

  if (size == 0)
    return iotjs_string_create("");

  char* buffer = iotjs_buffer_allocate(size + 1);
  jerry_char_t* jerry_buffer = reinterpret_cast<jerry_char_t*>(buffer);

  size_t check = jerry_string_to_char_buffer(_obj_val, jerry_buffer, size);

  IOTJS_ASSERT(check == size);
  buffer[size] = '\0';

  iotjs_string_t res = iotjs_string_create_with_buffer(buffer, size);

  return res;
}


#ifdef ENABLE_SNAPSHOT
JResult JObject::ExecSnapshot(const void *snapshot_p,
                              size_t snapshot_size) {
  JRawValueType res = jerry_exec_snapshot(
      snapshot_p,
      snapshot_size,
      false); /* the snapshot buffer can be referenced
               * until jerry_cleanup is not called */

  JResultType type = jerry_value_has_error_flag(res)
      ? JRESULT_EXCEPTION
      : JRESULT_OK;

  jerry_value_clear_error_flag(&res);

  return JResult(res, type);
}
#endif


JResult::JResult(const JObject& value, JResultType type)
    : _value(value)
    , _type(type) {
}


JResult::JResult(const JRawValueType raw_val, JResultType type)
    : _value(raw_val)
    , _type(type) {
}


JResult::JResult(const JResult& other)
    : _value(other._value)
    , _type(other._type) {
}


JObject& JResult::value() {
  return _value;
}


JResultType JResult::type() const {
  IOTJS_ASSERT(_type == JRESULT_OK || _type == JRESULT_EXCEPTION);
  return _type;
}


bool JResult::IsOk() const {
  return type() == JRESULT_OK;
}


bool JResult::IsException() const {
  return type() == JRESULT_EXCEPTION;
}


} // namespace iotjs


#ifdef __cplusplus
extern "C" {
#endif


static JRawValueType jundefined;
static JRawValueType jnull;
static JRawValueType jtrue;
static JRawValueType jfalse;


void iotjs_binding_initialize() {
  jundefined = jerry_create_undefined();
  jnull = jerry_create_null();
  jtrue = jerry_create_boolean(true);
  jfalse = jerry_create_boolean(false);
}


void iotjs_binding_finalize() {
  /* simple values do not have to be released */
}


JRawValueType iotjs_jval_undefined() {
  return jundefined;
}


JRawValueType iotjs_jval_null() {
  return jnull;
}


JRawValueType iotjs_jval_bool(bool v) {
  return v ? jtrue : jfalse;
}


JRawValueType iotjs_jval_number(double v) {
  return jerry_create_number(v);
}


JRawValueType iotjs_jval_string(const iotjs_string_t* v) {
  return jerry_create_string_sz((const jerry_char_t*)(iotjs_string_data(v)),
                                iotjs_string_size(v));

}


JRawValueType iotjs_jval_raw_string(const char* v) {
  IOTJS_ASSERT(v != NULL);
  return jerry_create_string((const jerry_char_t*)(v));
}



JRawValueType iotjs_jval_object() {
  return jerry_create_object();
}


iotjs_jargs_t iotjs_jargs_create(uint16_t capacity) {
  iotjs_jargs_t jargs;
  IOTJS_VALIDATED_STRUCT_CONSTRUCTOR(iotjs_jargs_t, &jargs);

  _this->capacity = capacity;
  _this->argc = 0;
  if (capacity > 0) {
    unsigned buffer_size = sizeof(void*) * capacity;
    _this->argv = (iotjs::JObject**) iotjs_buffer_allocate(buffer_size);
  } else {
    _this->argv = NULL;
  }

  return jargs;
}


void iotjs_jargs_destroy(iotjs_jargs_t* jargs) {
  IOTJS_VALIDATED_STRUCT_DESTRUCTOR(iotjs_jargs_t, jargs);

  IOTJS_ASSERT(_this->argv == NULL || _this->argc > 0);
  IOTJS_ASSERT(_this->argc <= _this->capacity);

  if (_this->capacity > 0) {
    for (int i = 0; i < _this->argc; ++i) {
      delete _this->argv[i];
    }
    iotjs_buffer_release((char*)_this->argv);
  } else {
    IOTJS_ASSERT(_this->argv == NULL);
  }
}


iotjs_jargs_t iotjs_jargs_empty =
  IOTJS_VALIDATED_STRUCT_STATIC_INITIALIZER({ 0, 0, NULL });


uint16_t iotjs_jargs_length(iotjs_jargs_t* jargs) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_jargs_t, jargs);
  return _this->argc;
}


void iotjs_jargs_append_obj(iotjs_jargs_t* jargs, iotjs::JObject* x) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_jargs_t, jargs);

  IOTJS_ASSERT(_this->argc < _this->capacity);
  _this->argv[_this->argc++] = new iotjs::JObject(*x);
}


void iotjs_jargs_append_bool(iotjs_jargs_t* jargs, bool x) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_jargs_t, jargs);

  iotjs::JObject jtmp(x);
  iotjs_jargs_append_obj(jargs, &jtmp);
}


void iotjs_jargs_append_number(iotjs_jargs_t* jargs, double x) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_jargs_t, jargs);

  iotjs::JObject jtmp(x);
  iotjs_jargs_append_obj(jargs, &jtmp);
}


void iotjs_jargs_append_string(iotjs_jargs_t* jargs, const iotjs_string_t* x) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_jargs_t, jargs);

  iotjs::JObject jtmp(*x);
  iotjs_jargs_append_obj(jargs, &jtmp);
}


void iotjs_jargs_append_raw_string(iotjs_jargs_t* jargs, const char* x) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_jargs_t, jargs);

  iotjs::JObject jtmp(x);
  iotjs_jargs_append_obj(jargs, &jtmp);
}


void iotjs_jargs_append_undefined(iotjs_jargs_t* jargs) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_jargs_t, jargs);

  iotjs_jargs_append_obj(jargs, &iotjs::JObject::Undefined());
}


void iotjs_jargs_append_null(iotjs_jargs_t* jargs) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_jargs_t, jargs);

  iotjs_jargs_append_obj(jargs, &iotjs::JObject::Null());
}


void iotjs_jargs_replace(iotjs_jargs_t* jargs, uint16_t i, iotjs::JObject* x) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_jargs_t, jargs);

  IOTJS_ASSERT(i < _this->argc);
  IOTJS_ASSERT(_this->argv[i] != NULL);

  delete _this->argv[i];
  _this->argv[i] = new iotjs::JObject(*x);
}


iotjs::JObject* iotjs_jargs_get(iotjs_jargs_t* jargs, uint16_t i) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_jargs_t, jargs);

  IOTJS_ASSERT(i < _this->argc);
  return _this->argv[i];
}


void iotjs_jhandler_initialize(iotjs_jhandler_t* jhandler,
                               const JRawValueType func_obj_val,
                               const JRawValueType this_val,
                               JRawValueType* ret_val_p,
                               const JRawValueType args_p[],
                               const uint16_t args_cnt) {
  IOTJS_VALIDATED_STRUCT_CONSTRUCTOR(iotjs_jhandler_t, jhandler);

  _this->function = new iotjs::JObject(func_obj_val, false);
  _this->this_val = new iotjs::JObject(this_val, false);
  _this->arg_list = iotjs_jargs_create(args_cnt);
  _this->ret_val_p = ret_val_p;
#ifndef NDEBUG
  _this->finished = false;
#endif

  if (args_cnt > 0) {
    for (int i = 0; i < args_cnt; ++i) {
      iotjs::JObject arg(args_p[i], false);
      iotjs_jargs_append_obj(&_this->arg_list, &arg);
    }
  }
}


void iotjs_jhandler_destroy(iotjs_jhandler_t* jhandler) {
  IOTJS_VALIDATED_STRUCT_CONSTRUCTOR(iotjs_jhandler_t, jhandler);
  delete _this->function;
  delete _this->this_val;
  iotjs_jargs_destroy(&_this->arg_list);
}


iotjs::JObject* iotjs_jhandler_get_function(iotjs_jhandler_t* jhandler) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_jhandler_t, jhandler);
  return _this->function;
}


iotjs::JObject* iotjs_jhandler_get_this(iotjs_jhandler_t* jhandler) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_jhandler_t, jhandler);
  return _this->this_val;
}


iotjs::JObject* iotjs_jhandler_get_arg(iotjs_jhandler_t* jhandler, uint16_t i) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_jhandler_t, jhandler);
  return iotjs_jargs_get(&_this->arg_list, i);
}


uint16_t iotjs_jhandler_get_arg_length(iotjs_jhandler_t* jhandler) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_jhandler_t, jhandler);
  return iotjs_jargs_length(&_this->arg_list);
}


void iotjs_jhandler_return_obj(iotjs_jhandler_t* jhandler,
                               iotjs::JObject* ret) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_jhandler_t, jhandler);

#ifndef NDEBUG
  IOTJS_ASSERT(_this->finished == false);
#endif

  ret->Ref();
  *_this->ret_val_p = ret->raw_value();

#ifndef NDEBUG
  _this->finished = true;
#endif
}


void iotjs_jhandler_return_val(iotjs_jhandler_t* jhandler, JRawValueType ret) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_jhandler_t, jhandler);
  iotjs::JObject jret(ret);
  iotjs_jhandler_return_obj(jhandler, &jret);
}


void iotjs_jhandler_return_bool(iotjs_jhandler_t* jhandler, bool ret) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_jhandler_t, jhandler);
  iotjs::JObject jret(ret);
  iotjs_jhandler_return_obj(jhandler, &jret);
}


void iotjs_jhandler_return_number(iotjs_jhandler_t* jhandler, double ret) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_jhandler_t, jhandler);
  iotjs::JObject jret(ret);
  iotjs_jhandler_return_obj(jhandler, &jret);
}


void iotjs_jhandler_return_undefined(iotjs_jhandler_t* jhandler) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_jhandler_t, jhandler);
  iotjs::JObject jret(iotjs_jval_undefined());
  iotjs_jhandler_return_obj(jhandler, &jret);
}


void iotjs_jhandler_return_null(iotjs_jhandler_t* jhandler) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_jhandler_t, jhandler);
  iotjs::JObject jret(iotjs_jval_null());
  iotjs_jhandler_return_obj(jhandler, &jret);
}


void iotjs_jhandler_return_string(iotjs_jhandler_t* jhandler,
                                  const iotjs_string_t* ret) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_jhandler_t, jhandler);
  iotjs::JObject jret(*ret);
  iotjs_jhandler_return_obj(jhandler, &jret);
}


void iotjs_jhandler_return_raw_string(iotjs_jhandler_t* jhandler,
                                      const char* ret) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_jhandler_t, jhandler);
  iotjs::JObject jret(ret);
  iotjs_jhandler_return_obj(jhandler, &jret);
}



void iotjs_jhandler_throw_obj(iotjs_jhandler_t* jhandler, iotjs::JObject* err) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_jhandler_t, jhandler);
#ifndef NDEBUG
  IOTJS_ASSERT(_this->finished == false);
#endif

  err->Ref();
  *_this->ret_val_p = err->raw_value();
  jerry_value_set_error_flag(_this->ret_val_p);

#ifndef NDEBUG
  _this->finished = true;
#endif
}


void iotjs_jhandler_throw_val(iotjs_jhandler_t* jhandler, JRawValueType err) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_jhandler_t, jhandler);
  iotjs::JObject jerr(err);
  iotjs_jhandler_throw_obj(jhandler, &jerr);
}


#ifdef __cplusplus
} // extern "C"
#endif
