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

#include "uv.h"

#include "iotjs_binding.h"


namespace iotjs {

class HandleWrap {
 public:
  HandleWrap(JObject* jobj, uv_handle_t* handle);
  virtual ~HandleWrap();

  JObject* object();

  JObject* callback();
  void set_callback(JObject& jcallback);

 protected:
  uv_handle_t* __handle;
  JObject* _jobj;
  JObject* _jcallback;
};

} // namespace iotjs

#endif /* IOTJS_HANDLEWRAP_H */
