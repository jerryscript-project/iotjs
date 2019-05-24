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

#ifndef IOTJS_ENV_H
#define IOTJS_ENV_H

#include "uv.h"

#ifdef JERRY_DEBUGGER
typedef struct {
  bool wait_source;
  bool context_reset;
  uint16_t port;
  char channel[16];
  char protocol[16];
  char serial_config[64];
} DebuggerConfig;
#endif

typedef struct {
  uint32_t memstat : 1;
  uint32_t show_opcode : 1;
#ifdef JERRY_DEBUGGER
  DebuggerConfig* debugger;
#endif
} Config;

typedef enum {
  kInitializing,
  kRunningMain,
  kRunningLoop,
  kExiting,
} State;


typedef struct {
  // Number of application arguments including 'iotjs' and app name.
  uint32_t argc;

  // Application arguments list including 'iotjs' and app name.
  char** argv;

  // I/O event loop.
  uv_loop_t* loop;

  // Running state.
  State state;

  // Run config
  Config config;

  // Exitcode
  uint8_t exitcode;
} iotjs_environment_t;


iotjs_environment_t* iotjs_environment_get(void);
void iotjs_environment_release(void);

bool iotjs_environment_parse_command_line_arguments(iotjs_environment_t* env,
                                                    uint32_t argc, char** argv);

uint32_t iotjs_environment_argc(const iotjs_environment_t* env);
const char* iotjs_environment_argv(const iotjs_environment_t* env,
                                   uint32_t idx);

uv_loop_t* iotjs_environment_loop(const iotjs_environment_t* env);
void iotjs_environment_set_loop(iotjs_environment_t* env, uv_loop_t* loop);

const Config* iotjs_environment_config(const iotjs_environment_t* env);
#ifdef JERRY_DEBUGGER
const DebuggerConfig* iotjs_environment_dconfig(const iotjs_environment_t* env);
#endif

void iotjs_environment_set_state(iotjs_environment_t* env, State s);
bool iotjs_environment_is_exiting(iotjs_environment_t* env);

#endif /* IOTJS_ENV_H */
