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

static void initialize(iotjs_environment_t* env);

/**
 * Get the singleton instance of iotjs_environment_t.
 */
iotjs_environment_t* iotjs_environment_get() {
  if (!initialized) {
    initialize(&current_env);
    initialized = true;
  }
  return &current_env;
}


/**
 * Release the singleton instance of iotjs_environment_t, and debugger config.
 */
void iotjs_environment_release() {
  if (!initialized)
    return;

  IOTJS_VALIDATED_STRUCT_DESTRUCTOR(iotjs_environment_t,
                                    iotjs_environment_get());
  if (_this->config.debugger)
    iotjs_buffer_release((char*)(_this->config.debugger));
  if (_this->argv)
    iotjs_buffer_release((char*)_this->argv);
  initialized = false;
}


static void initialize(iotjs_environment_t* env) {
  IOTJS_VALIDATED_STRUCT_CONSTRUCTOR(iotjs_environment_t, env);

  _this->argc = 0;
  _this->argv = NULL;
  _this->loop = NULL;
  _this->state = kInitializing;
  _this->config.memstat = false;
  _this->config.show_opcode = false;
  _this->config.debugger = NULL;
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
      _this->config.debugger =
          (DebuggerConfig*)iotjs_buffer_allocate(sizeof(DebuggerConfig));
      _this->config.debugger->port = 5001;
      _this->config.debugger->wait_source = false;
      _this->config.debugger->context_reset = false;
    } else if (!strncmp(argv[i], "--jerry-debugger-port=", port_arg_len) &&
               _this->config.debugger) {
      size_t port_length = sizeof(strlen(argv[i] - port_arg_len - 1));
      char port[port_length];
      memcpy(&port, argv[i] + port_arg_len, port_length);
      sscanf(port, "%hu", &(_this->config.debugger->port));
    } else if (!strcmp(argv[i], "--debugger-wait-source") &&
               _this->config.debugger) {
      _this->config.debugger->wait_source = true;
    } else {
      fprintf(stderr, "unknown command line option: %s\n", argv[i]);
      return false;
    }
    ++i;
  }

  // If IoT.js is waiting for source from the debugger client,
  // Further processing over command line argument is not needed.
  if (_this->config.debugger && _this->config.debugger->wait_source)
    return true;

  // There must be at least one argument after processing the IoT.js args,
  if (argc - i < 1) {
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

  // Clonning for argv is not required.
  // 1) We will only read
  // 2) Standard C guarantees that strings pointed by the argv array shall
  //    retain between program startup and program termination
  while (i < argc)
    _this->argv[_this->argc++] = argv[i++];

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


void iotjs_environment_set_state(iotjs_environment_t* env, State s) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_environment_t, env);
  switch (s) {
    case kInitializing:
      break;
    case kRunningMain:
      IOTJS_ASSERT(_this->state == kInitializing);
      break;
    case kRunningLoop:
      IOTJS_ASSERT(_this->state == kRunningMain);
      break;
    case kExiting:
      IOTJS_ASSERT(_this->state < kExiting);
      break;
    default:
      IOTJS_ASSERT(!"Should not reach here.");
  }
  _this->state = s;
}

bool iotjs_environment_is_exiting(iotjs_environment_t* env) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_environment_t, env);
  return _this->state == kExiting;
}
