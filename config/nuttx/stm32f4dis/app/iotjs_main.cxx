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

#include <nuttx/config.h>
#include <nuttx/arch.h>

#include <stdio.h>
#include <setjmp.h>

#ifdef CONFIG_IOTJS
# if !defined(CONFIG_HAVE_CXX) || !defined(CONFIG_HAVE_CXXINITIALIZE)
#   error Need CONFIG_HAVE_CXX and CONFIG_HAVE_CXXINITIALIZE
# endif
#endif

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/**
 * Compiler built-in setjmp function.
 *
 * @return 0 when called the first time
 *         1 when returns from a longjmp call
 */

int
setjmp (jmp_buf buf)
{
    return __builtin_setjmp (buf);
} /* setjmp */

/**
 * Compiler built-in longjmp function.
 *
 * Note:
 *   ignores value argument
 */

void
longjmp (jmp_buf buf, int value)
{
    /* Must be called with 1. */
    __builtin_longjmp (buf, 1);
} /* longjmp */

extern "C" int iotjs_entry(int argc, char *argv[]);
extern "C" int tuv_cleanup(void);

#ifdef CONFIG_BUILD_KERNEL
extern "C" int main(int argc, FAR char *argv[])
#else
extern "C" int iotjs_main(int argc, char *argv[])
#endif
{
  int ret = 0;
  up_cxxinitialize();
  ret = iotjs_entry(argc, argv);
  tuv_cleanup();
  return ret;
}
