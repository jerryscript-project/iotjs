/* Copyright 2015-present Samsung Electronics Co., Ltd. and other contributors
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


#include "iotjs_def.h"
#include "iotjs_env.h"

#include <string.h>


static iotjs_environment_t env;
static bool initialized = false;


/**
 * Constructor/Destructor on private section.
 * To prevent create an instance of iotjs_environment_t.
 * The only way to get an instance of environment is iotjs_environment_get()
 */


static void iotjs_environment_initialize(iotjs_environment_t* env);
static void iotjs_environment_destroy(iotjs_environment_t* env);


/**
 * Get the singleton instance of iotjs_environment_t.
 */
const iotjs_environment_t* iotjs_environment_get() {
  if (!initialized) {
    iotjs_environment_initialize(&env);
    initialized = true;
  }
  return &env;
}


/**
 * Release the singleton instance of iotjs_environment_t.
 */
void iotjs_environment_release() {
  if (initialized) {
    iotjs_environment_destroy(&env);
    initialized = false;
  }
}


/**
 * Initialize an instance of iotjs_environment_t.
 */
static void iotjs_environment_initialize(iotjs_environment_t* env) {
  IOTJS_VALIDATED_STRUCT_CONSTRUCTOR(iotjs_environment_t, env);

  _this->argc = 0;
  _this->argv = NULL;
  _this->loop = NULL;
  _this->state = kInitializing;
  _this->config.memstat = false;
  _this->config.show_opcode = false;
}


/**
 * Destroy an instance of iotjs_environment_t.
 */
static void iotjs_environment_destroy(iotjs_environment_t* env) {
  IOTJS_VALIDATED_STRUCT_DESTRUCTOR(iotjs_environment_t, env);
  if (_this->argv) {
    // release command line argument strings.
    // _argv[0] and _argv[1] refer addresses in static memory space.
    // Others refer addresses in heap space that is need to be deallocated.
    int i;
    for (i = 2; i < _this->argc; ++i) {
      iotjs_buffer_release(_this->argv[i]);
    }
    iotjs_buffer_release((char*)_this->argv);
  }
}


/**
 * Parse command line arguments
 */
bool iotjs_environment_parse_command_line_arguments(iotjs_environment_t* env,
                                                    int argc, char** argv) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_environment_t, env);

  // There must be at least two arguments.
  if (argc < 2) {
    fprintf(stderr,
            "usage: iotjs <js> [<iotjs arguments>] [-- <app arguments>]\n");
    return false;
  }

  // Parse IoT.js command line arguments.
  int i = 2;
  while (i < argc) {
    if (!strcmp(argv[i], "--")) {
      ++i;
      break;
    }
    if (!strcmp(argv[i], "--memstat")) {
      _this->config.memstat = true;
    } else if (!strcmp(argv[i], "--show-opcodes")) {
      _this->config.show_opcode = true;
    } else {
      fprintf(stderr, "unknown command line argument %s\n", argv[i]);
      return false;
    }
    ++i;
  }

  // Remaining arguments are for application.
  _this->argc = 2;
  size_t buffer_size = (_this->argc + argc - i) * sizeof(char*);
  _this->argv = (char**)iotjs_buffer_allocate(buffer_size);
  _this->argv[0] = argv[0];
  _this->argv[1] = argv[1];
  while (i < argc) {
    _this->argv[_this->argc] = iotjs_buffer_allocate(strlen(argv[i]) + 1);
    strcpy(_this->argv[_this->argc], argv[i]);
    _this->argc++;
    i++;
  }

  return true;
}

int iotjs_environment_argc(const iotjs_environment_t* env) {
  const IOTJS_VALIDATED_STRUCT_METHOD(iotjs_environment_t, env);
  return _this->argc;
}


const char* iotjs_environment_argv(const iotjs_environment_t* env, int idx) {
  const IOTJS_VALIDATED_STRUCT_METHOD(iotjs_environment_t, env);
  return _this->argv[idx];
}


uv_loop_t* iotjs_environment_loop(const iotjs_environment_t* env) {
  const IOTJS_VALIDATED_STRUCT_METHOD(iotjs_environment_t, env);
  return _this->loop;
}


void iotjs_environment_set_loop(iotjs_environment_t* env, uv_loop_t* loop) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_environment_t, env);
  _this->loop = loop;
}


const Config* iotjs_environment_config(const iotjs_environment_t* env) {
  const IOTJS_VALIDATED_STRUCT_METHOD(iotjs_environment_t, env);
  return &_this->config;
}


void iotjs_environment_go_state_running_main(iotjs_environment_t* env) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_environment_t, env);

  IOTJS_ASSERT(_this->state == kInitializing);
  _this->state = kRunningMain;
}


void iotjs_environment_go_state_running_loop(iotjs_environment_t* env) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_environment_t, env);

  IOTJS_ASSERT(_this->state == kRunningMain);
  _this->state = kRunningLoop;
}


void iotjs_environment_go_state_exiting(iotjs_environment_t* env) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_environment_t, env);
  IOTJS_ASSERT(_this->state < kExiting);
  _this->state = kExiting;
}
