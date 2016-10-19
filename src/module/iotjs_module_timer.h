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

#ifndef IOTJS_MODULE_TIMER_H
#define IOTJS_MODULE_TIMER_H

#include "iotjs_binding.h"
#include "iotjs_handlewrap.h"

namespace iotjs {

JObject* InitTimer();

class TimerWrap : public HandleWrap {
 public:
  explicit TimerWrap(Environment* env, JObject& jtimer)
      : HandleWrap(jtimer, reinterpret_cast<uv_handle_t*>(&_handle))
      , _jcallback(NULL) {
    // Initialze timer handler.
    uv_timer_init(env->loop(), &_handle);
  }

  // Timer timeout callback handler.
  void OnTimeout();

  // Timer close callback handler.
  void OnClose();

  // Start timer.
  int Start(int64_t timeout, int64_t repeat, JObject& jcallback);

  // Stop & close timer.
  int Stop();

  // Retrieve javascript callback function.
  JObject* jcallback() { return _jcallback; }

  uv_timer_t handle() { return _handle; }

 protected:
  // timer handle.
  uv_timer_t _handle;

  // Javascript callback function.
  JObject* _jcallback;
};

} // namespace iotjs

#endif /* IOTJS_MODULE_TIMER_H */
