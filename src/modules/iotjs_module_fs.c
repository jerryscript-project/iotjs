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

#include "iotjs_def.h"

#include "iotjs_module_buffer.h"
#include "iotjs_uv_request.h"

jerry_value_t make_stat_object(uv_stat_t* statbuf);


static jerry_value_t iotjs_create_uv_exception(int errorno,
                                               const char* syscall) {
  static char msg[256];
  snprintf(msg, sizeof(msg), "'%s' %s", syscall, uv_strerror(errorno));
  return iotjs_jval_create_error_without_error_flag(msg);
}


static void fs_after_async(uv_fs_t* req) {
  const jerry_value_t cb = *IOTJS_UV_REQUEST_JSCALLBACK(req);
  IOTJS_ASSERT(jerry_value_is_function(cb));

  jerry_value_t jargs[2] = { 0 };
  size_t jargc = 0;
  if (req->result < 0) {
    jerry_value_t jerror = iotjs_create_uv_exception(req->result, "open");
    jargs[jargc++] = jerror;
  } else {
    jargs[jargc++] = jerry_create_null();
    switch (req->fs_type) {
      case UV_FS_CLOSE: {
        break;
      }
      case UV_FS_OPEN:
      case UV_FS_READ:
      case UV_FS_WRITE: {
        jargs[jargc++] = jerry_create_number((double)req->result);
        break;
      }
      case UV_FS_SCANDIR: {
        int r;
        uv_dirent_t ent;
        uint32_t idx = 0;
        jargs[jargc++] = jerry_create_array(0);
        while ((r = uv_fs_scandir_next(req, &ent)) != UV_EOF) {
          jerry_value_t name =
              jerry_create_string((const jerry_char_t*)ent.name);
          iotjs_jval_set_property_by_index(jargs[1], idx, name);
          jerry_release_value(name);
          idx++;
        }
        break;
      }
      case UV_FS_FSTAT:
      case UV_FS_STAT: {
        uv_stat_t s = (req->statbuf);
        jargs[jargc++] = make_stat_object(&s);
        break;
      }
      default: { break; }
    }
  }

  iotjs_invoke_callback(cb, jerry_create_undefined(), jargs, jargc);

  jerry_release_value(jargs[0]);
  jerry_release_value(jargs[1]);
  uv_fs_req_cleanup(req);
  iotjs_uv_request_destroy((uv_req_t*)req);
}


static jerry_value_t fs_after_sync(uv_fs_t* req, int err,
                                   const char* syscall_name) {
  if (err < 0) {
    jerry_value_t jvalue = iotjs_create_uv_exception(err, syscall_name);
    jerry_value_t jerror = jerry_create_error_from_value(jvalue, true);
    return jerror;
  }

  switch (req->fs_type) {
    case UV_FS_CLOSE:
      break;
    case UV_FS_OPEN:
    case UV_FS_READ:
    case UV_FS_WRITE:
      return jerry_create_number(err);
    case UV_FS_FSTAT:
    case UV_FS_STAT: {
      uv_stat_t* s = &(req->statbuf);
      return make_stat_object(s);
    }
    case UV_FS_MKDIR:
    case UV_FS_RMDIR:
    case UV_FS_UNLINK:
    case UV_FS_RENAME:
      return jerry_create_undefined();
    case UV_FS_SCANDIR: {
      int r;
      uv_dirent_t ent;
      uint32_t idx = 0;
      jerry_value_t ret = jerry_create_array(0);
      while ((r = uv_fs_scandir_next(req, &ent)) != UV_EOF) {
        jerry_value_t name = jerry_create_string((const jerry_char_t*)ent.name);
        iotjs_jval_set_property_by_index(ret, idx, name);
        jerry_release_value(name);
        idx++;
      }
      return ret;
    }
    default: {
      IOTJS_ASSERT(false);
      break;
    }
  }
  return jerry_create_undefined();
}


static inline bool is_within_bounds(size_t off, size_t len, size_t max) {
  if (off >= max || max - off < len)
    return false;

  return true;
}


#define FS_ASYNC(env, syscall, pcallback, ...)                                \
  uv_fs_t* fs_req =                                                           \
      (uv_fs_t*)iotjs_uv_request_create(sizeof(uv_fs_t), pcallback, 0);       \
  int err = uv_fs_##syscall(iotjs_environment_loop(env), fs_req, __VA_ARGS__, \
                            fs_after_async);                                  \
  if (err < 0) {                                                              \
    fs_req->result = err;                                                     \
    fs_after_async(fs_req);                                                   \
  }                                                                           \
  ret_value = jerry_create_null();


#define FS_SYNC(env, syscall, ...)                                             \
  uv_fs_t fs_req;                                                              \
  int err = uv_fs_##syscall(iotjs_environment_loop(env), &fs_req, __VA_ARGS__, \
                            NULL);                                             \
  ret_value = fs_after_sync(&fs_req, err, #syscall);                           \
  uv_fs_req_cleanup(&fs_req);


JS_FUNCTION(fs_close) {
  DJS_CHECK_THIS();
  DJS_CHECK_ARGS(1, number);
  DJS_CHECK_ARG_IF_EXIST(1, function);

  const iotjs_environment_t* env = iotjs_environment_get();

  int fd = JS_GET_ARG(0, number);
  const jerry_value_t jcallback = JS_GET_ARG_IF_EXIST(1, function);

  jerry_value_t ret_value;
  if (!jerry_value_is_null(jcallback)) {
    FS_ASYNC(env, close, jcallback, fd);
  } else {
    FS_SYNC(env, close, fd);
  }
  return ret_value;
}


JS_FUNCTION(fs_open) {
  DJS_CHECK_THIS();
  DJS_CHECK_ARGS(3, string, number, number);
  DJS_CHECK_ARG_IF_EXIST(3, function);

  const iotjs_environment_t* env = iotjs_environment_get();

  iotjs_string_t path = JS_GET_ARG(0, string);
  int flags = JS_GET_ARG(1, number);
  int mode = JS_GET_ARG(2, number);
  const jerry_value_t jcallback = JS_GET_ARG_IF_EXIST(3, function);

  jerry_value_t ret_value;
  if (!jerry_value_is_null(jcallback)) {
    FS_ASYNC(env, open, jcallback, iotjs_string_data(&path), flags, mode);
  } else {
    FS_SYNC(env, open, iotjs_string_data(&path), flags, mode);
  }

  iotjs_string_destroy(&path);
  return ret_value;
}


typedef enum { IOTJS_FS_READ, IOTJS_FS_WRITE } iotjs_fs_op_t;

jerry_value_t fs_do_read_or_write(const jerry_value_t jfunc,
                                  const jerry_value_t jthis,
                                  const jerry_value_t jargv[],
                                  const jerry_length_t jargc,
                                  const iotjs_fs_op_t fs_op) {
  DJS_CHECK_THIS();
  DJS_CHECK_ARGS(5, number, object, number, number, number);
  DJS_CHECK_ARG_IF_EXIST(5, function);

  const iotjs_environment_t* env = iotjs_environment_get();

  int fd = JS_GET_ARG(0, number);
  const jerry_value_t jbuffer = JS_GET_ARG(1, object);
  size_t offset = JS_GET_ARG(2, number);
  size_t length = JS_GET_ARG(3, number);
  int position = JS_GET_ARG(4, number);
  const jerry_value_t jcallback = JS_GET_ARG_IF_EXIST(5, function);

  iotjs_bufferwrap_t* buffer_wrap = iotjs_bufferwrap_from_jbuffer(jbuffer);
  char* data = buffer_wrap->buffer;
  size_t data_length = iotjs_bufferwrap_length(buffer_wrap);
  DJS_CHECK(data != NULL && data_length > 0);

  if (!is_within_bounds(offset, length, data_length)) {
    return JS_CREATE_ERROR(RANGE, "length out of bound");
  }

  uv_buf_t uvbuf = uv_buf_init(data + offset, length);

  jerry_value_t ret_value;
  if (fs_op == IOTJS_FS_READ) {
    if (!jerry_value_is_null(jcallback)) {
      FS_ASYNC(env, read, jcallback, fd, &uvbuf, 1, position);
    } else {
      FS_SYNC(env, read, fd, &uvbuf, 1, position);
    }
  } else {
    if (!jerry_value_is_null(jcallback)) {
      FS_ASYNC(env, write, jcallback, fd, &uvbuf, 1, position);
    } else {
      FS_SYNC(env, write, fd, &uvbuf, 1, position);
    }
  }
  return ret_value;
}


JS_FUNCTION(fs_read) {
  return fs_do_read_or_write(jfunc, jthis, jargv, jargc, IOTJS_FS_READ);
}


JS_FUNCTION(fs_write) {
  return fs_do_read_or_write(jfunc, jthis, jargv, jargc, IOTJS_FS_WRITE);
}


jerry_value_t make_stat_object(uv_stat_t* statbuf) {
  const jerry_value_t fs = iotjs_module_get("fs");

  jerry_value_t stat_prototype =
      iotjs_jval_get_property(fs, IOTJS_MAGIC_STRING_STATS);
  IOTJS_ASSERT(jerry_value_is_object(stat_prototype));

  jerry_value_t jstat = jerry_create_object();
  iotjs_jval_set_prototype(jstat, stat_prototype);

  jerry_release_value(stat_prototype);


#define X(statobj, name) \
  iotjs_jval_set_property_number(statobj, #name, statbuf->st_##name);

  X(jstat, dev)
  X(jstat, mode)
  X(jstat, nlink)
  X(jstat, uid)
  X(jstat, gid)
  X(jstat, rdev)
  X(jstat, blksize)
  X(jstat, ino)
  X(jstat, size)
  X(jstat, blocks)

#undef X

  return jstat;
}


JS_FUNCTION(fs_stat) {
  DJS_CHECK_THIS();
  DJS_CHECK_ARGS(1, string);
  DJS_CHECK_ARG_IF_EXIST(1, function);

  const iotjs_environment_t* env = iotjs_environment_get();

  iotjs_string_t path = JS_GET_ARG(0, string);
  const jerry_value_t jcallback = JS_GET_ARG_IF_EXIST(1, function);

  jerry_value_t ret_value;
  if (!jerry_value_is_null(jcallback)) {
    FS_ASYNC(env, stat, jcallback, iotjs_string_data(&path));
  } else {
    FS_SYNC(env, stat, iotjs_string_data(&path));
  }

  iotjs_string_destroy(&path);
  return ret_value;
}


JS_FUNCTION(fs_fstat) {
  DJS_CHECK_THIS();
  DJS_CHECK_ARGS(1, number);
  DJS_CHECK_ARG_IF_EXIST(1, function);

  const iotjs_environment_t* env = iotjs_environment_get();

  int fd = JS_GET_ARG(0, number);
  const jerry_value_t jcallback = JS_GET_ARG_IF_EXIST(1, function);

  jerry_value_t ret_value;
  if (!jerry_value_is_null(jcallback)) {
    FS_ASYNC(env, fstat, jcallback, fd);
  } else {
    FS_SYNC(env, fstat, fd);
  }
  return ret_value;
}


JS_FUNCTION(fs_mkdir) {
  DJS_CHECK_THIS();
  DJS_CHECK_ARGS(2, string, number);
  DJS_CHECK_ARG_IF_EXIST(2, function);

  const iotjs_environment_t* env = iotjs_environment_get();

  iotjs_string_t path = JS_GET_ARG(0, string);
  int mode = JS_GET_ARG(1, number);
  const jerry_value_t jcallback = JS_GET_ARG_IF_EXIST(2, function);

  jerry_value_t ret_value;
  if (!jerry_value_is_null(jcallback)) {
    FS_ASYNC(env, mkdir, jcallback, iotjs_string_data(&path), mode);
  } else {
    FS_SYNC(env, mkdir, iotjs_string_data(&path), mode);
  }

  iotjs_string_destroy(&path);
  return ret_value;
}


JS_FUNCTION(fs_rmdir) {
  DJS_CHECK_THIS();
  DJS_CHECK_ARGS(1, string);
  DJS_CHECK_ARG_IF_EXIST(1, function);

  const iotjs_environment_t* env = iotjs_environment_get();

  iotjs_string_t path = JS_GET_ARG(0, string);
  const jerry_value_t jcallback = JS_GET_ARG_IF_EXIST(1, function);

  jerry_value_t ret_value;
  if (!jerry_value_is_null(jcallback)) {
    FS_ASYNC(env, rmdir, jcallback, iotjs_string_data(&path));
  } else {
    FS_SYNC(env, rmdir, iotjs_string_data(&path));
  }

  iotjs_string_destroy(&path);
  return ret_value;
}


JS_FUNCTION(fs_unlink) {
  DJS_CHECK_THIS();
  DJS_CHECK_ARGS(1, string);
  DJS_CHECK_ARG_IF_EXIST(1, function);

  const iotjs_environment_t* env = iotjs_environment_get();

  iotjs_string_t path = JS_GET_ARG(0, string);
  const jerry_value_t jcallback = JS_GET_ARG_IF_EXIST(1, function);

  jerry_value_t ret_value;
  if (!jerry_value_is_null(jcallback)) {
    FS_ASYNC(env, unlink, jcallback, iotjs_string_data(&path));
  } else {
    FS_SYNC(env, unlink, iotjs_string_data(&path));
  }

  iotjs_string_destroy(&path);
  return ret_value;
}


JS_FUNCTION(fs_rename) {
  DJS_CHECK_THIS();
  DJS_CHECK_ARGS(2, string, string);
  DJS_CHECK_ARG_IF_EXIST(2, function);

  const iotjs_environment_t* env = iotjs_environment_get();

  iotjs_string_t old_path = JS_GET_ARG(0, string);
  iotjs_string_t new_path = JS_GET_ARG(1, string);
  const jerry_value_t jcallback = JS_GET_ARG_IF_EXIST(2, function);

  jerry_value_t ret_value;
  if (!jerry_value_is_null(jcallback)) {
    FS_ASYNC(env, rename, jcallback, iotjs_string_data(&old_path),
             iotjs_string_data(&new_path));
  } else {
    FS_SYNC(env, rename, iotjs_string_data(&old_path),
            iotjs_string_data(&new_path));
  }

  iotjs_string_destroy(&old_path);
  iotjs_string_destroy(&new_path);
  return ret_value;
}


JS_FUNCTION(fs_read_dir) {
  DJS_CHECK_THIS();
  DJS_CHECK_ARGS(1, string);
  DJS_CHECK_ARG_IF_EXIST(1, function);

  const iotjs_environment_t* env = iotjs_environment_get();
  iotjs_string_t path = JS_GET_ARG(0, string);
  const jerry_value_t jcallback = JS_GET_ARG_IF_EXIST(1, function);

  jerry_value_t ret_value;
  if (!jerry_value_is_null(jcallback)) {
    FS_ASYNC(env, scandir, jcallback, iotjs_string_data(&path), 0);
  } else {
    FS_SYNC(env, scandir, iotjs_string_data(&path), 0);
  }
  iotjs_string_destroy(&path);
  return ret_value;
}

static jerry_value_t stats_is_typeof(jerry_value_t stats, int type) {
  jerry_value_t mode = iotjs_jval_get_property(stats, IOTJS_MAGIC_STRING_MODE);

  if (!jerry_value_is_number(mode)) {
    jerry_release_value(mode);
    return JS_CREATE_ERROR(TYPE, "fstat: file mode should be a number");
  }

  int mode_number = (int)iotjs_jval_as_number(mode);

  jerry_release_value(mode);

  return jerry_create_boolean((mode_number & S_IFMT) == type);
}

JS_FUNCTION(fs_stats_is_directory) {
  DJS_CHECK_THIS();
  jerry_value_t stats = JS_GET_THIS();
  return stats_is_typeof(stats, S_IFDIR);
}

JS_FUNCTION(fs_stats_is_file) {
  DJS_CHECK_THIS();
  jerry_value_t stats = JS_GET_THIS();
  return stats_is_typeof(stats, S_IFREG);
}

jerry_value_t iotjs_init_fs(void) {
  jerry_value_t fs = jerry_create_object();

  iotjs_jval_set_method(fs, IOTJS_MAGIC_STRING_CLOSE, fs_close);
  iotjs_jval_set_method(fs, IOTJS_MAGIC_STRING_OPEN, fs_open);
  iotjs_jval_set_method(fs, IOTJS_MAGIC_STRING_READ, fs_read);
  iotjs_jval_set_method(fs, IOTJS_MAGIC_STRING_WRITE, fs_write);
  iotjs_jval_set_method(fs, IOTJS_MAGIC_STRING_STAT, fs_stat);
  iotjs_jval_set_method(fs, IOTJS_MAGIC_STRING_FSTAT, fs_fstat);
  iotjs_jval_set_method(fs, IOTJS_MAGIC_STRING_MKDIR, fs_mkdir);
  iotjs_jval_set_method(fs, IOTJS_MAGIC_STRING_RMDIR, fs_rmdir);
  iotjs_jval_set_method(fs, IOTJS_MAGIC_STRING_UNLINK, fs_unlink);
  iotjs_jval_set_method(fs, IOTJS_MAGIC_STRING_RENAME, fs_rename);
  iotjs_jval_set_method(fs, IOTJS_MAGIC_STRING_READDIR, fs_read_dir);

  jerry_value_t stats_prototype = jerry_create_object();

  iotjs_jval_set_method(stats_prototype, IOTJS_MAGIC_STRING_ISDIRECTORY,
                        fs_stats_is_directory);
  iotjs_jval_set_method(stats_prototype, IOTJS_MAGIC_STRING_ISFILE,
                        fs_stats_is_file);

  iotjs_jval_set_property_jval(fs, IOTJS_MAGIC_STRING_STATS, stats_prototype);
  jerry_release_value(stats_prototype);

  return fs;
}
