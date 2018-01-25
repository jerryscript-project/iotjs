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

  iotjs_environment_t* env = iotjs_environment_get();
  if (env->config.debugger)
    iotjs_buffer_release((char*)(env->config.debugger));
  if (env->argv)
    iotjs_buffer_release((char*)env->argv);
  initialized = false;
}


static void initialize(iotjs_environment_t* env) {
  env->argc = 0;
  env->argv = NULL;
  env->loop = NULL;
  env->state = kInitializing;
  env->config.memstat = false;
  env->config.show_opcode = false;
  env->config.debugger = NULL;
}


/**
 * Parse command line arguments
 */
bool iotjs_environment_parse_command_line_arguments(iotjs_environment_t* env,
                                                    uint32_t argc,
                                                    char** argv) {
  // Parse IoT.js command line arguments.
  uint32_t i = 1;
  uint8_t port_arg_len = strlen("--jerry-debugger-port=");
  while (i < argc && argv[i][0] == '-') {
    if (!strcmp(argv[i], "--memstat")) {
      env->config.memstat = true;
    } else if (!strcmp(argv[i], "--show-opcodes")) {
      env->config.show_opcode = true;
    } else if (!strcmp(argv[i], "--start-debug-server")) {
      env->config.debugger =
          (DebuggerConfig*)iotjs_buffer_allocate(sizeof(DebuggerConfig));
      env->config.debugger->port = 5001;
      env->config.debugger->wait_source = false;
      env->config.debugger->context_reset = false;
    } else if (!strncmp(argv[i], "--jerry-debugger-port=", port_arg_len) &&
               env->config.debugger) {
      size_t port_length = sizeof(strlen(argv[i] - port_arg_len - 1));
      char port[port_length];
      memcpy(&port, argv[i] + port_arg_len, port_length);
      sscanf(port, "%hu", &env->config.debugger->port);
    } else if (!strcmp(argv[i], "--debugger-wait-source") &&
               env->config.debugger) {
      env->config.debugger->wait_source = true;
    } else {
      fprintf(stderr, "unknown command line option: %s\n", argv[i]);
      return false;
    }
    ++i;
  }

  // If IoT.js is waiting for source from the debugger client,
  // Further processing over command line argument is not needed.
  if (env->config.debugger && env->config.debugger->wait_source)
    return true;

  // There must be at least one argument after processing the IoT.js args,
  if (argc - i < 1) {
    fprintf(stderr,
            "Usage: iotjs [options] {script | script.js} [arguments]\n");
    return false;
  }

  // Remaining arguments are for application.
  env->argc = 2;
  size_t buffer_size = ((size_t)(env->argc + argc - i)) * sizeof(char*);
  env->argv = (char**)iotjs_buffer_allocate(buffer_size);
  env->argv[0] = argv[0];
  env->argv[1] = argv[i++];

  // Clonning for argv is not required.
  // 1) We will only read
  // 2) Standard C guarantees that strings pointed by the argv array shall
  //    retain between program startup and program termination
  while (i < argc)
    env->argv[env->argc++] = argv[i++];

  return true;
}

uint32_t iotjs_environment_argc(const iotjs_environment_t* env) {
  return env->argc;
}


const char* iotjs_environment_argv(const iotjs_environment_t* env,
                                   uint32_t idx) {
  return env->argv[idx];
}


uv_loop_t* iotjs_environment_loop(const iotjs_environment_t* env) {
  return env->loop;
}


void iotjs_environment_set_loop(iotjs_environment_t* env, uv_loop_t* loop) {
  env->loop = loop;
}


const Config* iotjs_environment_config(const iotjs_environment_t* env) {
  return &env->config;
}


void iotjs_environment_set_state(iotjs_environment_t* env, State s) {
  switch (s) {
    case kInitializing:
      break;
    case kRunningMain:
      IOTJS_ASSERT(env->state == kInitializing);
      break;
    case kRunningLoop:
      IOTJS_ASSERT(env->state == kRunningMain);
      break;
    case kExiting:
      IOTJS_ASSERT(env->state < kExiting);
      break;
    default:
      IOTJS_ASSERT(!"Should not reach here.");
  }
  env->state = s;
}

bool iotjs_environment_is_exiting(iotjs_environment_t* env) {
  return env->state == kExiting;
}
