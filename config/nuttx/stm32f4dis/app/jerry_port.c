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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "jerryscript-ext/handler.h"
#include "jerryscript-port.h"
#include "jerryscript.h"

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
  printf("%c", c);
} /* jerryx_port_handler_print_char */
