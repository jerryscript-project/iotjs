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

#include "iotjs_env.h"

#include <string.h>


namespace iotjs {


/**
 * Construct an instance of Environment.
 */
Environment::Environment()
    : _argc(0)
    , _argv(NULL)
    , _loop(NULL)
    , _state(kInitializing) {
  _config.memstat = false;
  _config.show_opcode = false;
}


/**
 * Release an instance of Environment.
 */
Environment::~Environment() {
  if (_argv) {
    // release command line argument strings.
    // _argv[0] and _argv[1] refer addresses in static memory space.
    // Ohters refer adresses in heap space that is need to be deallocated.
    for (int i = 2; i < _argc; ++i) {
      delete _argv[i];
    }
    delete _argv;
  }
}

/**
 * Parse command line arguments
 */
bool Environment::ParseCommandLineArgument(int argc, char** argv) {
  // There must be at least two arguemnts.
  if (argc < 2) {
    fprintf(stderr,
            "usage: iotjs <js> [<iotjs arguments>] [-- <app arguments>]\n");
    return false;
  }

  // Second argument should be IoT.js application.
  char* app = argv[1];
  _argc = 2;

  // Parse IoT.js command line arguments.
  int i = 2;
  while (i < argc) {
    if (!strcmp(argv[i], "--")) {
      ++i;
      break;
    }
    if (!strcmp(argv[i], "--memstat")) {
      _config.memstat = true;
    } else if (!strcmp(argv[i], "--show-opcodes")) {
      _config.show_opcode = true;
    } else {
      fprintf(stderr, "unknown command line argument %s\n", argv[i]);
      return false;
    }
    ++i;
  }

  // Remaining arguments are for application.
  _argv = new char*[_argc + argc - i];
  _argv[0] = argv[0];
  _argv[1] = argv[1];
  while (i < argc) {
    _argv[_argc] = new char[strlen(argv[i]) + 1];
    strcpy(_argv[_argc], argv[i]);
    _argc++;
    i++;
  }

  return true;
}


} // namespace iotjs
