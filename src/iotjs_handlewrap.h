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

#ifndef IOTJS_HANDLEWRAP_H
#define IOTJS_HANDLEWRAP_H


#include <uv.h>

#include "iotjs_binding.h"
#include "iotjs_objectwrap.h"


namespace iotjs {


// UV handle wrapper.
// This wrapper might refer javascript object but never increase reference count
// If the object is freed by GC, then this wrapper instance will be also freed.
class HandleWrap : public JObjectWrap {
 public:
  HandleWrap(JObject& jnative, /* Native object */
             JObject& jholder, /* Object hodling the native object */
             uv_handle_t* handle);

  virtual ~HandleWrap();

  static HandleWrap* FromHandle(uv_handle_t* handle);

  // Native object.
  JObject& jnative();

  // Javascript object that holds the native object.
  JObject& jholder();
  void set_jholder(JObject& jholder);

  typedef void (*OnCloseHandler)(uv_handle_t*);

  void Close(OnCloseHandler on_close_cb);

 protected:
  uv_handle_t* __handle;
  JObject* _jholder;
};


} // namespace iotjs


#endif /* IOTJS_HANDLEWRAP_H */
