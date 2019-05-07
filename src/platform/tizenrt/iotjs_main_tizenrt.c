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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef JERRY_DEBUGGER
#include <time.h>
#endif /* JERRY_DEBUGGER */

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
 * Dummy function to get local time zone adjustment, in milliseconds,
 * for the given timestamp.
 */
double jerry_port_get_local_time_zone_adjustment(double unix_ms, bool is_utc) {
  (void)unix_ms;
  (void)is_utc;
  return 0.0;
} /* jerry_port_get_local_time_zone_adjustment */

/**
 * Get system time
 *
 * @return milliseconds since Unix epoch
 */
double jerry_port_get_current_time(void) {
  struct timespec ts;

  /* Get the current time */
  int ret = clock_gettime(CLOCK_REALTIME, &ts);
  if (ret < 0) {
    return 0.0;
  }

  return ((double)ts.tv_sec) * 1000.0 + ((double)ts.tv_nsec) / 1000000.0;
} /* jerry_port_get_current_time */

/**
 * Provide the implementation of jerryx_port_handler_print_char.
 * Uses 'printf' to print a single character to standard output.
 */
void jerryx_port_handler_print_char(char c) { /**< the character to print */
  // printf("%c", c);
} /* jerryx_port_handler_print_char */

/**
 * Determines the size of the given file.
 * @return size of the file
 */
static size_t jerry_port_get_file_size(FILE *file_p) /**< opened file */
{
  fseek(file_p, 0, SEEK_END);
  long size = ftell(file_p);
  fseek(file_p, 0, SEEK_SET);

  return (size_t)size;
} /* jerry_port_get_file_size */

/**
 * Opens file with the given path and reads its source.
 * @return the source of the file
 */
uint8_t *jerry_port_read_source(const char *file_name_p, /**< file name */
                                size_t *out_size_p) /**< [out] read bytes */
{
  FILE *file_p = fopen(file_name_p, "rb");

  if (file_p == NULL) {
    jerry_port_log(JERRY_LOG_LEVEL_ERROR, "Error: failed to open file: %s\n",
                   file_name_p);
    return NULL;
  }

  size_t file_size = jerry_port_get_file_size(file_p);
  uint8_t *buffer_p = (uint8_t *)malloc(file_size);

  if (buffer_p == NULL) {
    fclose(file_p);

    jerry_port_log(JERRY_LOG_LEVEL_ERROR,
                   "Error: failed to allocate memory for module");
    return NULL;
  }

  size_t bytes_read = fread(buffer_p, 1u, file_size, file_p);

  if (!bytes_read) {
    fclose(file_p);
    free(buffer_p);

    jerry_port_log(JERRY_LOG_LEVEL_ERROR, "Error: failed to read file: %s\n",
                   file_name_p);
    return NULL;
  }

  fclose(file_p);
  *out_size_p = bytes_read;

  return buffer_p;
} /* jerry_port_read_source */

/**
 * Release the previously opened file's content.
 */
void jerry_port_release_source(uint8_t *buffer_p) /**< buffer to free */
{
  free(buffer_p);
} /* jerry_port_release_source */

/**
 * Normalize a file path
 *
 * @return length of the path written to the output buffer
 */
size_t jerry_port_normalize_path(
    const char *in_path_p, /**< input file path */
    char *out_buf_p,       /**< output buffer */
    size_t out_buf_size)   /**< size of output buffer */
{
  size_t len = strlen(in_path_p);
  if (len + 1 > out_buf_size) {
    return 0;
  }

  /* Return the original string. */
  strcpy(out_buf_p, in_path_p);
  return len;
} /* jerry_port_normalize_path */

#ifdef JERRY_DEBUGGER
void jerry_port_sleep(uint32_t sleep_time) {
  nanosleep(
      &(const struct timespec){
          (time_t)sleep_time / 1000,
          ((long int)sleep_time % 1000) * 1000000L /* Seconds, nanoseconds */
      },
      NULL);
} /* jerry_port_sleep */
#endif /* JERRY_DEBUGGER */

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
