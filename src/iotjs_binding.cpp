/* Copyright 2015 Samsung Electronics Co., Ltd.
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
    ((val_p)->type == JERRY_API_DATA_TYPE_NULL)

#define JVAL_IS_UNDEFINED(val_p) \
    ((val_p)->type == JERRY_API_DATA_TYPE_UNDEFINED)

#define JVAL_IS_STRING(val_p) \
    ((val_p)->type == JERRY_API_DATA_TYPE_STRING)

#define JVAL_IS_OBJECT(val_p) \
    ((val_p)->type == JERRY_API_DATA_TYPE_OBJECT)

#define JVAL_IS_FUNCTION(val_p) \
    (JVAL_IS_OBJECT(val_p) && jerry_api_is_function((val_p)->v_object))

#define JVAL_IS_BOOLEAN(val_p) \
    ((val_p)->type == JERRY_API_DATA_TYPE_BOOLEAN)

#define JVAL_IS_NUMBER(val_p) \
    (((val_p)->type == JERRY_API_DATA_TYPE_FLOAT32) || \
     ((val_p)->type == JERRY_API_DATA_TYPE_FLOAT64) || \
     ((val_p)->type == JERRY_API_DATA_TYPE_UINT32))

#define JVAL_TO_BOOLEAN(val_p) \
    (val_p)->v_bool

#define JVAL_TO_NUMBER(val_p) \
    ((val_p)->type == JERRY_API_DATA_TYPE_FLOAT32 ? \
       static_cast<double>((val_p)->v_float32) : \
     (val_p)->type == JERRY_API_DATA_TYPE_FLOAT64 ? \
       static_cast<double>((val_p)->v_float64) : \
     static_cast<double>((val_p)->v_uint32))



JRawObjectType* GetGlobal() {
  return jerry_api_get_global();
}



JObject::JObject() {
  JRawObjectType* new_obj = jerry_api_create_object();
  _obj_val = JVal::Object(new_obj);
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
  _unref_at_close = false;
}


JObject::JObject(double v) {
  _obj_val = JVal::Number(v);
  _unref_at_close = false;
}


JObject::JObject(const char* v) {
  IOTJS_ASSERT(v != NULL);
  _obj_val.type = JERRY_API_DATA_TYPE_STRING;
  _obj_val.v_string = jerry_api_create_string(
      reinterpret_cast<const jerry_api_char_t*>(v));
  _unref_at_close = true;
}


JObject::JObject(const String& v) {
  IOTJS_ASSERT(!v.IsEmpty());
  _obj_val.type = JERRY_API_DATA_TYPE_STRING;
  _obj_val.v_string = jerry_api_create_string(
      reinterpret_cast<const jerry_api_char_t*>(v.data()));
  _unref_at_close = true;
}


JObject::JObject(const JRawObjectType* obj, bool need_unref) {
  _obj_val = JVal::Object(obj);
  _unref_at_close = need_unref;
}


JObject::JObject(const JRawValueType* val, bool need_unref) {
  _obj_val = *val;
  _unref_at_close = need_unref;
}


JObject::JObject(JHandlerType handler) {
  _obj_val.v_object = jerry_api_create_external_function(handler);
  IOTJS_ASSERT(jerry_api_is_constructor(_obj_val.v_object));
  _obj_val.type = JERRY_API_DATA_TYPE_OBJECT;
  _unref_at_close = true;
}


JObject::~JObject() {
  if (_unref_at_close) {
    Unref();
  }
}


static JRawValueType jnull_val = JVal::Null();
static JObject jnull(&jnull_val);
JObject& JObject::Null() {
  return jnull;
}


static JRawValueType jundefined_val = JVal::Undefined();
static JObject jundefined(&jundefined_val);
JObject& JObject::Undefined() {
  return jundefined;
}


JObject JObject::Global() {
  return JObject(GetGlobal());
}


JObject CreateError(const char* message, jerry_api_error_t error) {
  return JObject(jerry_api_create_error(
      error, reinterpret_cast<const jerry_api_char_t*>(message)));
}


JObject JObject::Error(const char* message) {
  return CreateError(message, JERRY_API_ERROR_COMMON);
}


JObject JObject::Error(const String& message) {
  return CreateError(message.data(), JERRY_API_ERROR_COMMON);
}


JObject JObject::EvalError(const char* message) {
  return CreateError(message, JERRY_API_ERROR_EVAL);
}


JObject JObject::EvalError(const String& message) {
  return CreateError(message.data(), JERRY_API_ERROR_EVAL);
}


JObject JObject::RangeError(const char* message) {
  return CreateError(message, JERRY_API_ERROR_RANGE);
}


JObject JObject::RangeError(const String& message) {
  return CreateError(message.data(), JERRY_API_ERROR_RANGE);
}


JObject JObject::ReferenceError(const char* message) {
  return CreateError(message, JERRY_API_ERROR_REFERENCE);
}


JObject JObject::ReferenceError(const String& message) {
  return CreateError(message.data(), JERRY_API_ERROR_REFERENCE);
}


JObject JObject::SyntaxError(const char* message) {
  return CreateError(message, JERRY_API_ERROR_SYNTAX);
}


JObject JObject::SyntaxError(const String& message) {
  return CreateError(message.data(), JERRY_API_ERROR_SYNTAX);
}


JObject JObject::TypeError(const char* message) {
  return CreateError(message, JERRY_API_ERROR_TYPE);
}


JObject JObject::TypeError(const String& message) {
  return CreateError(message.data(), JERRY_API_ERROR_TYPE);
}


JObject JObject::URIError(const char* message) {
  return CreateError(message, JERRY_API_ERROR_URI);
}


JObject JObject::URIError(const String& message) {
  return CreateError(message.data(), JERRY_API_ERROR_URI);
}


JResult JObject::Eval(const String& source,
                      bool direct_mode,
                      bool strict_mode) {
  JRawValueType res;
  jerry_completion_code_t ret = jerry_api_eval(
      reinterpret_cast<const jerry_api_char_t*>(source.data()),
      source.size(),
      direct_mode,
      strict_mode,
      &res);

  IOTJS_ASSERT(ret == JERRY_COMPLETION_CODE_OK ||
               ret == JERRY_COMPLETION_CODE_UNHANDLED_EXCEPTION);

  JResultType type = (ret == JERRY_COMPLETION_CODE_OK)
                     ? JRESULT_OK
                     : JRESULT_EXCEPTION;

  return JResult(&res, type);
}


void JObject::SetMethod(const char* name, JHandlerType handler) {
  IOTJS_ASSERT(IsObject());
  JObject method(jerry_api_create_external_function(handler));
  SetProperty(name, method);
}


void JObject::SetProperty(const char* name, const JObject& val) {
  IOTJS_ASSERT(IsObject());
  JRawValueType v = val.raw_value();
  bool is_ok  = jerry_api_set_object_field_value(
      _obj_val.v_object,
      reinterpret_cast<const jerry_api_char_t*>(name),
      &v);
  IOTJS_ASSERT(is_ok);
}


void JObject::SetProperty(const String& name, const JObject& val) {
  SetProperty(name.data(), val);
}


void JObject::SetProperty(const char* name, JRawValueType val) {
  IOTJS_ASSERT(IsObject());
  bool is_ok  = jerry_api_set_object_field_value(
        _obj_val.v_object,
        reinterpret_cast<const jerry_api_char_t*>(name),
        &val);
  IOTJS_ASSERT(is_ok);
}


void JObject::SetProperty(const String& name, JRawValueType val) {
  SetProperty(name.data(), val);
}


JObject JObject::GetProperty(const char* name) {
  IOTJS_ASSERT(IsObject());
  JRawValueType res;
  bool is_ok = jerry_api_get_object_field_value(
      _obj_val.v_object,
      reinterpret_cast<const jerry_api_char_t*>(name),
      &res);
  IOTJS_ASSERT(is_ok);
  return JObject(&res);
}


JObject JObject::GetProperty(const String& name) {
  return GetProperty(name.data());
}


void JObject::Ref() {
  if (JVAL_IS_STRING(&_obj_val)) {
    jerry_api_acquire_string(_obj_val.v_string);
  } else if (JVAL_IS_OBJECT(&_obj_val)) {
    jerry_api_acquire_object(_obj_val.v_object);
  }
}


void JObject::Unref() {
  if (JVAL_IS_STRING(&_obj_val)) {
    jerry_api_release_string(_obj_val.v_string);
  } else if (JVAL_IS_OBJECT(&_obj_val)) {
    jerry_api_release_object(_obj_val.v_object);
  }
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
  jerry_api_set_object_native_handle(_obj_val.v_object, ptr, free_handler);
}


uintptr_t JObject::GetNative() {
  IOTJS_ASSERT(IsObject());
  uintptr_t ptr;
  jerry_api_get_object_native_handle(_obj_val.v_object, &ptr);
  return ptr;
}


JResult JObject::Call(JObject& this_, JArgList& arg) {
  IOTJS_ASSERT(IsFunction());

  JRawObjectType* this_obj_p = this_.IsNull() ? NULL
                                              : this_.raw_value().v_object;
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

  bool is_ok = jerry_api_call_function(_obj_val.v_object,
                                       this_obj_p,
                                       &res,
                                       val_args,
                                       val_argv);

  if (val_args) {
    delete [] val_args;
  }

  JResultType type = is_ok ? JRESULT_OK : JRESULT_EXCEPTION;

  return JResult(&res, type);
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

  size_t size = -jerry_api_string_to_char_buffer(_obj_val.v_string, NULL, 0);

  String res("", size);

  jerry_api_char_t* buffer = reinterpret_cast<jerry_api_char_t*>(res.data());

  size_t check = jerry_api_string_to_char_buffer(_obj_val.v_string,
                                                 buffer,
                                                 size);

  IOTJS_ASSERT(check == size);

  return res;
}


#ifdef ENABLE_SNAPSHOT
JResult JObject::ExecSnapshot(const void *snapshot_p,
                              size_t snapshot_size) {
  JRawValueType res;
  jerry_completion_code_t ret;
  ret = jerry_exec_snapshot (snapshot_p,
                             snapshot_size,
                             false, /* the snapshot buffer
                                     * can be referenced
                                     * until jerry_cleanup is not called */
                             &res);

  IOTJS_ASSERT(ret == JERRY_COMPLETION_CODE_OK ||
               ret == JERRY_COMPLETION_CODE_UNHANDLED_EXCEPTION);

  JResultType type = (ret == JERRY_COMPLETION_CODE_OK)
                     ? JRESULT_OK
                     : JRESULT_EXCEPTION;

  return JResult(&res, type);
}
#endif


JResult::JResult(const JObject& value, JResultType type)
    : _value(value)
    , _type(type) {
}


JResult::JResult(const JRawValueType* raw_val, JResultType type)
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



JRawValueType JVal::Void() {
  JRawValueType val;
  val.type = JERRY_API_DATA_TYPE_VOID;
  return val;
}


JRawValueType JVal::Undefined() {
  JRawValueType val;
  val.type = JERRY_API_DATA_TYPE_UNDEFINED;
  return val;
}


JRawValueType JVal::Null() {
  JRawValueType val;
  val.type = JERRY_API_DATA_TYPE_NULL;
  return val;
}


JRawValueType JVal::Bool(bool v) {
  JRawValueType val;
  val.type = JERRY_API_DATA_TYPE_BOOLEAN;
  val.v_bool = v;
  return val;
}


JRawValueType JVal::Number(int v) {
  return JVal::Number((double)v);
}


JRawValueType JVal::Number(double v) {
  JRawValueType val;
  val.type = JERRY_API_DATA_TYPE_FLOAT64;
  val.v_float64 = v;
  return val;
}


JRawValueType JVal::Object(const JRawObjectType* obj) {
  JRawValueType val;
  val.type = JERRY_API_DATA_TYPE_OBJECT;
  val.v_object = const_cast<JRawObjectType*>(obj);
  return val;
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
  JObject jtmp(&x);
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
  JObject jtmp(&x);
  Set(i, jtmp);
}


JObject* JArgList::Get(uint16_t i) {
  IOTJS_ASSERT(i < _argc);
  return _argv[i];
}


JHandlerInfo::JHandlerInfo(const JRawObjectType* function_obj_p,
                           const JRawValueType* this_p,
                           JRawValueType* ret_val_p,
                           const JRawValueType args_p[],
                           const uint16_t args_cnt)
    : _function(function_obj_p, false)
    , _this(this_p, false)
    , _arg_list(args_cnt)
    , _ret_val_p(ret_val_p)
    , _thrown(false) {
  if (args_cnt > 0) {
    for (int i = 0; i < args_cnt; ++i) {
      JObject arg(&args_p[i], false);
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
  JObject jret(&raw_val);
  Return(jret);
}


void JHandlerInfo::Throw(JObject& ret) {
  IOTJS_ASSERT(_thrown == false);

  ret.Ref();
  *_ret_val_p = ret.raw_value();

  _thrown = true;
}


void JHandlerInfo::Throw(JRawValueType raw_val) {
  JObject jthrow(&raw_val);
  Throw(jthrow);
}


bool JHandlerInfo::HasThrown() {
  return _thrown;
}


} // namespace iotjs
