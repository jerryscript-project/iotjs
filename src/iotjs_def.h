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


#ifndef IOTJS_DEF_H
#define IOTJS_DEF_H


#ifndef IOTJS_MAX_READ_BUFFER_SIZE
#if defined(__NUTTX__) || defined(__TIZENRT__)
#define IOTJS_MAX_READ_BUFFER_SIZE 1023
#define IOTJS_MAX_PATH_SIZE 120
#else /* !__NUTTX__ && !__TIZENRT__ */
#define IOTJS_MAX_READ_BUFFER_SIZE 65535
#define IOTJS_MAX_PATH_SIZE PATH_MAX
#endif /* __NUTTX__ || TIZENRT */
#endif /* IOTJS_MAX_READ_BUFFER_SIZE */


#ifndef IOTJS_ASSERT
#ifdef NDEBUG
#define IOTJS_ASSERT(x) ((void)(x))
#else /* !NDEBUG */
extern void print_stacktrace();
extern void force_terminate();
#define IOTJS_ASSERT(x)                                                      \
  do {                                                                       \
    if (!(x)) {                                                              \
      fprintf(stderr, "%s:%d: Assertion '%s' failed.\n", __FILE__, __LINE__, \
              #x);                                                           \
      print_stacktrace();                                                    \
      force_terminate();                                                     \
    }                                                                        \
  } while (0)
#endif /* NDEBUG */
#endif /* IOTJS_ASSERT */

#if defined(__arm__)
#define TARGET_ARCH "arm"
#elif defined(__i686__)
#define TARGET_ARCH "ia32"
#elif defined(__x86_64__)
#define TARGET_ARCH "x64"
#else /* !__arm__ && !__i686__ && !__x86_64__ */
#define TARGET_ARCH "unknown"
#endif /* __arm__ */


#if defined(__linux__)
#if defined(__TIZEN__)
#define TARGET_OS "tizen"
#else
#define TARGET_OS "linux"
#endif /* __TIZEN__ */
#elif defined(__NUTTX__)
#define TARGET_OS "nuttx"
#elif defined(__APPLE__)
#define TARGET_OS "darwin"
#elif defined(__TIZENRT__)
#define TARGET_OS "tizenrt"
#elif defined(WIN32)
#define TARGET_OS "windows"
#else /* !__linux__ && !__NUTTX__ !__APPLE__ && !__TIZENRT__ && !WIN32 */
#define TARGET_OS "unknown"
#endif /* __linux__ */

#define IOTJS_VERSION "1.0.0"

#if !defined(STRINGIFY)
#define STRINGIFY(x) #x
#endif /* STRINGIFY */

#if !defined(TOSTRING)
#define TOSTRING(x) STRINGIFY(x)
#endif /* TOSTRING */

#if !defined(TARGET_BOARD)
#define TARGET_BOARD "unknown"
#endif /* TARGET_BOARD */

/* Avoid compiler warnings if needed. */
#define IOTJS_UNUSED(x) ((void)(x))

#define IOTJS_DEFINE_NATIVE_HANDLE_INFO_THIS_MODULE(name)                  \
  static void iotjs_##name##_destroy(iotjs_##name##_t* wrap);              \
  static const jerry_object_native_info_t this_module_native_info = {      \
    .free_cb = (jerry_object_native_free_callback_t)iotjs_##name##_destroy \
  }

#include <uv.h>
#include <assert.h>
#include <limits.h> /* PATH_MAX */
#include <stdbool.h>
#include <string.h>

// commonly used header files
#include "iotjs_binding.h"
#include "iotjs_binding_helper.h"
#include "iotjs_debuglog.h"
#include "iotjs_env.h"
#include "iotjs_magic_strings.h"
#include "iotjs_module.h"
#include "iotjs_string.h"
#include "iotjs_util.h"


#endif /* IOTJS_DEF_H */
