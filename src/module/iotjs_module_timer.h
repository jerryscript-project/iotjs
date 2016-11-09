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


class TimerWrap {
 public:
  explicit TimerWrap(const iotjs_jval_t* jtimer) {
    iotjs_handlewrap_initialize(&_handlewrap, jtimer,
            reinterpret_cast<uv_handle_t*>(&_handle), (uintptr_t)this, Delete);

    // Initialize timer handler.
    const iotjs_environment_t* env = iotjs_environment_get();
    uv_timer_init(iotjs_environment_loop(env), &_handle);
  }

  ~TimerWrap() {
    iotjs_handlewrap_destroy(&_handlewrap);
  }

  // Timer timeout callback handler.
  void OnTimeout();

  // Timer close callback handler.
  void OnClose();

  // Start timer.
  int Start(int64_t timeout, int64_t repeat);

  // Stop & close timer.
  int Stop();

  uv_timer_t handle() { return _handle; }

  iotjs_jval_t* jobject() {
    return iotjs_handlewrap_jobject(&_handlewrap);
  }

  static void Delete(const uintptr_t data) {
    delete ((TimerWrap*)data);
  }

 protected:
  iotjs_handlewrap_t _handlewrap;

  // timer handle.
  uv_timer_t _handle;
};


} // namespace iotjs


#endif /* IOTJS_MODULE_TIMER_H */
