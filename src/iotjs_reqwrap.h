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
  ReqWrap(JObject& jcallback);
  virtual ~ReqWrap();

  // To retrieve javascript callback funciton object.
  JObject& jcallback();

  // To retrieve pointer to uv request.
  T* req();

 protected:
  T _req;
  JObject* _jcallback;
};


template<typename T>
ReqWrap<T>::ReqWrap(JObject& jcallback)
    : _jcallback(NULL) {
  if (!jcallback.IsNull()) {
    _jcallback = new JObject(jcallback);
  }
  _req.data = this;
}


template<typename T>
ReqWrap<T>::~ReqWrap() {
  if (_jcallback != NULL) {
    delete _jcallback;
  }
}


template<typename T>
JObject& ReqWrap<T>::jcallback() {
  IOTJS_ASSERT(this->_jcallback != NULL);
  return *(this->_jcallback);
}


template<typename T>
T* ReqWrap<T>::req() {
  return &_req;
}


} // namespace iotjs


#endif /* IOTJS_REQWRAP_H */
