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


struct Config {
  bool memstat;
  bool show_opcode;
};


class Environment {
 public:
  enum State {
    kInitializing,
    kRunningMain,
    kRunningLoop,
    kExiting
  };

  /**
   * Get the singleton instance of Environment.
   */
  static Environment* GetEnv() {
    if (Environment::_env == NULL) {
      Environment::_env = new Environment();
    }
    return _env;
  }

  /**
   * Release the singleton instance of Environment.
   */
  static void Release() {
    if (Environment::_env) {
      delete Environment::_env;
      Environment::_env = NULL;
    }
  }

  /**
   * Parse command line arguments
   */
  bool ParseCommandLineArgument(int argc, char** argv);

  int argc() {return _argc; }

  char** argv() { return _argv; }

  uv_loop_t* loop() { return _loop; }
  void set_loop(uv_loop_t* loop) { _loop = loop; }

  State state() { return _state; }

  const Config* config() { return &_config;}

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
  // Number of application arguments including 'iotjs' and app name.
  int _argc;

  // Application arguments list including 'iotjs' and app name.
  char** _argv;

  // I/O event loop.
  uv_loop_t* _loop;

  // Running state.
  State _state;

  // Run config
  Config _config;


 private:
  /**
   * Constructor on private section.
   *  To prevent create an instance of Environment.
   *  The only way to create an instance of Environment is by using GetEnv().
   */
  Environment();

  ~Environment();

  // The singleton instance of Environment.
  static Environment* _env;
}; // class Environment

} // namespace iotjs

#endif /* IOTJS_ENV_H */
