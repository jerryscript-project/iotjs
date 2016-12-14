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
#ifdef __NUTTX__
#define IOTJS_MAX_READ_BUFFER_SIZE 1023
#define IOTJS_MAX_PATH_SIZE 120
#else
#define IOTJS_MAX_READ_BUFFER_SIZE 65535
#define IOTJS_MAX_PATH_SIZE PATH_MAX
#endif
#endif


#ifndef IOTJS_ASSERT
#ifdef NDEBUG
#define IOTJS_ASSERT(x) ((void)(x))
#else
#define IOTJS_ASSERT(x) assert(x)
#endif
#endif


#if defined(__ARM__)
#define TARGET_ARCH "arm"
#elif defined(__i686__)
#define TARGET_ARCH "ia32"
#elif defined(__x86_64__)
#define TARGET_ARCH "x64"
#else
#define TARGET_ARCH "unknown"
#endif


#if defined(__LINUX__)
#define TARGET_OS "linux"
#elif defined(__NUTTX__)
#define TARGET_OS "nuttx"
#elif defined(__DARWIN__)
#define TARGET_OS "darwin"
#else
#define TARGET_OS "unknown"
#endif

#if !defined(STRINGIFY)
#define STRINGIFY(x) #x
#endif

#if !defined(TOSTRING)
#define TOSTRING(x) STRINGIFY(x)
#endif


#if !defined(TARGET_BOARD)
#define TARGET_BOARD "unknown"
#endif


#define IOTJS_VALID_MAGIC_SEQUENCE 0xfee1c001   /* feel cool */
#define IOTJS_INVALID_MAGIC_SEQUENCE 0xfee1badd /* feel bad */

#define IOTJS_DECLARE_THIS(iotjs_classname_t, x) \
  iotjs_classname_t##_impl_t* _this = &(x)->unsafe;


#ifdef NDEBUG

#define IOTJS_VALIDATED_STRUCT(iotjs_classname_t) \
  iotjs_classname_t##_impl_t;                     \
  typedef struct iotjs_classname_t {              \
    iotjs_classname_t##_impl_t unsafe;            \
  } iotjs_classname_t;

#define IOTJS_VALIDATED_STRUCT_STATIC_INITIALIZER(...) __VA_ARGS__

#define IOTJS_VALIDATED_STRUCT_CONSTRUCTOR(iotjs_classname_t, x) \
  IOTJS_DECLARE_THIS(iotjs_classname_t, x);
#define IOTJS_VALIDATED_STRUCT_DESTRUCTOR(iotjs_classname_t, x) \
  IOTJS_DECLARE_THIS(iotjs_classname_t, x);
#define IOTJS_VALIDATED_STRUCT_METHOD(iotjs_classname_t, x) \
  IOTJS_DECLARE_THIS(iotjs_classname_t, x);

#else

#define IOTJS_VALIDATED_STRUCT(iotjs_classname_t) \
  iotjs_classname_t##_impl_t;                     \
  typedef struct iotjs_classname_t {              \
    iotjs_classname_t##_impl_t unsafe;            \
    uint32_t flag_create;                         \
    char* valgrind_tracer;                        \
  } iotjs_classname_t;

#define IOTJS_VALIDATED_STRUCT_STATIC_INITIALIZER(...) \
  { IOTJS_VALID_MAGIC_SEQUENCE, iotjs_buffer_allocate(4), __VA_ARGS__ }

#define IOTJS_VALIDATE_FLAG(iotjs_classname_t, x)                         \
  if ((x)->flag_create != IOTJS_VALID_MAGIC_SEQUENCE) {                   \
    DLOG("`%s %s` is not initialized properly.", #iotjs_classname_t, #x); \
    IOTJS_ASSERT(false);                                                  \
  }

#define IOTJS_VALIDATED_STRUCT_CONSTRUCTOR(iotjs_classname_t, x)      \
  IOTJS_DECLARE_THIS(iotjs_classname_t, x);                           \
  /* IOTJS_ASSERT((x)->flag_create != IOTJS_VALID_MAGIC_SEQUENCE); */ \
  (x)->flag_create = IOTJS_VALID_MAGIC_SEQUENCE;                      \
  (x)->valgrind_tracer = iotjs_buffer_allocate(4);

#define IOTJS_VALIDATED_STRUCT_DESTRUCTOR(iotjs_classname_t, x) \
  IOTJS_DECLARE_THIS(iotjs_classname_t, x);                     \
  IOTJS_VALIDATE_FLAG(iotjs_classname_t, x);                    \
  (x)->flag_create = IOTJS_INVALID_MAGIC_SEQUENCE;              \
  iotjs_buffer_release((x)->valgrind_tracer);

#define IOTJS_VALIDATED_STRUCT_METHOD(iotjs_classname_t, x) \
  IOTJS_DECLARE_THIS(iotjs_classname_t, x);                 \
  IOTJS_VALIDATE_FLAG(iotjs_classname_t, x);

#endif

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
#include "iotjs_module.h"
#include "iotjs_string.h"
#include "iotjs_util.h"


#endif /* IOTJS_DEF_H */
