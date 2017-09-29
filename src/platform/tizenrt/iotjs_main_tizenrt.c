/* Copyright 2017-present Samsung Electronics Co., Ltd. and other contributors
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

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <apps/shell/tash.h>
#include <tinyara/arch.h>
#include <tinyara/config.h>

#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "jerryscript-port.h"
#include "jerryscript.h"

#define USE_IOTJS_THREAD 1

/**
 * Aborts the program.
 */
void jerry_port_fatal(jerry_fatal_code_t code) {
  exit(1);
} /* jerry_port_fatal */

/**
 * Provide log message implementation for the engine.
 */
void jerry_port_log(jerry_log_level_t level, /**< log level */
                    const char *format,      /**< format string */
                    ...) {                   /**< parameters */
  /* Drain log messages since IoT.js has not support log levels yet. */
} /* jerry_port_log */

/**
 * Dummy function to get the time zone.
 *
 * @return true
 */
bool jerry_port_get_time_zone(jerry_time_zone_t *tz_p) {
  /* We live in UTC. */
  tz_p->offset = 0;
  tz_p->daylight_saving_time = 0;

  return true;
} /* jerry_port_get_time_zone */

/**
 * Dummy function to get the current time.
 *
 * @return 0
 */
double jerry_port_get_current_time(void) {
  return 0;
} /* jerry_port_get_current_time */

/**
 * Provide the implementation of jerryx_port_handler_print_char.
 * Uses 'printf' to print a single character to standard output.
 */
void jerryx_port_handler_print_char(char c) { /**< the character to print */
  // printf("%c", c);
} /* jerryx_port_handler_print_char */


/**
 * Compiler built-in setjmp function.
 *
 * @return 0 when called the first time
 *         1 when returns from a longjmp call
 */

int setjmp(jmp_buf buf) {
  return __builtin_setjmp(buf);
} /* setjmp */

/**
 * Compiler built-in longjmp function.
 *
 * Note:
 *   ignores value argument
 */

void longjmp(jmp_buf buf, int value) {
  /* Must be called with 1. */
  __builtin_longjmp(buf, 1);
} /* longjmp */

int iotjs_entry(int argc, char **argv);
int tuv_cleanup(void);


#if USE_IOTJS_THREAD
struct iotjs_thread_arg {
  int argc;
  char **argv;
};

pthread_addr_t iotjs_thread(void *thread_arg) {
  struct iotjs_thread_arg *arg = thread_arg;

#ifdef CONFIG_DEBUG_VERBOSE
  int ret = iotjs_entry(arg->argc, arg->argv);
  printf("IoT.js Result: %d\n", ret);
#else
  iotjs_entry(arg->argc, arg->argv);
#endif
  tuv_cleanup();

  sleep(1);
  return NULL;
}

int iotjs(int argc, char *argv[]) {
  pthread_attr_t attr;
  int status;
  struct sched_param sparam;
  pthread_t tid = (pthread_t)0;
  struct iotjs_thread_arg arg;

  status = pthread_attr_init(&attr);
  if (status != 0) {
    printf("fail to initialize iotjs thread\n");
    return -1;
  }

  sparam.sched_priority = CONFIG_IOTJS_PRIORITY;
  status = pthread_attr_setschedparam(&attr, &sparam);
  status = pthread_attr_setschedpolicy(&attr, SCHED_RR);
  status = pthread_attr_setstacksize(&attr, CONFIG_IOTJS_STACKSIZE);

  arg.argc = argc;
  arg.argv = argv;

  status = pthread_create(&tid, &attr, iotjs_thread, &arg);
  if (status < 0) {
    printf("fail to start iotjs thread\n");
    return -1;
  }
  pthread_setname_np(tid, "iotjs_thread");
  pthread_join(tid, NULL);

  return 0;
}

#else

static int iotjs(int argc, char *argv[]) {
  int ret = 0;
  ret = iotjs_entry(argc, argv);
#ifdef CONFIG_DEBUG_VERBOSE
  printf("IoT.js Result: %d\n", ret);
#endif
  tuv_cleanup();
  return ret;
}

#endif

int iotjs_register_cmds(void) {
  tash_cmd_install("iotjs", iotjs, TASH_EXECMD_SYNC);
  return 0;
}
