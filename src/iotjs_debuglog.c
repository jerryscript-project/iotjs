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

#include <stdio.h>
#include <stdlib.h>

#include "iotjs_debuglog.h"

#ifdef ENABLE_DEBUG_LOG
int iotjs_debug_level = DBGLEV_ERR;
FILE* iotjs_log_stream;
const char* iotjs_debug_prefix[4] = { "", "ERR", "WRN", "INF" };
#endif // ENABLE_DEBUG_LOG


void init_debug_settings() {
#ifdef ENABLE_DEBUG_LOG
  const char* dbglevel = NULL;
  const char* dbglogfile = NULL;

#if defined(__LINUX__)
  dbglevel = getenv("IOTJS_DEBUG_LEVEL");
  dbglogfile = getenv("IOTJS_DEBUG_LOGFILE");
#endif // defined(__LINUX__)
  if (dbglevel) {
    iotjs_debug_level = atoi(dbglevel);
    if (iotjs_debug_level < 0)
      iotjs_debug_level = 0;
    if (iotjs_debug_level > DBGLEV_INFO)
      iotjs_debug_level = DBGLEV_INFO;
  }
  iotjs_log_stream = stderr;
  if (dbglogfile) {
    FILE* logstream;
    logstream = fopen(dbglogfile, "w+");
    if (logstream != NULL)
      iotjs_log_stream = logstream;
  }
// fprintf(stderr, "DBG LEV = %d", iotjs_debug_level);
// fprintf(stderr, "DBG OUT = %s", (dbglogfile==NULL?"(stderr)":dbglogfile));
#endif // ENABLE_DEBUG_LOG
}


void release_debug_settings() {
#ifdef ENABLE_DEBUG_LOG
  if (iotjs_log_stream != stderr || iotjs_log_stream != stdout) {
    fclose(iotjs_log_stream);
  }
  // some embed systems(ex, nuttx) may need this
  iotjs_log_stream = stderr;
  iotjs_debug_level = DBGLEV_ERR;
#endif // ENABLE_DEBUG_LOG
}
