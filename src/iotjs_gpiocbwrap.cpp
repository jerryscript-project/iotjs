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

#include "iotjs_gpiocbwrap.h"


namespace iotjs {


GpioCbWrap::GpioCbWrap(JObject& jcallback, gpio_cb_t* cb)
    : __cb(cb)
    , _jcallback(NULL) {
  if (!jcallback.IsNull()) {
    _jcallback = new JObject(jcallback);
  }
}


GpioCbWrap::~GpioCbWrap() {
  if (_jcallback != NULL) {
    delete _jcallback;
  }
}


JObject& GpioCbWrap::jcallback() {
  IOTJS_ASSERT(_jcallback != NULL);
  return *_jcallback;
}


gpio_cb_t* GpioCbWrap::cb() {
  return __cb;
}


void GpioCbWrap::Dispatched() {
  cb()->data = this;
}


} // namespace iotjs
