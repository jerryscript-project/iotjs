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

#include <stdlib.h>

#include "iotjs_binding.h"
#include "iotjs_module.h"
#include "iotjs_module_buffer.h"


namespace iotjs {

static bool Write(const jerry_api_object_t *function_obj_p,
                  const jerry_api_value_t *this_p,
                  jerry_api_value_t *ret_val_p,
                  const jerry_api_value_t args_p [],
                  const uint16_t args_cnt) {
  assert(args_cnt == 3);
  assert(JVAL_IS_STRING(&args_p[0]));
  assert(JVAL_IS_NUMBER(&args_p[1]));
  assert(JVAL_IS_NUMBER(&args_p[2]));

  char* src = DupJerryString(&args_p[0]);

  int offset = JVAL_TO_INT32(&args_p[1]);
  int length = JVAL_TO_INT32(&args_p[2]);

  JObject buffer_obj(this_p, false);
  char* buffer = reinterpret_cast<char*>(buffer_obj.GetNative());
  assert(buffer != NULL);

  int buffer_length = buffer_obj.GetProperty("length").valInt32();
  assert(buffer_length >= offset + length);

  // TODO: memcpy is not supported by nuttx.
  // TODO: we need to account encoding.
  // memcpy(buffer + offset, src, length);
  for (int i = 0; i < length; ++i) {
    *(buffer + offset + i) = *(src + i);
  }

  *ret_val_p = JVal::Int(length);

  ReleaseJerryString(src);

  return true;
}

static bool ToString(const jerry_api_object_t *function_obj_p,
                     const jerry_api_value_t *this_p,
                     jerry_api_value_t *ret_val_p,
                     const jerry_api_value_t args_p [],
                     const uint16_t args_cnt) {
  assert(args_cnt == 0);

  JObject buffer_obj(this_p, false);
  char* buffer = reinterpret_cast<char*>(buffer_obj.GetNative());
  assert(buffer != NULL);

  JObject jstring(buffer);
  jstring.Ref();
  *ret_val_p = jstring.val();

  return true;
}

static bool Copy(const jerry_api_object_t *function_obj_p,
                 const jerry_api_value_t *this_p,
                 jerry_api_value_t *ret_val_p,
                 const jerry_api_value_t args_p [],
                 const uint16_t args_cnt) {
  assert(args_cnt >= 1);
  assert(JVAL_IS_OBJECT(&args_p[0]));
  assert(args_cnt <= 1 || JVAL_IS_NUMBER(&args_p[1]));
  assert(args_cnt <= 2 || JVAL_IS_NUMBER(&args_p[2]));
  assert(args_cnt <= 3 || JVAL_IS_NUMBER(&args_p[3]));
  assert(JVAL_IS_OBJECT(this_p));

  JObject source_buffer_obj(this_p, false);
  int source_length = source_buffer_obj.GetProperty("length").valInt32();
  char* source_buffer = reinterpret_cast<char*>(source_buffer_obj.GetNative());

  JObject target_buffer_obj(&args_p[0], false);
  int target_length = target_buffer_obj.GetProperty("length").valInt32();
  char* target_buffer = reinterpret_cast<char*>(target_buffer_obj.GetNative());

  int target_start = 0;
  int source_start = 0;
  int source_end = source_length;

  if (args_cnt >= 2) {
    target_start = JVAL_TO_INT32(&args_p[1]);
  }
  if (args_cnt >= 3) {
    source_start = JVAL_TO_INT32(&args_p[2]);
  }
  if (args_cnt >= 4) {
    source_end = JVAL_TO_INT32(&args_p[3]);
    if (source_end > source_length) {
      source_end = source_length;
    }
  }

  int copied = 0;
  for (int i = source_start, j = target_start;
       i < source_end && j < target_length;
       ++i, ++j) {
    *(target_buffer + j) = *(source_buffer + i);
    ++copied;
  }

  *ret_val_p = JVal::Int(copied);

  return true;
}

static bool SetupBufferJs(const jerry_api_object_t *function_obj_p,
                          const jerry_api_value_t *this_p,
                          jerry_api_value_t *ret_val_p,
                          const jerry_api_value_t args_p [],
                          const uint16_t args_cnt) {
  assert(args_cnt == 1);
  assert(JVAL_IS_FUNCTION(&args_p[0]));

  JObject jbuffer(&args_p[0], false);
  JObject prototype(jbuffer.GetProperty("prototype"));
  prototype.CreateMethod("_write", Write);
  prototype.CreateMethod("_toString", ToString);
  prototype.CreateMethod("copy", Copy);

  return true;
}

static bool Alloc(const jerry_api_object_t *function_obj_p,
                  const jerry_api_value_t *this_p,
                  jerry_api_value_t *ret_val_p,
                  const jerry_api_value_t args_p [],
                  const uint16_t args_cnt) {
  assert(args_cnt == 2);
  assert(JVAL_IS_OBJECT(&args_p[0]));
  assert(JVAL_IS_NUMBER(&args_p[1]));

  int length = JVAL_TO_INT32(&args_p[1]);
  char* buffer = AllocCharBuffer(length);

  JObject buffer_obj(&args_p[0], false);
  buffer_obj.SetNative(reinterpret_cast<uintptr_t>(buffer));

  *ret_val_p = JVal::Int(length);

  return true;
}


JObject* InitBuffer() {
  Module* module = GetBuiltinModule(MODULE_BUFFER);
  JObject* buffer = module->module;

  if (buffer == NULL) {
    buffer = new JObject();
    buffer->CreateMethod("setupBufferJs", SetupBufferJs);
    buffer->CreateMethod("alloc", Alloc);

    module->module = buffer;
  }

  return buffer;
}

} // namespace iotjs
