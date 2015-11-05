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

#ifndef IOTJS_BINDING_H
#define IOTJS_BINDING_H

#include "jerry-api.h"
#include "iotjs_util.h"

#include <stdio.h>


namespace iotjs {


typedef jerry_external_handler_t JHandlerType;
typedef jerry_object_free_callback_t JFreeHandlerType;
typedef jerry_api_object_t JRawObjectType;
typedef jerry_api_value_t JRawValueType;
typedef jerry_api_length_t JRawLengthType;


class JObject;
class JResult;
class JArgList;
class JHandlerInfo;
class JLocalScope;

enum JResultType {
  JRESULT_OK,
  JRESULT_EXCEPTION
};


/// Wrapper for Javascript objects.
class JObject {
 public:
  // Consturctors.

  // Creates a initail javascript object.
  explicit JObject();

  // Creates a object copied from other object.
  JObject(const JObject& other);

  // Creates a javascript boolean object.
  explicit JObject(bool v);

  // Creates a javascript number object from various c type.
  explicit JObject(int v);
  explicit JObject(double v);

  // Creates a javascirpt string object.
  explicit JObject(const char* v);
  explicit JObject(const String& v);

  // Creates a object from `JRawObjectType*`.
  // If second argument set true, then ref count for the object will be
  // decreased when this wrapper is being destroyed.
  explicit JObject(const JRawObjectType* obj, bool need_unref = true);

  // Creates a object from `JRawValueType*`.
  // If second argument set true, then ref count for the object will be
  // decreased when this wrapper is being destroyed.
  explicit JObject(const JRawValueType* val, bool need_unref = true);

  // Creates a javascript function object.
  // When the function is called, the handler will be triggered.
  explicit JObject(JHandlerType handler);

  // Create a javascript null object.
  static JObject& Null();

  // Crate a javascript undefined object.
  static JObject& Undefined();

  // Get the javascript global object.
  static JObject Global();

  // Create a javascript error object.
  static JObject Error(const char* message);
  static JObject Error(const String& message);

  static JObject EvalError(const char* message);
  static JObject EvalError(const String& message);

  static JObject RangeError(const char* message);
  static JObject RangeError(const String& message);

  static JObject ReferenceError(const char* message);
  static JObject ReferenceError(const String& message);

  static JObject SyntaxError(const char* message);
  static JObject SyntaxError(const String& message);

  static JObject TypeError(const char* message);
  static JObject TypeError(const String& message);

  static JObject URIError(const char* message);
  static JObject URIError(const String& message);


  // Evaluate javascript source file.
  static JResult Eval(const String& source,
                      bool direct_mode = true,
                      bool strict_mode = false);


  // Destoyer for this class.
  // When the wrapper is being destroyed, ref count for correspoding javascript
  // object will be decreased unless `need_unref` was set false.
  ~JObject();

  // Increases ref count.
  void Ref();

  // Decreases ref count.
  void Unref();

  // Returns whether the object is specific type.
  bool IsNull();
  bool IsUndefined();
  bool IsBoolean();
  bool IsNumber();
  bool IsString();
  bool IsObject();
  bool IsFunction();

  // Sets native handler method for the javascript object.
  void SetMethod(const char* name, JHandlerType handler);

  // Sets & gets property for the javascript object.
  void SetProperty(const char* name, const JObject& val);
  void SetProperty(const String& name, const JObject& val);

  void SetProperty(const char* name, JRawValueType val);
  void SetProperty(const String& name, JRawValueType val);

  JObject GetProperty(const char* name);
  JObject GetProperty(const String& name);

  // Sets & gets native data for the javascript object.
  void SetNative(uintptr_t ptr, JFreeHandlerType free_handler);
  uintptr_t GetNative();

  // Returns value for boolean contents of the object.
  bool GetBoolean();

  // Retruns value for 32bit integer contents of number object.
  int32_t GetInt32();

  // Returns value for 64bit integer contents of number object.
  int64_t GetInt64();

  // Returns value for number contents of number object.
  double GetNumber();

  // Returns value for string contents of string object.
  String GetString();

#ifdef ENABLE_SNAPSHOT
  // Evaluate javascript snapshot.
  static JResult ExecSnapshot(const void *snapshot_p,
                              size_t snapshot_size);
#endif

  // Calls javascript function.
  JResult Call(JObject& this_, JArgList& arg);
  JObject CallOk(JObject& this_, JArgList& arg);

  JRawValueType raw_value() const { return _obj_val; }

 protected:
  JRawValueType _obj_val;
  bool _unref_at_close;

 private:
  // disable assignment.
  JObject& operator=(const JObject& rhs) = delete;
};


class JResult {
 public:
  JResult(const JObject& value, JResultType type);
  JResult(const JRawValueType* raw_val, JResultType type);
  JResult(const JResult& other);

  JObject& value();
  JResultType type() const;

  bool IsOk() const;
  bool IsException() const;

 private:
  JObject _value;
  JResultType _type;

  // disable assignment.
  JResult& operator=(const JResult& rhs) = delete;
};


class JVal {
 public:
  static JRawValueType Void();
  static JRawValueType Undefined();
  static JRawValueType Null();
  static JRawValueType Bool(bool v);
  static JRawValueType Number(int v);
  static JRawValueType Number(double v);
  static JRawValueType Object(const JRawObjectType* obj);
};


class JArgList {
 public:
  JArgList(uint16_t capacity);
  ~JArgList();

  static JArgList& Empty();

  uint16_t GetLength();

  void Add(JObject& x);
  void Add(JRawValueType x);

  void Set(uint16_t i, JObject& x);
  void Set(uint16_t i, JRawValueType x);

  JObject* Get(uint16_t i);

 private:
  uint16_t _capacity;
  uint16_t _argc;
  JObject** _argv;
};


class JHandlerInfo {
 public:
  JHandlerInfo(const JRawObjectType* function_obj_p,
               const JRawValueType* this_p,
               JRawValueType* ret_val_p,
               const JRawValueType args_p[],
               const uint16_t args_cnt);
  ~JHandlerInfo();

  JObject* GetFunction();
  JObject* GetThis();
  JObject* GetArg(uint16_t i);
  uint16_t GetArgLength();

  void Return(JObject& ret);
  void Return(JRawValueType raw_val);

  void Throw(JObject& err);
  void Throw(JRawValueType raw_val);

  bool HasThrown();

 private:
  JObject _function;
  JObject _this;
  JArgList _arg_list;
  JRawValueType* _ret_val_p;
  bool _thrown;
};


#define JHANDLER_THROW(error_type, message) \
  JObject error = JObject::error_type(message); \
  handler.Throw(error);

#define JHANDLER_THROW_RETURN(error_type, message) \
  JHANDLER_THROW(error_type, message); \
  return false;

#define JHANDLER_CHECK(predicate) \
  if (!(predicate)) { \
    char buffer[64]; \
    snprintf(buffer, 63, "Internal error (%s)", __func__); \
    JHANDLER_THROW_RETURN(Error, buffer) \
  }

#define JHANDLER_FUNCTION(name) \
  static bool ___ ## name ## _native(JHandlerInfo& handler); \
  static bool name(const JRawObjectType *function_obj_p, \
                   const JRawValueType *this_p, \
                   JRawValueType *ret_val_p, \
                   const JRawValueType args_p [], \
                   const JRawLengthType args_cnt) { \
    JHandlerInfo handler(function_obj_p, this_p, ret_val_p, args_p, args_cnt); \
    return ___ ## name ## _native(handler); \
  } \
  static bool ___ ## name ## _native(JHandlerInfo& handler)


} // namespace iotjs


#endif /* IOTJS_BINDING_H */
