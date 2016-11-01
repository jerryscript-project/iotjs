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

#ifndef IOTJS_BINDING_H
#define IOTJS_BINDING_H

#include "jerry-api.h"
#include "iotjs_util.h"

#include <stdio.h>


#ifdef __cplusplus
extern "C" {
#endif

typedef jerry_external_handler_t JHandlerType;
typedef jerry_object_free_callback_t JFreeHandlerType;
typedef jerry_value_t JRawValueType;
typedef jerry_length_t JRawLengthType;

struct iotjs_jargs_t;

enum JResultType {
  JRESULT_OK,
  JRESULT_EXCEPTION
};


void iotjs_binding_initialize();
void iotjs_binding_finalize();


#ifdef __cplusplus
} // extern "C"
#endif


namespace iotjs {


class JObject;
class JResult;


/// Wrapper for Javascript objects.
class JObject {
 public:
  // Consturctors

  // Creates an initial javascript object.
  explicit JObject();

  // Creates an object copied from other object.
  JObject(const JObject& other);

  // Creates a javascript boolean object.
  explicit JObject(bool v);

  // Creates a javascript number object from various c type.
  explicit JObject(int v);
  explicit JObject(double v);

  // Creates a javascript string object.
  explicit JObject(const char* v);
  explicit JObject(const iotjs_string_t& v);

  // Creates a javascript array object from char array
  explicit JObject(uint32_t len, const char* data);

  // Creates an object from `JRawValueType*`.
  // If second argument set true, then ref count for the object will be
  // decreased when this wrapper is being destroyed.
  explicit JObject(const JRawValueType val, bool need_unref = true);

  // Creates a javascript function object.
  // When the function is called, the handler will be triggered.
  explicit JObject(JHandlerType handler);

  // Initializes statically null and undefined objects.
  // This should be called once before javascript API calls
  static void init();

  // Releases null and undefined objects.
  static void cleanup();

  // Creates a javascript null object.
  static JObject& Null();

  // Creates a javascript undefined object.
  static JObject& Undefined();

  // Gets the javascript global object.
  static JObject Global();

  // Creates a javascript error object.
  static JObject Error(const char* message);
  static JObject EvalError(const char* message);
  static JObject RangeError(const char* message);
  static JObject ReferenceError(const char* message);
  static JObject SyntaxError(const char* message);
  static JObject TypeError(const char* message);
  static JObject URIError(const char* message);

  // Evaluate javascript source file.
  static JResult Eval(const iotjs_string_t& source,
                      bool strict_mode = false);


  // Destructor for this class
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
  bool IsArray();

  // Sets native handler method for the javascript object.
  void SetMethod(const char* name, JHandlerType handler);

  // Sets & gets property for the javascript object.
  void SetProperty(const char* name, const JObject& val);
  void SetProperty(const char* name, JRawValueType val);
  void SetPropertyByIdx(uint32_t idx, const JObject& val);
  void SetPropertyByIdx(uint32_t idx, JRawValueType val);

  JObject GetProperty(const char* name);

  // Sets & gets native data for the javascript object.
  void SetNative(uintptr_t ptr, JFreeHandlerType free_handler);
  uintptr_t GetNative();

  // Returns value for boolean contents of the object.
  bool GetBoolean();

  // Returns value for 32bit integer contents of number object.
  int32_t GetInt32();

  // Returns value for 64bit integer contents of number object.
  int64_t GetInt64();

  // Returns value for number contents of number object.
  double GetNumber();

  // Returns value for string contents of string object.
  iotjs_string_t GetString();

#ifdef ENABLE_SNAPSHOT
  // Evaluate javascript snapshot.
  static JResult ExecSnapshot(const void *snapshot_p,
                              size_t snapshot_size);
#endif

  // Calls javascript function.
  JResult Call(JObject& this_, iotjs_jargs_t& arg);
  JObject CallOk(JObject& this_, iotjs_jargs_t& arg);

  JRawValueType raw_value() const { return _obj_val; }

 protected:
  JRawValueType _obj_val;
  bool _unref_at_close;

 private:
  // disable assignment.
  JObject& operator=(const JObject& rhs) = delete;

  static JObject* _null;
  static JObject* _undefined;
};


class JResult {
 public:
  JResult(const JObject& value, JResultType type);
  JResult(const JRawValueType raw_val, JResultType type);
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


} // namespace iotjs


#ifdef __cplusplus
extern "C" {
#endif


JRawValueType iotjs_jval_undefined();
JRawValueType iotjs_jval_null();
JRawValueType iotjs_jval_bool(bool v);
JRawValueType iotjs_jval_number(double v);
JRawValueType iotjs_jval_string(const iotjs_string_t* v);
JRawValueType iotjs_jval_raw_string(const char* data);


typedef struct {
  uint16_t capacity;
  uint16_t argc;
  iotjs::JObject** argv;
} IOTJS_VALIDATED_STRUCT(iotjs_jargs_t);

extern iotjs_jargs_t iotjs_jargs_empty;

iotjs_jargs_t iotjs_jargs_create(uint16_t capacity);

void iotjs_jargs_destroy(iotjs_jargs_t* jargs);

uint16_t iotjs_jargs_length(iotjs_jargs_t* jargs);

void iotjs_jargs_append_obj(iotjs_jargs_t* jargs, iotjs::JObject* x);
void iotjs_jargs_append_bool(iotjs_jargs_t* jargs, bool x);
void iotjs_jargs_append_number(iotjs_jargs_t* jargs, double x);
void iotjs_jargs_append_string(iotjs_jargs_t* jargs, const iotjs_string_t* x);
void iotjs_jargs_append_raw_string(iotjs_jargs_t* jargs, const char* x);
void iotjs_jargs_append_undefined(iotjs_jargs_t* jargs);
void iotjs_jargs_append_null(iotjs_jargs_t* jargs);

void iotjs_jargs_replace(iotjs_jargs_t* jargs, uint16_t i, iotjs::JObject* x);

iotjs::JObject* iotjs_jargs_get(iotjs_jargs_t* jargs, uint16_t i);


typedef struct {
  iotjs::JObject* function;
  iotjs::JObject* this_val;
  iotjs_jargs_t arg_list;
  JRawValueType* ret_val_p;
#ifndef NDEBUG
  bool finished;
#endif
} IOTJS_VALIDATED_STRUCT(iotjs_jhandler_t);

void iotjs_jhandler_initialize(iotjs_jhandler_t* jhandler,
                               const JRawValueType func_obj_val,
                               const JRawValueType this_val,
                               JRawValueType* ret_val_p,
                               const JRawValueType args_p[],
                               const uint16_t args_cnt);

void iotjs_jhandler_destroy(iotjs_jhandler_t* jhandler);

iotjs::JObject* iotjs_jhandler_get_function(iotjs_jhandler_t* jhandler);
iotjs::JObject* iotjs_jhandler_get_this(iotjs_jhandler_t* jhandler);
iotjs::JObject* iotjs_jhandler_get_arg(iotjs_jhandler_t* jhandler, uint16_t i);
uint16_t iotjs_jhandler_get_arg_length(iotjs_jhandler_t* jhandler);

void iotjs_jhandler_return_obj(iotjs_jhandler_t* jhandler, iotjs::JObject* ret);
void iotjs_jhandler_return_bool(iotjs_jhandler_t* jhandler, bool x);
void iotjs_jhandler_return_number(iotjs_jhandler_t* jhandler, double x);
void iotjs_jhandler_return_undefined(iotjs_jhandler_t* jhandler);
void iotjs_jhandler_return_null(iotjs_jhandler_t* jhandler);
void iotjs_jhandler_return_string(iotjs_jhandler_t* jhandler,
                                  const iotjs_string_t* x);
void iotjs_jhandler_return_raw_string(iotjs_jhandler_t* jhandler,
                                      const char* x);

void iotjs_jhandler_throw_obj(iotjs_jhandler_t* jhandler, iotjs::JObject* err);
void iotjs_jhandler_throw_val(iotjs_jhandler_t* jhandler, JRawValueType err);


#define JHANDLER_THROW(error_type, message) \
  JObject error = JObject::error_type(message); \
  iotjs_jhandler_throw_obj(jhandler, &error);

#define JHANDLER_THROW_RETURN(error_type, message) \
  JHANDLER_THROW(error_type, message); \
  return;

#define JHANDLER_CHECK(predicate) \
  if (!(predicate)) { \
    char buffer[64]; \
    snprintf(buffer, 63, "Internal error (%s)", __func__); \
    JHANDLER_THROW_RETURN(Error, buffer) \
  }

#define JHANDLER_FUNCTION(name) \
  static void ___ ## name ## _native(iotjs_jhandler_t* jhandler); \
  static JRawValueType name(const JRawValueType func_obj_val, \
                            const JRawValueType this_val, \
                            const JRawValueType args_p [], \
                            const JRawLengthType args_cnt) { \
    JRawValueType ret_val = jerry_create_undefined(); \
    iotjs_jhandler_t jhandler; \
    iotjs_jhandler_initialize(&jhandler, func_obj_val, this_val, &ret_val, \
                              args_p, args_cnt); \
    ___ ## name ## _native(&jhandler); \
    iotjs_jhandler_destroy(&jhandler); \
    return ret_val; \
  } \
  static void ___ ## name ## _native(iotjs_jhandler_t* jhandler)


#ifdef __cplusplus
} // extern "C"
#endif


#endif /* IOTJS_BINDING_H */
