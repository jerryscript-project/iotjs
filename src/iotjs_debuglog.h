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

#ifndef IOTJS_DEBUGLOG_H
#define IOTJS_DEBUGLOG_H


#ifdef ENABLE_DEBUG_LOG

#include <stdio.h>

extern int iotjs_debug_level;
extern FILE* iotjs_log_stream;
extern const char* iotjs_debug_prefix[4];

#define DBGLEV_ERR 1
#define DBGLEV_WARN 2
#define DBGLEV_INFO 3

#define IOTJS_DLOG(lvl, ...)                                        \
  do {                                                              \
    if (0 <= lvl && lvl <= iotjs_debug_level && iotjs_log_stream) { \
      fprintf(iotjs_log_stream, "[%s] ", iotjs_debug_prefix[lvl]);  \
      fprintf(iotjs_log_stream, __VA_ARGS__);                       \
      fprintf(iotjs_log_stream, "\n");                              \
      fflush(iotjs_log_stream);                                     \
    }                                                               \
  } while (0)
#define DLOG(...) IOTJS_DLOG(DBGLEV_ERR, __VA_ARGS__)
#define DDLOG(...) IOTJS_DLOG(DBGLEV_WARN, __VA_ARGS__)
#define DDDLOG(...) IOTJS_DLOG(DBGLEV_INFO, __VA_ARGS__)

/*
  Use DLOG for errors, default you will see them
  Use DDLOG for warnings, set iotjs_debug_level=2 to see them
  USE DDDLOG for information, set iotjs_debug_level=3 to see them
*/

#else /* !ENABLE_DEBUG_LOG */

#define IOTJS_DLOG(...)
#define DLOG(...)
#define DDLOG(...)
#define DDDLOG(...)

#endif /* ENABLE_DEBUG_LOG */


void init_debug_settings();
void release_debug_settings();


#endif /* IOTJS_DEBUGLOG_H */
