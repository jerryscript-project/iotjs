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

#define DBGLEV_ERR 1
#define DBGLEV_WARN 2
#define DBGLEV_INFO 3

typedef int (*iotjs_console_out_t)(int level, const char* format, ...);
extern iotjs_console_out_t iotjs_console_out;
extern void iotjs_set_console_out(iotjs_console_out_t output);

#ifdef ENABLE_DEBUG_LOG


extern int iotjs_debug_level;
extern FILE* iotjs_log_stream;
extern const char* iotjs_debug_prefix[4];

#if defined(__TIZEN__)
#include <dlog.h>
#define DLOG_TAG "IOTJS"
#define DLOG_PRINT(lvl, ...)                                       \
  dlog_print((lvl == DBGLEV_ERR                                    \
                  ? DLOG_ERROR                                     \
                  : (lvl == DBGLEV_WARN ? DLOG_WARN : DLOG_INFO)), \
             DLOG_TAG, __VA_ARGS__);
#else
#include <stdio.h>
#define DLOG_PRINT(lvl, ...)                                   \
  fprintf(iotjs_log_stream, "[%s] ", iotjs_debug_prefix[lvl]); \
  fprintf(iotjs_log_stream, __VA_ARGS__);                      \
  fprintf(iotjs_log_stream, "\n");                             \
  fflush(iotjs_log_stream);
#endif /* defined(__TIZEN__) */

#define IOTJS_DLOG(lvl, ...)                                        \
  do {                                                              \
    if (0 <= lvl && lvl <= iotjs_debug_level && iotjs_log_stream) { \
      if (iotjs_console_out) {                                      \
        iotjs_console_out(lvl, __VA_ARGS__);                        \
      } else {                                                      \
        DLOG_PRINT(lvl, __VA_ARGS__)                                \
      }                                                             \
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


void iotjs_debuglog_init(void);
void iotjs_debuglog_release(void);


#endif /* IOTJS_DEBUGLOG_H */
