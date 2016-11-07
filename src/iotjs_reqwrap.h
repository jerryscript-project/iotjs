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

#ifndef IOTJS_REQWRAP_H
#define IOTJS_REQWRAP_H


#include <uv.h>

#include "iotjs_binding.h"


namespace iotjs {


// UV request wrapper.
// Wrapping UV request and javascript callback.
// When an instance of request wrapper is created. it will increase ref count
// for javascript callback function to prevent it from reclaimed by GC. The
// reference count will decrease back when wrapper is being freed.
template<typename T>
class ReqWrap {
 public:
  ReqWrap(const iotjs_jval_t* jcallback);
  virtual ~ReqWrap();

  // To retrieve javascript callback funciton object.
  const iotjs_jval_t* jcallback();

  // To retrieve pointer to uv request.
  T* req();

 protected:
  T _req;
  iotjs_jval_t _jcallback;
};


template<typename T>
ReqWrap<T>::ReqWrap(const iotjs_jval_t* jcallback) {
  _jcallback = iotjs_jval_create_copied(jcallback);
  _req.data = this;
}


template<typename T>
ReqWrap<T>::~ReqWrap() {
  iotjs_jval_destroy(&_jcallback);
}


template<typename T>
const iotjs_jval_t* ReqWrap<T>::jcallback() {
  IOTJS_ASSERT(!iotjs_jval_is_null(&this->_jcallback));
  return &this->_jcallback;
}


template<typename T>
T* ReqWrap<T>::req() {
  return &_req;
}


} // namespace iotjs


#endif /* IOTJS_REQWRAP_H */
