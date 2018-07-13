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

#include <stdlib.h>
#include <string.h>

typedef enum {
  OPT_HELP,
  OPT_MEM_STATS,
  OPT_SHOW_OP,
  OPT_DEBUG_SERVER,
  OPT_DEBUGGER_WAIT_SOURCE,
  OPT_DEBUG_PORT,
  NUM_OF_OPTIONS
} cli_option_id_t;

typedef struct {
  const cli_option_id_t id;
  const char* opt;
  const char* longopt;
  const char* help;
  const uint32_t more; // The number of options coming with the given option
} cli_option_t;

#define CLI_DEFAULT_HELP_STRING \
  "Usage: iotjs [options] {FILE | FILE.js} [arguments]\n"

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
  IOTJS_RELEASE(env->config.debugger);
  IOTJS_RELEASE(env->argv);
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
  env->exitcode = 0;
}


/**
 * Parse command line arguments
 */
bool iotjs_environment_parse_command_line_arguments(iotjs_environment_t* env,
                                                    uint32_t argc,
                                                    char** argv) {
  // declare options
  const cli_option_t opts[] = {
    {
        .id = OPT_HELP,
        .opt = "h",
        .longopt = "help",
        .help = "print this help and exit",
    },
    {
        .id = OPT_MEM_STATS,
        .longopt = "mem-stats",
        .help = "dump memory statistics",
    },
    {
        .id = OPT_SHOW_OP,
        .longopt = "show-opcodes",
        .help = "dump parser byte-code",
    },
    {
        .id = OPT_DEBUG_SERVER,
        .opt = "d",
        .longopt = "start-debug-server",
        .help = "start debug server and wait for a connecting client",
    },
    {
        .id = OPT_DEBUGGER_WAIT_SOURCE,
        .opt = "w",
        .longopt = "debugger-wait-source",
        .help = "wait for an executable source from the client",
    },
    {
        .id = OPT_DEBUG_PORT,
        .longopt = "debug-port",
        .more = 1,
        .help = "debug server port (default: 5001)",
    },
  };

  const cli_option_t* cur_opt;
  uint32_t i = 1;

  while (i < argc && argv[i][0] == '-') {
    cur_opt = NULL;

    // check if the known option is given.
    for (uint32_t k = 0; k < NUM_OF_OPTIONS; k++) {
      if ((opts[k].opt && !strcmp(&argv[i][1], opts[k].opt)) ||
          (opts[k].longopt && !strcmp(&argv[i][2], opts[k].longopt))) {
        cur_opt = &opts[k];
        break;
      }
    }

    if (cur_opt == NULL) {
      fprintf(stderr, "unknown command line option: %s\n", argv[i]);
      return false;
    }

    switch (cur_opt->id) {
      case OPT_HELP: {
        fprintf(stderr, "%s\n  Options:\n\n", CLI_DEFAULT_HELP_STRING);
        for (uint32_t k = 0; k < NUM_OF_OPTIONS; k++) {
          if (opts[k].opt) {
            fprintf(stderr, "    -%s, --%-21s %s\n", opts[k].opt,
                    opts[k].longopt, opts[k].help);
          } else {
            fprintf(stderr, "    --%-25s %s\n", opts[k].longopt, opts[k].help);
          }
        }
        fprintf(stderr, "\n");
        return false;
      } break;
      case OPT_MEM_STATS: {
        env->config.memstat = true;
      } break;
      case OPT_SHOW_OP: {
        env->config.show_opcode = true;
      } break;
      case OPT_DEBUG_SERVER: {
        if (!env->config.debugger) {
          env->config.debugger =
              (DebuggerConfig*)iotjs_buffer_allocate(sizeof(DebuggerConfig));
        }
        env->config.debugger->port = 5001;
        env->config.debugger->wait_source = false;
        env->config.debugger->context_reset = false;
      } break;
      case OPT_DEBUG_PORT: {
        if (env->config.debugger) {
          char* pos = NULL;
          env->config.debugger->port = (uint16_t)strtoul(argv[i + 1], &pos, 10);
        }
      } break;
      case OPT_DEBUGGER_WAIT_SOURCE: {
        if (env->config.debugger)
          env->config.debugger->wait_source = true;
      } break;
      default:
        break;
    }

    // increase index of argv
    i += (1 + cur_opt->more);
  }

  // If IoT.js is waiting for source from the debugger client,
  // Further processing over command line argument is not needed.
  if (env->config.debugger && env->config.debugger->wait_source)
    return true;

  // There must be at least one argument after processing the IoT.js args,
  if (argc - i < 1) {
    fprintf(stderr, CLI_DEFAULT_HELP_STRING);
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
