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

#include "iotjs_def.h"
#include "iotjs_objectwrap.h"
#include "iotjs_reqwrap.h"
#include "../../iotjs_module_buffer.h"
#include "../../iotjs_module_fs.h"
#include "../common.h"
#include "../structures.h"

iotjs_jval_t callback_for_open;
iotjs_jval_t callback_for_close;
iotjs_jval_t callback_for_read;
iotjs_jval_t callback_for_write;
iotjs_jval_t callback_for_unlink;

static FILE *default_open(const char *path, const char *mode) {
  iotjs_jargs_t jarg = iotjs_jargs_create(2);
  iotjs_jval_t js_path = iotjs_jval_create_string_raw(path);
  iotjs_jval_t js_mode = iotjs_jval_create_string_raw(mode);

  iotjs_jargs_append_jval(&jarg, &js_path);
  iotjs_jargs_append_jval(&jarg, &js_mode);

  iotjs_jval_t iotjs_result =
      iotjs_make_callback_with_result(&callback_for_open,
                                      iotjs_jval_get_undefined(), &jarg);

  int test = -1;
  if (iotjs_jval_as_number(&iotjs_result) >= 0) {
    test = iotjs_jval_as_number(&iotjs_result);
  }

  iotjs_jargs_destroy(&jarg);
  iotjs_jval_destroy(&js_path);
  iotjs_jval_destroy(&js_mode);
  iotjs_jval_destroy(&iotjs_result);
  FILE *fp = fdopen(test, mode);
  return fp;
}

static size_t default_read(void *ptr, size_t size, size_t count, FILE *stream) {
  size_t size_read = 0;
  size_t total_size = size * count;
  if (total_size <= UINT32_MAX) {
    iotjs_jargs_t jarg = iotjs_jargs_create(3);
    int fp = fileno(stream);
    iotjs_jval_t js_fp = iotjs_jval_create_number((double)fp);
    iotjs_jval_t js_total_size = iotjs_jval_create_number(total_size);
    iotjs_jval_t js_buffer = iotjs_bufferwrap_create_buffer(total_size);
    iotjs_jargs_append_jval(&jarg, &js_buffer);
    iotjs_jargs_append_jval(&jarg, &js_total_size);
    iotjs_jargs_append_jval(&jarg, &js_fp);
    iotjs_jval_t iotjs_result =
        iotjs_make_callback_with_result(&callback_for_read,
                                        iotjs_jval_get_undefined(), &jarg);
    size_read = (size_t)iotjs_jval_as_number(&iotjs_result);
    iotjs_bufferwrap_t *buf = iotjs_bufferwrap_create(&js_buffer, total_size);
    char *buffer = iotjs_bufferwrap_buffer(buf);
    memcpy(ptr, buffer, total_size);

    iotjs_jargs_destroy(&jarg);
    iotjs_jval_destroy(&js_fp);
    iotjs_jval_destroy(&js_buffer);
    iotjs_jval_destroy(&js_total_size);
    iotjs_jval_destroy(&iotjs_result);
  }
  return size_read;
}

static size_t default_write(const void *ptr, size_t size, size_t count,
                            FILE *stream) {
  size_t size_written = 0;
  size_t total_size = size * count;
  if (total_size <= UINT32_MAX) {
    iotjs_jargs_t jarg = iotjs_jargs_create(3);
    int fp = fileno(stream);
    iotjs_jval_t js_fp = iotjs_jval_create_number((double)fp);
    iotjs_jval_t js_total_size = iotjs_jval_create_number(total_size);
    iotjs_jval_t js_buffer = iotjs_bufferwrap_create_buffer(total_size);

    iotjs_jargs_append_jval(&jarg, &js_buffer);
    iotjs_jargs_append_jval(&jarg, &js_total_size);
    iotjs_jargs_append_jval(&jarg, &js_fp);

    iotjs_jval_t iotjs_result =
        iotjs_make_callback_with_result(&callback_for_write,
                                        iotjs_jval_get_undefined(), &jarg);

    iotjs_jval_destroy(&js_fp);
    iotjs_jval_destroy(&js_total_size);
    iotjs_jval_destroy(&js_buffer);
    iotjs_jargs_destroy(&jarg);
    size_written = (size_t)iotjs_jval_as_number(&iotjs_result);
    iotjs_jval_destroy(&iotjs_result);
  }
  return size_written;
}

static int default_close(FILE *stream) {
  int result = -1;

  iotjs_jargs_t jarg = iotjs_jargs_create(1);
  int fp = fileno(stream);
  iotjs_jval_t js_fp = iotjs_jval_create_number((double)fp);
  iotjs_jargs_append_jval(&jarg, &js_fp);

  iotjs_jval_t iotjs_result =
      iotjs_make_callback_with_result(&callback_for_close,
                                      iotjs_jval_get_undefined(), &jarg);

  result = (int)iotjs_jval_as_number(&iotjs_result);

  iotjs_jargs_destroy(&jarg);
  iotjs_jval_destroy(&iotjs_result);
  iotjs_jval_destroy(&js_fp);
  return result;
}

static int default_unlink(const char *path) {
  int result = -1;

  iotjs_jargs_t jarg = iotjs_jargs_create(1);
  iotjs_jval_t js_path = iotjs_jval_create_string_raw(path);

  iotjs_jargs_append_jval(&jarg, &js_path);

  iotjs_jval_t iotjs_result =
      iotjs_make_callback_with_result(&callback_for_unlink,
                                      iotjs_jval_get_undefined(), &jarg);

  result = (int)iotjs_jval_as_number(&iotjs_result);

  iotjs_jargs_destroy(&jarg);
  iotjs_jval_destroy(&iotjs_result);
  iotjs_jval_destroy(&js_path);
  return result;
}

static OCPersistentStorage ps_callbacks = { default_open, default_read,
                                            default_write, default_close,
                                            default_unlink };

#define VALIDATE_MEMBER(js_object, member_name)                 \
  iotjs_jval_t member_##member_name =                           \
      iotjs_jval_get_property(js_object, #member_name);         \
  if (!iotjs_jval_is_function(&member_##member_name)) {         \
    iotjs_jval_t error_##member_name = iotjs_jval_create_error( \
        "Persistent storage callback must be a function");      \
    iotjs_jhandler_throw(jhandler, &error_##member_name);       \
    return;                                                     \
  }                                                             \
  iotjs_jval_destroy(&member_##member_name);
