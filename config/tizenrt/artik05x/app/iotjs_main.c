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

/****************************************************************************
 *   Copyright (C) 2013 Gregory Nutt. All rights reserved.
 *   Author: Gregory Nutt <gnutt@nuttx.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name NuttX nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <apps/shell/tash.h>
#include <tinyara/arch.h>
#include <tinyara/config.h>

#include <setjmp.h>
#include <stdio.h>

#define USE_IOTJS_THREAD 1

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
  int ret = 0;

  ret = iotjs_entry(arg->argc, arg->argv);
  tuv_cleanup();

  sleep(1);
  printf("iotjs thread end\n");
  return NULL;
}

int iotjs(int argc, char *argv[]) {
  pthread_attr_t attr;
  int status;
  struct sched_param sparam;
  pthread_t tid;
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
  tuv_cleanup();
  return ret;
}

#endif

int iotjs_register_cmds(void) {
  tash_cmd_install("iotjs", iotjs, TASH_EXECMD_SYNC);
  return 0;
}

#ifdef CONFIG_BUILD_KERNEL
int main(int argc, FAR char *argv[])
#else
int iotjs_main(int argc, char *argv[])
#endif
{
  return iotjs_register_cmds();
}
