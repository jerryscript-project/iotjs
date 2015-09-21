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
// This wrapper connects a Javascript object and a libuv handler.
// This wrapper will increase ref count for the Javascript object and decrease
//  it after corresponding handle has closed. Hence the Javasciprt object will
//  not turn into garbage untile the handle is open.

// Javascirpt object
//   ->
// Create a handle wrap, initializing uv handle, increase ref count.
//   ->
// The javascript object will be alive until handle has closed.
//   ->
// Handle closed, release handle, decrease ref count.
//   ->
// The javascript object now can be reclaimed by GC.
class HandleWrap : public JObjectWrap {
 public:
  HandleWrap(JObject& jobject, /* Object that connect with the uv handle*/
             uv_handle_t* handle);

  virtual ~HandleWrap();

  static HandleWrap* FromHandle(uv_handle_t* handle);

  typedef void (*OnCloseHandler)(uv_handle_t*);

  void Close(OnCloseHandler on_close_cb);

  void OnClose();

 protected:
  virtual void Destroy(void);

 protected:
  uv_handle_t* __handle;
  OnCloseHandler _on_close_cb;
};


} // namespace iotjs


#endif /* IOTJS_HANDLEWRAP_H */
