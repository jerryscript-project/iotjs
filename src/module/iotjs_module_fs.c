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

#include "iotjs_module_fs.h"

#include "iotjs_module_buffer.h"

#include "iotjs_exception.h"
#include "iotjs_reqwrap.h"


iotjs_fs_reqwrap_t* iotjs_fs_reqwrap_create(const iotjs_jval_t* jcallback) {
  iotjs_fs_reqwrap_t* fs_reqwrap = IOTJS_ALLOC(iotjs_fs_reqwrap_t);
  IOTJS_VALIDATED_STRUCT_CONSTRUCTOR(iotjs_fs_reqwrap_t, fs_reqwrap);
  iotjs_reqwrap_initialize(&_this->reqwrap, jcallback, (uv_req_t*)&_this->req);
  return fs_reqwrap;
}


static void iotjs_fs_reqwrap_destroy(iotjs_fs_reqwrap_t* fs_reqwrap) {
  IOTJS_VALIDATED_STRUCT_DESTRUCTOR(iotjs_fs_reqwrap_t, fs_reqwrap);
  uv_fs_req_cleanup(&_this->req);
  iotjs_reqwrap_destroy(&_this->reqwrap);
  IOTJS_RELEASE(fs_reqwrap);
}


void iotjs_fs_reqwrap_dispatched(iotjs_fs_reqwrap_t* fs_reqwrap) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_fs_reqwrap_t, fs_reqwrap);
  iotjs_fs_reqwrap_destroy(fs_reqwrap);
}


uv_fs_t* iotjs_fs_reqwrap_req(iotjs_fs_reqwrap_t* fs_reqwrap) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_fs_reqwrap_t, fs_reqwrap);
  return &_this->req;
}

const iotjs_jval_t* iotjs_fs_reqwrap_jcallback(iotjs_fs_reqwrap_t* fs_reqwrap) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_fs_reqwrap_t, fs_reqwrap);
  return iotjs_reqwrap_jcallback(&_this->reqwrap);
}


iotjs_jval_t MakeStatObject(uv_stat_t* statbuf);


static void AfterAsync(uv_fs_t* req) {
  iotjs_fs_reqwrap_t* req_wrap = (iotjs_fs_reqwrap_t*)(req->data);
  IOTJS_ASSERT(req_wrap != NULL);
  IOTJS_ASSERT(iotjs_fs_reqwrap_req(req_wrap) == req);

  const iotjs_jval_t* cb = iotjs_fs_reqwrap_jcallback(req_wrap);
  IOTJS_ASSERT(iotjs_jval_is_function(cb));

  iotjs_jargs_t jarg = iotjs_jargs_create(2);
  if (req->result < 0) {
    iotjs_jval_t jerror = iotjs_create_uv_exception(req->result, "open");
    iotjs_jargs_append_jval(&jarg, &jerror);
    iotjs_jval_destroy(&jerror);
  } else {
    iotjs_jargs_append_null(&jarg);
    switch (req->fs_type) {
      case UV_FS_CLOSE: {
        break;
      }
      case UV_FS_OPEN:
      case UV_FS_READ:
      case UV_FS_WRITE: {
        iotjs_jargs_append_number(&jarg, req->result);
        break;
      }
      case UV_FS_SCANDIR: {
        int r;
        uv_dirent_t ent;
        uint32_t idx = 0;
        iotjs_jval_t ret = iotjs_jval_create_array(0);
        while ((r = uv_fs_scandir_next(req, &ent)) != UV_EOF) {
          iotjs_jval_t name = iotjs_jval_create_string_raw(ent.name);
          iotjs_jval_set_property_by_index(&ret, idx, &name);
          iotjs_jval_destroy(&name);
          idx++;
        }
        iotjs_jargs_append_jval(&jarg, &ret);
        iotjs_jval_destroy(&ret);
        break;
      }
      case UV_FS_STAT: {
        uv_stat_t s = (req->statbuf);
        iotjs_jval_t ret = MakeStatObject(&s);
        iotjs_jargs_append_jval(&jarg, &ret);
        iotjs_jval_destroy(&ret);
        break;
      }
      default: {
        iotjs_jargs_append_null(&jarg);
        break;
      }
    }
  }

  iotjs_make_callback(cb, iotjs_jval_get_undefined(), &jarg);

  iotjs_jargs_destroy(&jarg);
  iotjs_fs_reqwrap_dispatched(req_wrap);
}


static void AfterSync(uv_fs_t* req, int err, const char* syscall_name,
                      iotjs_jhandler_t* jhandler) {
  if (err < 0) {
    iotjs_jval_t jerror = iotjs_create_uv_exception(err, syscall_name);
    iotjs_jhandler_throw(jhandler, &jerror);
    iotjs_jval_destroy(&jerror);
  } else {
    switch (req->fs_type) {
      case UV_FS_CLOSE:
        break;
      case UV_FS_OPEN:
      case UV_FS_READ:
      case UV_FS_WRITE:
        iotjs_jhandler_return_number(jhandler, err);
        break;
      case UV_FS_STAT: {
        uv_stat_t* s = &(req->statbuf);
        iotjs_jval_t stat = MakeStatObject(s);
        iotjs_jhandler_return_jval(jhandler, &stat);
        iotjs_jval_destroy(&stat);
        break;
      }
      case UV_FS_MKDIR:
      case UV_FS_RMDIR:
      case UV_FS_UNLINK:
      case UV_FS_RENAME:
        iotjs_jhandler_return_undefined(jhandler);
        break;
      case UV_FS_SCANDIR: {
        int r;
        uv_dirent_t ent;
        uint32_t idx = 0;
        iotjs_jval_t ret = iotjs_jval_create_array(0);
        while ((r = uv_fs_scandir_next(req, &ent)) != UV_EOF) {
          iotjs_jval_t name = iotjs_jval_create_string_raw(ent.name);
          iotjs_jval_set_property_by_index(&ret, idx, &name);
          iotjs_jval_destroy(&name);
          idx++;
        }
        iotjs_jhandler_return_jval(jhandler, &ret);
        iotjs_jval_destroy(&ret);
        break;
      }
      default: {
        IOTJS_ASSERT(false);
        break;
      }
    }
  }
}


#define FS_ASYNC(env, syscall, pcallback, ...)                                \
  iotjs_fs_reqwrap_t* req_wrap = iotjs_fs_reqwrap_create(pcallback);          \
  uv_fs_t* fs_req = iotjs_fs_reqwrap_req(req_wrap);                           \
  int err = uv_fs_##syscall(iotjs_environment_loop(env), fs_req, __VA_ARGS__, \
                            AfterAsync);                                      \
  if (err < 0) {                                                              \
    fs_req->result = err;                                                     \
    AfterAsync(fs_req);                                                       \
  }                                                                           \
  iotjs_jhandler_return_null(jhandler);


#define FS_SYNC(env, syscall, ...)                                             \
  uv_fs_t fs_req;                                                              \
  int err = uv_fs_##syscall(iotjs_environment_loop(env), &fs_req, __VA_ARGS__, \
                            NULL);                                             \
  AfterSync(&fs_req, err, #syscall, jhandler);                                 \
  uv_fs_req_cleanup(&fs_req);


JHANDLER_FUNCTION(Close) {
  JHANDLER_CHECK_THIS(object);
  JHANDLER_CHECK_ARGS(1, number);
  JHANDLER_CHECK_ARG_IF_EXIST(1, function);

  const iotjs_environment_t* env = iotjs_environment_get();

  int fd = JHANDLER_GET_ARG(0, number);
  const iotjs_jval_t* jcallback = JHANDLER_GET_ARG_IF_EXIST(1, function);

  if (jcallback) {
    FS_ASYNC(env, close, jcallback, fd);
  } else {
    FS_SYNC(env, close, fd);
  }
}


JHANDLER_FUNCTION(Open) {
  JHANDLER_CHECK_THIS(object);
  JHANDLER_CHECK_ARGS(3, string, number, number);
  JHANDLER_CHECK_ARG_IF_EXIST(3, function);

  const iotjs_environment_t* env = iotjs_environment_get();

  iotjs_string_t path = JHANDLER_GET_ARG(0, string);
  int flags = JHANDLER_GET_ARG(1, number);
  int mode = JHANDLER_GET_ARG(2, number);
  const iotjs_jval_t* jcallback = JHANDLER_GET_ARG_IF_EXIST(3, function);

  if (jcallback) {
    FS_ASYNC(env, open, jcallback, iotjs_string_data(&path), flags, mode);
  } else {
    FS_SYNC(env, open, iotjs_string_data(&path), flags, mode);
  }

  iotjs_string_destroy(&path);
}


JHANDLER_FUNCTION(Read) {
  JHANDLER_CHECK_THIS(object);
  JHANDLER_CHECK_ARGS(5, number, object, number, number, number);
  JHANDLER_CHECK_ARG_IF_EXIST(5, function);

  const iotjs_environment_t* env = iotjs_environment_get();

  int fd = JHANDLER_GET_ARG(0, number);
  const iotjs_jval_t* jbuffer = JHANDLER_GET_ARG(1, object);
  int offset = JHANDLER_GET_ARG(2, number);
  int length = JHANDLER_GET_ARG(3, number);
  int position = JHANDLER_GET_ARG(4, number);
  const iotjs_jval_t* jcallback = JHANDLER_GET_ARG_IF_EXIST(5, function);

  iotjs_bufferwrap_t* buffer_wrap = iotjs_bufferwrap_from_jbuffer(jbuffer);
  char* data = iotjs_bufferwrap_buffer(buffer_wrap);
  int data_length = iotjs_bufferwrap_length(buffer_wrap);
  JHANDLER_CHECK(data != NULL);
  JHANDLER_CHECK(data_length > 0);

  if (offset >= data_length) {
    JHANDLER_THROW(RANGE, "offset out of bound");
    return;
  }
  if (offset + length > data_length) {
    JHANDLER_THROW(RANGE, "length out of bound");
    return;
  }

  uv_buf_t uvbuf = uv_buf_init(data + offset, length);

  if (jcallback) {
    FS_ASYNC(env, read, jcallback, fd, &uvbuf, 1, position);
  } else {
    FS_SYNC(env, read, fd, &uvbuf, 1, position);
  }
}


JHANDLER_FUNCTION(Write) {
  JHANDLER_CHECK_THIS(object);
  JHANDLER_CHECK_ARGS(5, number, object, number, number, number);
  JHANDLER_CHECK_ARG_IF_EXIST(5, function);

  const iotjs_environment_t* env = iotjs_environment_get();

  int fd = JHANDLER_GET_ARG(0, number);
  const iotjs_jval_t* jbuffer = JHANDLER_GET_ARG(1, object);
  int offset = JHANDLER_GET_ARG(2, number);
  int length = JHANDLER_GET_ARG(3, number);
  int position = JHANDLER_GET_ARG(4, number);
  const iotjs_jval_t* jcallback = JHANDLER_GET_ARG_IF_EXIST(5, function);

  iotjs_bufferwrap_t* buffer_wrap = iotjs_bufferwrap_from_jbuffer(jbuffer);
  char* data = iotjs_bufferwrap_buffer(buffer_wrap);
  int data_length = iotjs_bufferwrap_length(buffer_wrap);
  JHANDLER_CHECK(data != NULL);
  JHANDLER_CHECK(data_length > 0);

  if (offset >= data_length) {
    JHANDLER_THROW(RANGE, "offset out of bound");
    return;
  }
  if (offset + length > data_length) {
    JHANDLER_THROW(RANGE, "length out of bound");
    return;
  }

  uv_buf_t uvbuf = uv_buf_init(data + offset, length);

  if (jcallback) {
    FS_ASYNC(env, write, jcallback, fd, &uvbuf, 1, position);
  } else {
    FS_SYNC(env, write, fd, &uvbuf, 1, position);
  }
}


iotjs_jval_t MakeStatObject(uv_stat_t* statbuf) {
  const iotjs_jval_t* fs = iotjs_module_get(MODULE_FS);

  iotjs_jval_t create_stat = iotjs_jval_get_property(fs, "_createStat");
  IOTJS_ASSERT(iotjs_jval_is_function(&create_stat));

  iotjs_jval_t jstat = iotjs_jval_create_object();

#define X(statobj, name) \
  iotjs_jval_set_property_number(statobj, #name, statbuf->st_##name);

  X(&jstat, dev)
  X(&jstat, mode)
  X(&jstat, nlink)
  X(&jstat, uid)
  X(&jstat, gid)
  X(&jstat, rdev)
  X(&jstat, blksize)
  X(&jstat, ino)
  X(&jstat, size)
  X(&jstat, blocks)

#undef X

  iotjs_jargs_t jargs = iotjs_jargs_create(1);
  iotjs_jargs_append_jval(&jargs, &jstat);
  iotjs_jval_destroy(&jstat);

  iotjs_jval_t res =
      iotjs_jhelper_call_ok(&create_stat, iotjs_jval_get_undefined(), &jargs);

  iotjs_jargs_destroy(&jargs);
  iotjs_jval_destroy(&create_stat);

  return res;
}


JHANDLER_FUNCTION(Stat) {
  JHANDLER_CHECK_THIS(object);
  JHANDLER_CHECK_ARGS(1, string);
  JHANDLER_CHECK_ARG_IF_EXIST(1, function);

  const iotjs_environment_t* env = iotjs_environment_get();

  iotjs_string_t path = JHANDLER_GET_ARG(0, string);
  const iotjs_jval_t* jcallback = JHANDLER_GET_ARG_IF_EXIST(1, function);

  if (jcallback) {
    FS_ASYNC(env, stat, jcallback, iotjs_string_data(&path));
  } else {
    FS_SYNC(env, stat, iotjs_string_data(&path));
  }

  iotjs_string_destroy(&path);
}


JHANDLER_FUNCTION(MkDir) {
  JHANDLER_CHECK_THIS(object);
  JHANDLER_CHECK_ARGS(2, string, number);
  JHANDLER_CHECK_ARG_IF_EXIST(2, function);

  const iotjs_environment_t* env = iotjs_environment_get();

  iotjs_string_t path = JHANDLER_GET_ARG(0, string);
  int mode = JHANDLER_GET_ARG(1, number);
  const iotjs_jval_t* jcallback = JHANDLER_GET_ARG_IF_EXIST(2, function);

  if (jcallback) {
    FS_ASYNC(env, mkdir, jcallback, iotjs_string_data(&path), mode);
  } else {
    FS_SYNC(env, mkdir, iotjs_string_data(&path), mode);
  }

  iotjs_string_destroy(&path);
}


JHANDLER_FUNCTION(RmDir) {
  JHANDLER_CHECK_THIS(object);
  JHANDLER_CHECK_ARGS(1, string);
  JHANDLER_CHECK_ARG_IF_EXIST(1, function);

  const iotjs_environment_t* env = iotjs_environment_get();

  iotjs_string_t path = JHANDLER_GET_ARG(0, string);
  const iotjs_jval_t* jcallback = JHANDLER_GET_ARG_IF_EXIST(1, function);

  if (jcallback) {
    FS_ASYNC(env, rmdir, jcallback, iotjs_string_data(&path));
  } else {
    FS_SYNC(env, rmdir, iotjs_string_data(&path));
  }

  iotjs_string_destroy(&path);
}


JHANDLER_FUNCTION(Unlink) {
  JHANDLER_CHECK_THIS(object);
  JHANDLER_CHECK_ARGS(1, string);
  JHANDLER_CHECK_ARG_IF_EXIST(1, function);

  const iotjs_environment_t* env = iotjs_environment_get();

  iotjs_string_t path = JHANDLER_GET_ARG(0, string);
  const iotjs_jval_t* jcallback = JHANDLER_GET_ARG_IF_EXIST(1, function);

  if (jcallback) {
    FS_ASYNC(env, unlink, jcallback, iotjs_string_data(&path));
  } else {
    FS_SYNC(env, unlink, iotjs_string_data(&path));
  }

  iotjs_string_destroy(&path);
}


JHANDLER_FUNCTION(Rename) {
  JHANDLER_CHECK_THIS(object);
  JHANDLER_CHECK_ARGS(2, string, string);
  JHANDLER_CHECK_ARG_IF_EXIST(2, function);

  const iotjs_environment_t* env = iotjs_environment_get();

  iotjs_string_t oldPath = JHANDLER_GET_ARG(0, string);
  iotjs_string_t newPath = JHANDLER_GET_ARG(1, string);
  const iotjs_jval_t* jcallback = JHANDLER_GET_ARG_IF_EXIST(2, function);

  if (jcallback) {
    FS_ASYNC(env, rename, jcallback, iotjs_string_data(&oldPath),
             iotjs_string_data(&newPath));
  } else {
    FS_SYNC(env, rename, iotjs_string_data(&oldPath),
            iotjs_string_data(&newPath));
  }

  iotjs_string_destroy(&oldPath);
  iotjs_string_destroy(&newPath);
}


JHANDLER_FUNCTION(ReadDir) {
  JHANDLER_CHECK_THIS(object);
  JHANDLER_CHECK_ARGS(1, string);
  JHANDLER_CHECK_ARG_IF_EXIST(2, function);

  const iotjs_environment_t* env = iotjs_environment_get();
  iotjs_string_t path = JHANDLER_GET_ARG(0, string);
  const iotjs_jval_t* jcallback = JHANDLER_GET_ARG_IF_EXIST(2, function);

  if (jcallback) {
    FS_ASYNC(env, scandir, jcallback, iotjs_string_data(&path), 0);
  } else {
    FS_SYNC(env, scandir, iotjs_string_data(&path), 0);
  }
  iotjs_string_destroy(&path);
}


iotjs_jval_t InitFs() {
  iotjs_jval_t fs = iotjs_jval_create_object();

  iotjs_jval_set_method(&fs, "close", Close);
  iotjs_jval_set_method(&fs, "open", Open);
  iotjs_jval_set_method(&fs, "read", Read);
  iotjs_jval_set_method(&fs, "write", Write);
  iotjs_jval_set_method(&fs, "stat", Stat);
  iotjs_jval_set_method(&fs, "mkdir", MkDir);
  iotjs_jval_set_method(&fs, "rmdir", RmDir);
  iotjs_jval_set_method(&fs, "unlink", Unlink);
  iotjs_jval_set_method(&fs, "rename", Rename);
  iotjs_jval_set_method(&fs, "readdir", ReadDir);

  return fs;
}
