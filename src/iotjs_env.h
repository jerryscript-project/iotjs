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

#ifndef IOTJS_ENV_H
#define IOTJS_ENV_H

#include "uv.h"

#include "iotjs_def.h"


namespace iotjs {


class Environment {
 public:
  enum State {
    kInitializing,
    kRunningMain,
    kRunningLoop,
    kExiting
  };

  static Environment* GetEnv() {
    if (Environment::_env == NULL) {
      Environment::_env = new Environment();
    }
    return _env;
  }

  void Init(int argc, char** argv, uv_loop_t* loop) {
    _argc = argc;
    _argv = argv;
    _loop = loop;
  }

  int argc() {return _argc; }

  char** argv() { return _argv; }

  uv_loop_t* loop() { return _loop; }

  State state() { return _state; }

  void GoStateRunningMain() {
    IOTJS_ASSERT(_state == kInitializing);
    _state = kRunningMain;
  }

  void GoStateRunningLoop() {
    IOTJS_ASSERT(_state == kRunningMain);
    _state = kRunningLoop;
  }

  void GoStateExiting() {
    IOTJS_ASSERT(_state < kExiting);
    _state = kExiting;
  }

 private:
  int _argc;
  char** _argv;
  uv_loop_t* _loop;

  State _state;


 private:
  Environment()
      : _argc(0)
      , _argv(NULL)
      , _loop(NULL)
      , _state(kInitializing) {
  }

  static Environment* _env;
}; // class Environment

} // namespace iotjs

#endif /* IOTJS_ENV_H */
