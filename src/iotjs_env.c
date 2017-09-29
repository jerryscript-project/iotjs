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


static iotjs_environment_t current_env;
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
iotjs_environment_t* iotjs_environment_get() {
  if (!initialized) {
    iotjs_environment_initialize(&current_env);
    initialized = true;
  }
  return &current_env;
}


/**
 * Release the singleton instance of iotjs_environment_t.
 */
void iotjs_environment_release() {
  if (initialized) {
    iotjs_environment_destroy(&current_env);
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
  _this->config.debugger = false;
  _this->config.debugger_port = 5001;
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
    for (uint32_t i = 2; i < _this->argc; ++i) {
      iotjs_buffer_release(_this->argv[i]);
    }
    iotjs_buffer_release((char*)_this->argv);
  }
}


/**
 * Parse command line arguments
 */
bool iotjs_environment_parse_command_line_arguments(iotjs_environment_t* env,
                                                    uint32_t argc,
                                                    char** argv) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_environment_t, env);

  // Parse IoT.js command line arguments.
  uint32_t i = 1;
  uint8_t port_arg_len = strlen("--jerry-debugger-port=");
  while (i < argc && argv[i][0] == '-') {
    if (!strcmp(argv[i], "--memstat")) {
      _this->config.memstat = true;
    } else if (!strcmp(argv[i], "--show-opcodes")) {
      _this->config.show_opcode = true;
    } else if (!strcmp(argv[i], "--start-debug-server")) {
      _this->config.debugger = true;
    } else if (!strncmp(argv[i], "--jerry-debugger-port=", port_arg_len)) {
      size_t port_length = sizeof(strlen(argv[i] - port_arg_len - 1));
      char port[port_length];
      memcpy(&port, argv[i] + port_arg_len, port_length);
      sscanf(port, "%d", &(_this->config.debugger_port));
    } else {
      fprintf(stderr, "unknown command line option: %s\n", argv[i]);
      return false;
    }
    ++i;
  }

  // There must be at least one argument after processing the IoT.js args.
  if ((argc - i) < 1) {
    fprintf(stderr,
            "Usage: iotjs [options] {script | script.js} [arguments]\n");
    return false;
  }

  // Remaining arguments are for application.
  _this->argc = 2;
  size_t buffer_size = ((size_t)(_this->argc + argc - i)) * sizeof(char*);
  _this->argv = (char**)iotjs_buffer_allocate(buffer_size);
  _this->argv[0] = argv[0];
  _this->argv[1] = argv[i++];

  size_t len = 0;
  while (i < argc) {
    len = strlen(argv[i]) + 1;
    _this->argv[_this->argc] = iotjs_buffer_allocate(len);
    strncpy(_this->argv[_this->argc], argv[i], len);
    _this->argc++;
    i++;
  }

  return true;
}

uint32_t iotjs_environment_argc(const iotjs_environment_t* env) {
  const IOTJS_VALIDATED_STRUCT_METHOD(iotjs_environment_t, env);
  return _this->argc;
}


const char* iotjs_environment_argv(const iotjs_environment_t* env,
                                   uint32_t idx) {
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

bool iotjs_environment_is_exiting(iotjs_environment_t* env) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_environment_t, env);
  return _this->state == kExiting;
}
