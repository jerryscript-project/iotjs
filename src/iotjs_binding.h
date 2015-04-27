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


namespace iotjs {

#define JVAL_IS_STRING(val_p) ((val_p)->type == JERRY_API_DATA_TYPE_STRING)
#define JVAL_IS_OBJECT(val_p) ((val_p)->type == JERRY_API_DATA_TYPE_OBJECT)
#define JVAL_IS_FUNCTION(val_p) \
  (JVAL_IS_OBJECT(val_p) && jerry_api_is_function((val_p)->v_object))

#define JVAL_IS_NUMBER(val_p) \
  (((val_p)->type == JERRY_API_DATA_TYPE_FLOAT32) || \
   ((val_p)->type == JERRY_API_DATA_TYPE_FLOAT64) || \
   ((val_p)->type == JERRY_API_DATA_TYPE_UINT32))

#define JVAL_TO_INT32(val_p) \
   ((val_p)->type == JERRY_API_DATA_TYPE_FLOAT32 ? \
      static_cast<int32_t>((val_p)->v_float32) : \
    (val_p)->type == JERRY_API_DATA_TYPE_FLOAT64 ? \
      static_cast<int32_t>((val_p)->v_float64) : \
    static_cast<int32_t>((val_p)->v_uint32))

#define JVAL_TO_INT64(val_p) \
   ((val_p)->type == JERRY_API_DATA_TYPE_FLOAT32 ? \
      static_cast<int64_t>((val_p)->v_float32) : \
    (val_p)->type == JERRY_API_DATA_TYPE_FLOAT64 ? \
      static_cast<int64_t>((val_p)->v_float64) : \
    static_cast<int64_t>((val_p)->v_uint32))

#define JERRY_THROW(msg) do { assert(!"not implemented"); } while (false)


jerry_api_object_t* GetGlobal();

jerry_api_value_t JerryValFromObject(const jerry_api_object_t* obj);

void SetObjectField(jerry_api_object_t* obj,
                    const char* name,
                    jerry_api_value_t* val);

jerry_api_value_t GetObjectField(jerry_api_object_t* obj, const char* name);

void SetObjectNative(jerry_api_object_t* obj, uintptr_t ptr);

uintptr_t GetObjectNative(jerry_api_object_t* obj);

size_t GetJerryStringLength(const jerry_api_value_t* val);
char* DupJerryString(const jerry_api_value_t* val);
void ReleaseJerryString(char* str);




class JObject;
class JLocalScope;


class JObject {
 public:
  explicit JObject();
  JObject(const JObject& other);
  explicit JObject(bool v);
  explicit JObject(int32_t v);
  explicit JObject(float v);
  explicit JObject(double v);
  explicit JObject(const char* v);
  explicit JObject(const jerry_api_object_t* obj, bool need_unref = true);
  explicit JObject(const jerry_api_value_t* val, bool need_unref = true);
  explicit JObject(jerry_external_handler_t handler);
  ~JObject();

  void Ref();
  void Unref();

  bool IsNumber() { return JVAL_IS_NUMBER(&_obj_val); }
  bool IsString() { return JVAL_IS_STRING(&_obj_val); }
  bool IsObject() { return JVAL_IS_OBJECT(&_obj_val); }
  bool IsFunction() { return JVAL_IS_FUNCTION(&_obj_val); }

  void CreateMethod(const char* name, jerry_external_handler_t handler);
  void SetProperty(const char* name, JObject* val);
  JObject GetProperty(const char* name);
  void SetNative(uintptr_t ptr);
  JObject Call(JObject* this_, JObject** args, uint16_t argv);
  uintptr_t GetNative();

  jerry_api_value_t val() { return _obj_val; }
  int valInt32() { return JVAL_TO_INT32(&_obj_val); }

 private:
  jerry_api_value_t _obj_val;
  bool _unref_at_close;
};


class JVal {
 public:
  static jerry_api_value_t Void();
  static jerry_api_value_t Undefined();
  static jerry_api_value_t Null();
  static jerry_api_value_t Bool(bool v);
  static jerry_api_value_t Int(int32_t v);
  static jerry_api_value_t Float(float v);
  static jerry_api_value_t Double(double v);
};


class JLocalScope {
 public:
  explicit JLocalScope();
  ~JLocalScope();

  JObject* CreateJObject();

 private:
  LinkedList<JObject*> _object_list;
};

} // namespace iotjs


#endif /* IOTJS_BINDING_H */

