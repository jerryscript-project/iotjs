/* Copyright 2015-2016 Samsung Electronics Co., Ltd.
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

#include "iotjs_exception.h"
#include "iotjs_reqwrap.h"


namespace iotjs {


class FsReqWrap : public ReqWrap<uv_fs_t> {
 public:
  FsReqWrap(const iotjs_jval_t* jcallback) : ReqWrap<uv_fs_t>(jcallback) {
  }

  ~FsReqWrap() {
    uv_fs_req_cleanup(&_req);
  }
};


iotjs_jval_t MakeStatObject(uv_stat_t* statbuf);


static void After(uv_fs_t* req) {
  FsReqWrap* req_wrap = static_cast<FsReqWrap*>(req->data);
  IOTJS_ASSERT(req_wrap != NULL);
  IOTJS_ASSERT(req_wrap->req() == req);

  const iotjs_jval_t* cb = req_wrap->jcallback();
  IOTJS_ASSERT(iotjs_jval_is_function(cb));

  iotjs_jargs_t jarg = iotjs_jargs_create(2);
  if (req->result < 0) {
    iotjs_jval_t jerror = CreateUVException(req->result, "open");
    iotjs_jargs_append_jval(&jarg, &jerror);
    iotjs_jval_destroy(&jerror);
  } else {
    iotjs_jargs_append_null(&jarg);
    switch (req->fs_type) {
      case UV_FS_CLOSE:
      {
        break;
      }
      case UV_FS_OPEN:
      case UV_FS_READ:
      case UV_FS_WRITE:
      {
        iotjs_jargs_append_number(&jarg, req->result);
        break;
      }
      case UV_FS_SCANDIR:
      {
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

  MakeCallback(cb, iotjs_jval_get_undefined(), &jarg);

  iotjs_jargs_destroy(&jarg);
  delete req_wrap;
}


#define FS_ASYNC(env, syscall, pcallback, ...) \
  FsReqWrap* req_wrap = new FsReqWrap(pcallback); \
  uv_fs_t* fs_req = req_wrap->req(); \
  int err = uv_fs_ ## syscall(env->loop(), \
                              fs_req, \
                              __VA_ARGS__, \
                              After); \
  if (err < 0) { \
    fs_req->result = err; \
    After(fs_req); \
  } \
  iotjs_jhandler_return_null(jhandler);


#define FS_SYNC(env, syscall, ...) \
  FsReqWrap req_wrap(iotjs_jval_get_null()); \
  int err = uv_fs_ ## syscall(env->loop(), \
                              req_wrap.req(), \
                              __VA_ARGS__, \
                              NULL); \
  if (err < 0) { \
    iotjs_jval_t jerror = CreateUVException(err, #syscall); \
    iotjs_jhandler_throw(jhandler, &jerror); \
    iotjs_jval_destroy(&jerror); \
  }


JHANDLER_FUNCTION(Close) {
  JHANDLER_CHECK_THIS(object);
  JHANDLER_CHECK_ARGS(1, number);
  JHANDLER_CHECK_ARG_IF_EXIST(1, function);

  Environment* env = Environment::GetEnv();

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

  Environment* env = Environment::GetEnv();

  iotjs_string_t path = JHANDLER_GET_ARG(0, string);
  int flags = JHANDLER_GET_ARG(1, number);
  int mode = JHANDLER_GET_ARG(2, number);
  const iotjs_jval_t* jcallback = JHANDLER_GET_ARG_IF_EXIST(3, function);

  if (jcallback) {
    FS_ASYNC(env, open, jcallback, iotjs_string_data(&path), flags, mode);
  } else {
    FS_SYNC(env, open, iotjs_string_data(&path), flags, mode);
    if (err >= 0)
      iotjs_jhandler_return_number(jhandler, err);
  }

  iotjs_string_destroy(&path);
}


JHANDLER_FUNCTION(Read) {
  JHANDLER_CHECK_THIS(object);
  JHANDLER_CHECK_ARGS(5, number, object, number, number, number);
  JHANDLER_CHECK_ARG_IF_EXIST(5, function);

  Environment* env = Environment::GetEnv();

  int fd = JHANDLER_GET_ARG(0, number);
  const iotjs_jval_t* jbuffer = JHANDLER_GET_ARG(1, object);
  int offset = JHANDLER_GET_ARG(2, number);
  int length = JHANDLER_GET_ARG(3, number);
  int position = JHANDLER_GET_ARG(4, number);
  const iotjs_jval_t* jcallback = JHANDLER_GET_ARG_IF_EXIST(5, function);

  BufferWrap* buffer_wrap = BufferWrap::FromJBuffer(jbuffer);
  char* data = buffer_wrap->buffer();
  int data_length = buffer_wrap->length();
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

  uv_buf_t uvbuf = uv_buf_init(reinterpret_cast<char*>(data + offset),
                               length);

  if (jcallback) {
    FS_ASYNC(env, read, jcallback, fd, &uvbuf, 1, position);
  } else {
    FS_SYNC(env, read, fd, &uvbuf, 1, position);
    if (err >= 0)
      iotjs_jhandler_return_number(jhandler, err);
  }
}


JHANDLER_FUNCTION(Write) {
  JHANDLER_CHECK_THIS(object);
  JHANDLER_CHECK_ARGS(5, number, object, number, number, number);
  JHANDLER_CHECK_ARG_IF_EXIST(5, function);

  Environment* env = Environment::GetEnv();

  int fd = JHANDLER_GET_ARG(0, number);
  const iotjs_jval_t* jbuffer = JHANDLER_GET_ARG(1, object);
  int offset = JHANDLER_GET_ARG(2, number);
  int length = JHANDLER_GET_ARG(3, number);
  int position = JHANDLER_GET_ARG(4, number);
  const iotjs_jval_t* jcallback = JHANDLER_GET_ARG_IF_EXIST(5, function);

  BufferWrap* buffer_wrap = BufferWrap::FromJBuffer(jbuffer);
  char* data = buffer_wrap->buffer();
  int data_length = buffer_wrap->length();
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

  uv_buf_t uvbuf = uv_buf_init(reinterpret_cast<char*>(data + offset),
                               length);

  if (jcallback) {
    FS_ASYNC(env, write, jcallback, fd, &uvbuf, 1, position);
  } else {
    FS_SYNC(env, write, fd, &uvbuf, 1, position);
    if (err >= 0)
      iotjs_jhandler_return_number(jhandler, err);
  }
}


iotjs_jval_t MakeStatObject(uv_stat_t* statbuf) {
  Module* module = GetBuiltinModule(MODULE_FS);
  IOTJS_ASSERT(module != NULL);

  iotjs_jval_t* fs = &module->module;
  IOTJS_ASSERT(!iotjs_jval_is_undefined(fs));

  iotjs_jval_t create_stat = iotjs_jval_get_property(fs, "_createStat");
  IOTJS_ASSERT(iotjs_jval_is_function(&create_stat));

  iotjs_jval_t jstat = iotjs_jval_create_object();

#define X(statobj, name) \
  iotjs_jval_set_property_number(statobj, #name, statbuf->st_##name); \

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

  iotjs_jval_t res = iotjs_jhelper_call_ok(&create_stat,
                                           iotjs_jval_get_undefined(), &jargs);

  iotjs_jargs_destroy(&jargs);
  iotjs_jval_destroy(&create_stat);

  return res;
}


JHANDLER_FUNCTION(Stat) {
  JHANDLER_CHECK_THIS(object);
  JHANDLER_CHECK_ARGS(1, string);
  JHANDLER_CHECK_ARG_IF_EXIST(1, function);

  Environment* env = Environment::GetEnv();

  iotjs_string_t path = JHANDLER_GET_ARG(0, string);
  const iotjs_jval_t* jcallback = JHANDLER_GET_ARG_IF_EXIST(1, function);

  if (jcallback) {
    FS_ASYNC(env, stat, jcallback, iotjs_string_data(&path));
  } else {
    FS_SYNC(env, stat, iotjs_string_data(&path));
    if (err >= 0) {
      uv_stat_t* s = &(req_wrap.req()->statbuf);
      iotjs_jval_t stat = MakeStatObject(s);
      iotjs_jhandler_return_jval(jhandler, &stat);
      iotjs_jval_destroy(&stat);
    }
  }

  iotjs_string_destroy(&path);
}



JHANDLER_FUNCTION(MkDir) {
  JHANDLER_CHECK_THIS(object);
  JHANDLER_CHECK_ARGS(2, string, number);
  JHANDLER_CHECK_ARG_IF_EXIST(2, function);

  Environment* env = Environment::GetEnv();

  iotjs_string_t path = JHANDLER_GET_ARG(0, string);
  int mode = JHANDLER_GET_ARG(1, number);
  const iotjs_jval_t* jcallback = JHANDLER_GET_ARG_IF_EXIST(2, function);

  if (jcallback) {
    FS_ASYNC(env, mkdir, jcallback, iotjs_string_data(&path), mode);
  } else {
    FS_SYNC(env, mkdir, iotjs_string_data(&path), mode);
    if (err >= 0)
      iotjs_jhandler_return_undefined(jhandler);
  }

  iotjs_string_destroy(&path);
}


JHANDLER_FUNCTION(RmDir) {
  JHANDLER_CHECK_THIS(object);
  JHANDLER_CHECK_ARGS(1, string);
  JHANDLER_CHECK_ARG_IF_EXIST(1, function);

  Environment* env = Environment::GetEnv();

  iotjs_string_t path = JHANDLER_GET_ARG(0, string);
  const iotjs_jval_t* jcallback = JHANDLER_GET_ARG_IF_EXIST(1, function);

  if (jcallback) {
    FS_ASYNC(env, rmdir, jcallback, iotjs_string_data(&path));
  } else {
    FS_SYNC(env, rmdir, iotjs_string_data(&path));
    if (err >= 0)
      iotjs_jhandler_return_undefined(jhandler);
  }

  iotjs_string_destroy(&path);
}


JHANDLER_FUNCTION(Unlink) {
  JHANDLER_CHECK_THIS(object);
  JHANDLER_CHECK_ARGS(1, string);
  JHANDLER_CHECK_ARG_IF_EXIST(1, function);

  Environment* env = Environment::GetEnv();

  iotjs_string_t path = JHANDLER_GET_ARG(0, string);
  const iotjs_jval_t* jcallback = JHANDLER_GET_ARG_IF_EXIST(1, function);

  if (jcallback) {
    FS_ASYNC(env, unlink, jcallback, iotjs_string_data(&path));
  } else {
    FS_SYNC(env, unlink, iotjs_string_data(&path));
    if (err >= 0)
      iotjs_jhandler_return_undefined(jhandler);
  }

  iotjs_string_destroy(&path);
}


JHANDLER_FUNCTION(Rename) {
  JHANDLER_CHECK_THIS(object);
  JHANDLER_CHECK_ARGS(2, string, string);
  JHANDLER_CHECK_ARG_IF_EXIST(2, function);

  Environment* env = Environment::GetEnv();

  iotjs_string_t oldPath = JHANDLER_GET_ARG(0, string);
  iotjs_string_t newPath = JHANDLER_GET_ARG(1, string);
  const iotjs_jval_t* jcallback = JHANDLER_GET_ARG_IF_EXIST(2, function);

  if (jcallback) {
    FS_ASYNC(env, rename, jcallback, iotjs_string_data(&oldPath),
             iotjs_string_data(&newPath));
  } else {
    FS_SYNC(env, rename, iotjs_string_data(&oldPath),
            iotjs_string_data(&newPath));
    if (err >= 0)
      iotjs_jhandler_return_undefined(jhandler);
  }

  iotjs_string_destroy(&oldPath);
  iotjs_string_destroy(&newPath);
}


JHANDLER_FUNCTION(ReadDir) {
  JHANDLER_CHECK_THIS(object);
  JHANDLER_CHECK_ARGS(1, string);
  JHANDLER_CHECK_ARG_IF_EXIST(2, function);

  Environment* env = Environment::GetEnv();
  iotjs_string_t path = JHANDLER_GET_ARG(0, string);
  const iotjs_jval_t* jcallback = JHANDLER_GET_ARG_IF_EXIST(2, function);

  if (jcallback) {
    FS_ASYNC(env, scandir, jcallback, iotjs_string_data(&path), 0);
  } else {
    FS_SYNC(env, scandir, iotjs_string_data(&path), 0);
    if (err >= 0) {
      int r;
      uv_dirent_t ent;
      uint32_t idx = 0;
      iotjs_jval_t ret = iotjs_jval_create_array(0);
      while ((r = uv_fs_scandir_next(req_wrap.req(), &ent)) != UV_EOF) {
        iotjs_jval_t name = iotjs_jval_create_string_raw(ent.name);
        iotjs_jval_set_property_by_index(&ret, idx, &name);
        iotjs_jval_destroy(&name);
        idx++;
      }
      iotjs_jhandler_return_jval(jhandler, &ret);
      iotjs_jval_destroy(&ret);
    }
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


} // namespace iotjs
