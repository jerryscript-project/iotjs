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

#include <assert.h>

#include "iotjs_binding.h"


namespace iotjs {

jerry_api_object_t* GetGlobal() {
  return jerry_api_get_global();
}

jerry_api_value_t JerryValFromObject(const jerry_api_object_t* obj) {
  jerry_api_value_t res;
  res.type = JERRY_API_DATA_TYPE_OBJECT;
  res.v_object = const_cast<jerry_api_object_t*>(obj);
  return res;
}

void SetObjectField(jerry_api_object_t* obj,
                    const char* name,
                    jerry_api_value_t* val) {
  bool is_ok = jerry_api_set_object_field_value(obj, name, val);
  assert(is_ok);
}

jerry_api_value_t GetObjectField(jerry_api_object_t* obj, const char* name) {
  jerry_api_value_t res;
  bool is_ok = jerry_api_get_object_field_value(obj, name, &res);
  assert(is_ok);
  return res;
}

void SetObjectNative(jerry_api_object_t* obj, uintptr_t ptr) {
  jerry_api_set_object_native_handle(obj, ptr);
}

uintptr_t GetObjectNative(jerry_api_object_t* obj) {
  uintptr_t res;
  bool is_ok = jerry_api_get_object_native_handle(obj, &res);
  assert(is_ok);
  return res;
}

size_t GetJerryStringLength(const jerry_api_value_t* val) {
  assert(JVAL_IS_STRING(val));
  return jerry_api_string_to_char_buffer (val->v_string, NULL, 0);
}

char* DupJerryString(const jerry_api_value_t* val) {
  assert(JVAL_IS_STRING(val));
  size_t size = -GetJerryStringLength(val);
  char* buffer = AllocCharBuffer(size);
  size_t check = jerry_api_string_to_char_buffer(val->v_string, buffer, size);
  assert(check == size);
  return buffer;
}

void ReleaseJerryString(char* str) {
  ReleaseCharBuffer(str);
}


JObject::JObject() {
  jerry_api_object_t* new_obj = jerry_api_create_object();
  _obj_val = JerryValFromObject(new_obj);
  _unref_at_close = true;
}

JObject::JObject(const JObject& other) {
  _obj_val = other._obj_val;
  _unref_at_close = true;
  Ref();
}

JObject::JObject(bool v) {
  _obj_val = JVal::Bool(v);
}

JObject::JObject(int32_t v) {
  _obj_val = JVal::Int(v);
}

JObject::JObject(float v) {
  _obj_val = JVal::Float(v);
}

JObject::JObject(double v) {
  _obj_val = JVal::Double(v);
}

JObject::JObject(const char* v) {
  _obj_val.type = JERRY_API_DATA_TYPE_STRING;
  _obj_val.v_string = jerry_api_create_string(v);
  _unref_at_close = true;
}

JObject::JObject(const jerry_api_object_t* obj, bool need_unref) {
  _obj_val = JerryValFromObject(obj);
  _unref_at_close = need_unref;
}

JObject::JObject(const jerry_api_value_t* val, bool need_unref) {
  _obj_val = *val;
  _unref_at_close = need_unref;
}

JObject::JObject(jerry_external_handler_t handler) {
  _obj_val.v_object = jerry_api_create_external_function(handler);
  assert(jerry_api_is_constructor(_obj_val.v_object));
  _obj_val.type = JERRY_API_DATA_TYPE_OBJECT;
  _unref_at_close = true;
}

JObject::~JObject() {
  if (_unref_at_close) {
    Unref();
  }
}


void JObject::CreateMethod(const char* name, jerry_external_handler_t handler) {
  assert(IsObject());
  JObject method(jerry_api_create_external_function(handler));
  SetProperty(name, &method);
}

void JObject::SetProperty(const char* name, JObject* val) {
  assert(IsObject());
  jerry_api_value_t v = val->val();
  SetObjectField(_obj_val.v_object, name, &v);
}

JObject JObject::GetProperty(const char* name) {
  assert(IsObject());
  jerry_api_value_t val = GetObjectField(_obj_val.v_object, name);
  return JObject(&val);
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

void JObject::SetNative(uintptr_t ptr) {
  assert(IsObject());
  jerry_api_set_object_native_handle(_obj_val.v_object, ptr);
}

uintptr_t JObject::GetNative() {
  assert(IsObject());
  uintptr_t ptr;
  jerry_api_get_object_native_handle(_obj_val.v_object, &ptr);
  return ptr;
}

void JObject::SetFreeCallback(jerry_object_free_callback_t freecb) {
  assert(IsObject());
  jerry_api_set_object_free_callback(_obj_val.v_object, freecb);
}

JObject JObject::Call(JObject* this_, JObject** args, uint16_t argv) {
  assert(IsFunction());

  jerry_api_object_t* this_obj_p = this_ ? this_->val().v_object : NULL;
  jerry_api_value_t res;
  jerry_api_value_t* val_args = NULL;
  uint16_t val_argv = 0;

  if (argv > 0) {
    val_args = new jerry_api_value_t[argv];
    val_argv = argv;
    for (int i = 0; i < argv; ++i) {
      val_args[i] = args[i]->val();
    }
  }

  bool is_ok = jerry_api_call_function(_obj_val.v_object,
                                       this_obj_p,
                                       &res,
                                       val_args,
                                       val_argv);
  assert(is_ok);

  if (val_args) {
    delete val_args;
  }

  return JObject(&res);
}


jerry_api_value_t JVal::Void() {
  jerry_api_value_t val;
  val.type = JERRY_API_DATA_TYPE_VOID;
  return val;
}

jerry_api_value_t JVal::Undefined() {
  jerry_api_value_t val;
  val.type = JERRY_API_DATA_TYPE_UNDEFINED;
  return val;
}

jerry_api_value_t JVal::Null() {
  jerry_api_value_t val;
  val.type = JERRY_API_DATA_TYPE_NULL;
  return val;
}

jerry_api_value_t JVal::Bool(bool v) {
  jerry_api_value_t val;
  val.type = JERRY_API_DATA_TYPE_BOOLEAN;
  val.v_bool = v;
  return val;
}

jerry_api_value_t JVal::Int(int32_t v) {
  jerry_api_value_t val;
  val.type = JERRY_API_DATA_TYPE_UINT32;
  val.v_uint32 = v;
  return val;
}

jerry_api_value_t JVal::Float(float v) {
  jerry_api_value_t val;
  val.type = JERRY_API_DATA_TYPE_FLOAT32;
  val.v_uint32 = v;
  return val;
}

jerry_api_value_t JVal::Double(double v) {
  jerry_api_value_t val;
  val.type = JERRY_API_DATA_TYPE_FLOAT64;
  val.v_uint32 = v;
  return val;
}


JLocalScope::JLocalScope() {
}

JLocalScope::~JLocalScope() {
  _object_list.Clear();
}

JObject* JLocalScope::CreateJObject() {
  JObject* new_object = new JObject();
  _object_list.InsertTail(new_object);
  return new_object;
}

} // namespace iotjs
