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

#define JVAL_IS_STRING(val_p) \
    jerry_value_is_string(*val_p)

#define JVAL_IS_OBJECT(val_p) \
    jerry_value_is_object(*val_p)

#define JVAL_IS_FUNCTION(val_p) \
    jerry_value_is_function(*val_p)

#define JVAL_IS_BOOLEAN(val_p) \
    jerry_value_is_boolean(*val_p)

#define JVAL_IS_NUMBER(val_p) \
    jerry_value_is_number(*val_p)

#define JVAL_TO_BOOLEAN(val_p) \
    jerry_get_boolean_value(*val_p)

#define JVAL_TO_NUMBER(val_p) \
    jerry_get_number_value(*val_p)


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
  _obj_val = JVal::Bool(v);
  _unref_at_close = false;
}


JObject::JObject(int v) {
  _obj_val = JVal::Number(v);
  _unref_at_close = true;
}


JObject::JObject(double v) {
  _obj_val = JVal::Number(v);
  _unref_at_close = true;
}


JObject::JObject(const char* v) {
  IOTJS_ASSERT(v != NULL);
  _obj_val = jerry_create_string(reinterpret_cast<const jerry_char_t*>(v));
  _unref_at_close = true;
}


JObject::JObject(const String& v) {
  _obj_val = jerry_create_string_sz(
      reinterpret_cast<const jerry_char_t*>(v.data()), v.size());
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


JObject& JObject::Null() {
  static JObject null(JVal::Null(), false);
  return null;
}

JObject& JObject::Undefined() {
  static JObject undefined(JVal::Undefined(), false);
  return undefined;
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


JResult JObject::Eval(const String& source,
                      bool strict_mode) {
  JRawValueType res = jerry_eval(
      reinterpret_cast<const jerry_char_t*>(source.data()),
      source.size(),
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


JResult JObject::Call(JObject& this_, JArgList& arg) {
  IOTJS_ASSERT(IsFunction());

  JRawValueType res;
  JRawValueType* val_args = NULL;
  uint16_t val_argv = 0;

  if (arg.GetLength() > 0) {
    val_argv = arg.GetLength();
    val_args = new JRawValueType[val_argv];
    for (int i = 0; i < val_argv; ++i) {
      val_args[i] = arg.Get(i)->raw_value();
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


JObject JObject::CallOk(JObject& this_, JArgList& arg) {
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


String JObject::GetString() {
  IOTJS_ASSERT(IsString());

  jerry_size_t size = jerry_get_string_size(_obj_val);

  String res(NULL, size);

  jerry_char_t* buffer = reinterpret_cast<jerry_char_t*>(res.data());

  size_t check = jerry_string_to_char_buffer(_obj_val, buffer, size);

  IOTJS_ASSERT(check == size);

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


JRawValueType JVal::Undefined() {
  return jerry_create_undefined();
}


JRawValueType JVal::Null() {
  return jerry_create_null();
}


JRawValueType JVal::Bool(bool v) {
  return jerry_create_boolean(v);
}


JRawValueType JVal::Number(int v) {
  return JVal::Number((double)v);
}


JRawValueType JVal::Number(double v) {
  return jerry_create_number(v);
}


JRawValueType JVal::Object() {
  return jerry_create_object();
}


JArgList::JArgList(uint16_t capacity)
    : _capacity(capacity)
    , _argc(0)
    , _argv(NULL) {
  if (_capacity > 0) {
    _argv = new JObject*[_capacity];
    for (int i = 0; i < _capacity; ++i) {
      _argv[i] = NULL;
    }
  }
}


JArgList::~JArgList() {
  IOTJS_ASSERT(_argv == NULL || _argc > 0);
  IOTJS_ASSERT(_argc <= _capacity);
  for (int i = 0; i < _argc; ++i) {
    delete _argv[i];
  }
  delete [] _argv;
}


static JArgList jarg_empty(0);
JArgList& JArgList::Empty() {
  return jarg_empty;
}


uint16_t JArgList::GetLength() {
  return _argc;
}


void JArgList::Add(JObject& x) {
  IOTJS_ASSERT(_argc < _capacity);
  _argv[_argc++] = new JObject(x);
}


void JArgList::Add(JRawValueType x) {
  JObject jtmp(x);
  Add(jtmp);
}


void JArgList::Set(uint16_t i, JObject& x) {
  IOTJS_ASSERT(i < _argc);
  if (_argv[i] != NULL) {
    delete _argv[i];
  }
  _argv[i] = new JObject(x);
}


void JArgList::Set(uint16_t i, JRawValueType x) {
  JObject jtmp(x);
  Set(i, jtmp);
}


JObject* JArgList::Get(uint16_t i) {
  IOTJS_ASSERT(i < _argc);
  return _argv[i];
}


JHandlerInfo::JHandlerInfo(const JRawValueType func_obj_val,
                           const JRawValueType this_val,
                           JRawValueType* ret_val_p,
                           const JRawValueType args_p[],
                           const uint16_t args_cnt)
    : _function(func_obj_val, false)
    , _this(this_val, false)
    , _arg_list(args_cnt)
    , _ret_val_p(ret_val_p)
    , _thrown(false) {
  if (args_cnt > 0) {
    for (int i = 0; i < args_cnt; ++i) {
      JObject arg(args_p[i], false);
      _arg_list.Add(arg);
    }
  }
}


JHandlerInfo::~JHandlerInfo() {
}


JObject* JHandlerInfo::GetFunction() {
  return &_function;
}


JObject* JHandlerInfo::GetThis() {
  return &_this;
}


JObject* JHandlerInfo::GetArg(uint16_t i) {
  return _arg_list.Get(i);
}


uint16_t JHandlerInfo::GetArgLength() {
  return _arg_list.GetLength();
}


void JHandlerInfo::Return(JObject& ret) {
  ret.Ref();
  *_ret_val_p = ret.raw_value();
}


void JHandlerInfo::Return(JRawValueType raw_val) {
  JObject jret(raw_val);
  Return(jret);
}


void JHandlerInfo::Throw(JObject& ret) {
  IOTJS_ASSERT(_thrown == false);

  ret.Ref();
  *_ret_val_p = ret.raw_value();
  jerry_value_set_error_flag(_ret_val_p);

  _thrown = true;
}


void JHandlerInfo::Throw(JRawValueType raw_val) {
  JObject jthrow(raw_val);
  Throw(jthrow);
}


bool JHandlerInfo::HasThrown() {
  return _thrown;
}


} // namespace iotjs
