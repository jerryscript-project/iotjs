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

#ifndef IOTJS_GPIOCBWRAP_H
#define IOTJS_GPIOCBWRAP_H

#include "iotjs_def.h"
#include "iotjs_objectwrap.h"


namespace iotjs {

struct gpio_cb_s {
  void* data;
};

typedef struct gpio_cb_s gpio_cb_t;


class GpioCbWrap {
 public:
  GpioCbWrap(JObject& jcallback, gpio_cb_t* gpiocb);
  virtual ~GpioCbWrap();

  JObject& jcallback();

  gpio_cb_t* cb();

  void Dispatched();

 protected:
  gpio_cb_t* __cb;
  JObject* _jcallback;
};


} // namespace iotjs

#endif /* IOTJS_GPIOCBWRAP_H */
