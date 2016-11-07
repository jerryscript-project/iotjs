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


class TimerWrap : public HandleWrap {
 public:
  explicit TimerWrap(const iotjs_environment_t* env, const iotjs_jval_t* jtimer)
      : HandleWrap(jtimer, reinterpret_cast<uv_handle_t*>(&_handle)) {
    // Initialize timer handler.
    uv_timer_init(iotjs_environment_loop(env), &_handle);
    _jcallback = *iotjs_jval_get_undefined();
  }

  // Timer timeout callback handler.
  void OnTimeout();

  // Timer close callback handler.
  void OnClose();

  // Start timer.
  int Start(int64_t timeout, int64_t repeat, const iotjs_jval_t* jcallback);

  // Stop & close timer.
  int Stop();

  // Retrieve javascript callback function.
  const iotjs_jval_t* jcallback() { return &_jcallback; }

  uv_timer_t handle() { return _handle; }

 protected:
  // timer handle.
  uv_timer_t _handle;

  // Javascript callback function.
  iotjs_jval_t _jcallback;
};

} // namespace iotjs

#endif /* IOTJS_MODULE_TIMER_H */
