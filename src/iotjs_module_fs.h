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

#ifndef IOTJS_MODULE_FS_H
#define IOTJS_MODULE_FS_H

#include "uv.h"

#include "iotjs_binding.h"
#include "iotjs.h"

namespace iotjs {

class FsReqWrap {
 public:
  FsReqWrap(JObject* jcallback);
  ~FsReqWrap();

  uv_req_t* req() { return (uv_req_t*)&_data; }
  uv_fs_t* data() { return &_data; }
  JObject* callback() { return _callback; }

  void Dispatched() { req()->data = this; }

 private:
  uv_fs_t _data;
  JObject* _callback;
};

JObject* InitFs();

} // namespace iotjs

#endif /* IOTJS_MODULE_FS_H */
