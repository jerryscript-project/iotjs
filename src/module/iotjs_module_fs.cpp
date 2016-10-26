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
#include "iotjs_module_fs.h"

#include "iotjs_module_buffer.h"
#include "iotjs_exception.h"
#include "iotjs_reqwrap.h"


namespace iotjs {


class FsReqWrap : public ReqWrap<uv_fs_t> {
 public:
  FsReqWrap(JObject& jcallback) : ReqWrap<uv_fs_t>(jcallback) {
  }

  ~FsReqWrap() {
    uv_fs_req_cleanup(&_req);
  }
};


static void After(uv_fs_t* req) {
  FsReqWrap* req_wrap = static_cast<FsReqWrap*>(req->data);
  IOTJS_ASSERT(req_wrap != NULL);
  IOTJS_ASSERT(req_wrap->req() == req);

  JObject cb = req_wrap->jcallback();
  IOTJS_ASSERT(cb.IsFunction());

  iotjs_jargs_t jarg = iotjs_jargs_create(2);
  if (req->result < 0) {
    JObject jerror(CreateUVException(req->result, "open"));
    iotjs_jargs_append_obj(&jarg, &jerror);
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
      case UV_FS_STAT: {
        uv_stat_t s = (req->statbuf);
        JObject ret(MakeStatObject(&s));
        iotjs_jargs_append_obj(&jarg, &ret);
        break;
      }
      default:
        iotjs_jargs_append_null(&jarg);
    }
  }

  JObject res = MakeCallback(cb, JObject::Undefined(), jarg);

  iotjs_jargs_destroy(&jarg);
  delete req_wrap;
}


#define FS_ASYNC(env, syscall, pcallback, ...) \
  FsReqWrap* req_wrap = new FsReqWrap(*pcallback); \
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
  FsReqWrap req_wrap(JObject::Null()); \
  int err = uv_fs_ ## syscall(env->loop(), \
                              req_wrap.req(), \
                              __VA_ARGS__, \
                              NULL); \
  if (err < 0) { \
    JObject jerror(CreateUVException(err, #syscall)); \
    iotjs_jhandler_throw_obj(jhandler, &jerror); \
  }


JHANDLER_FUNCTION(Close) {
  JHANDLER_CHECK(iotjs_jhandler_get_this(jhandler)->IsObject());
  JHANDLER_CHECK(iotjs_jhandler_get_arg_length(jhandler) >= 1);
  JHANDLER_CHECK(iotjs_jhandler_get_arg(jhandler, 0)->IsNumber());

  Environment* env = Environment::GetEnv();

  int fd = iotjs_jhandler_get_arg(jhandler, 0)->GetInt32();

  if (iotjs_jhandler_get_arg_length(jhandler) > 1 &&
      iotjs_jhandler_get_arg(jhandler, 1)->IsFunction()) {
    FS_ASYNC(env, close, iotjs_jhandler_get_arg(jhandler, 1), fd);
  } else {
    FS_SYNC(env, close, fd);
  }
}


JHANDLER_FUNCTION(Open) {
  JHANDLER_CHECK(iotjs_jhandler_get_this(jhandler)->IsObject());
  JHANDLER_CHECK(iotjs_jhandler_get_arg_length(jhandler) >= 3);
  JHANDLER_CHECK(iotjs_jhandler_get_arg(jhandler, 0)->IsString());
  JHANDLER_CHECK(iotjs_jhandler_get_arg(jhandler, 1)->IsNumber());
  JHANDLER_CHECK(iotjs_jhandler_get_arg(jhandler, 2)->IsNumber());

  Environment* env = Environment::GetEnv();

  iotjs_string_t path = iotjs_jhandler_get_arg(jhandler, 0)->GetString();
  int flags = iotjs_jhandler_get_arg(jhandler, 1)->GetInt32();
  int mode = iotjs_jhandler_get_arg(jhandler, 2)->GetInt32();

  if (iotjs_jhandler_get_arg_length(jhandler) > 3 &&
      iotjs_jhandler_get_arg(jhandler, 3)->IsFunction()) {
    FS_ASYNC(env, open, iotjs_jhandler_get_arg(jhandler, 3),
             iotjs_string_data(&path), flags, mode);
  } else {
    FS_SYNC(env, open, iotjs_string_data(&path), flags, mode);
    if (err >= 0)
      iotjs_jhandler_return_number(jhandler, err);
  }

  iotjs_string_destroy(&path);
}


JHANDLER_FUNCTION(Read) {
  JHANDLER_CHECK(iotjs_jhandler_get_this(jhandler)->IsObject());
  JHANDLER_CHECK(iotjs_jhandler_get_arg_length(jhandler) >= 5);
  JHANDLER_CHECK(iotjs_jhandler_get_arg(jhandler, 0)->IsNumber());
  JHANDLER_CHECK(iotjs_jhandler_get_arg(jhandler, 1)->IsObject());
  JHANDLER_CHECK(iotjs_jhandler_get_arg(jhandler, 2)->IsNumber());
  JHANDLER_CHECK(iotjs_jhandler_get_arg(jhandler, 3)->IsNumber());
  JHANDLER_CHECK(iotjs_jhandler_get_arg(jhandler, 4)->IsNumber());

  Environment* env = Environment::GetEnv();

  int fd = iotjs_jhandler_get_arg(jhandler, 0)->GetInt32();
  int offset = iotjs_jhandler_get_arg(jhandler, 2)->GetInt32();
  int length = iotjs_jhandler_get_arg(jhandler, 3)->GetInt32();
  int position = iotjs_jhandler_get_arg(jhandler, 4)->GetInt32();

  JObject* jbuffer = iotjs_jhandler_get_arg(jhandler, 1);
  BufferWrap* buffer_wrap = BufferWrap::FromJBuffer(*jbuffer);
  char* data = buffer_wrap->buffer();
  int data_length = buffer_wrap->length();
  JHANDLER_CHECK(data != NULL);
  JHANDLER_CHECK(data_length > 0);

  if (offset >= data_length) {
    JHANDLER_THROW_RETURN(RangeError, "offset out of bound");
  }
  if (offset + length > data_length) {
    JHANDLER_THROW_RETURN(RangeError, "length out of bound");
  }

  uv_buf_t uvbuf = uv_buf_init(reinterpret_cast<char*>(data + offset),
                               length);

  if (iotjs_jhandler_get_arg_length(jhandler) > 5 &&
      iotjs_jhandler_get_arg(jhandler, 5)->IsFunction()) {
    FS_ASYNC(env, read, iotjs_jhandler_get_arg(jhandler, 5), fd, &uvbuf, 1,
             position);
  } else {
    FS_SYNC(env, read, fd, &uvbuf, 1, position);
    if (err >= 0)
      iotjs_jhandler_return_number(jhandler, err);
  }
}


JHANDLER_FUNCTION(Write) {
  JHANDLER_CHECK(iotjs_jhandler_get_this(jhandler)->IsObject());
  JHANDLER_CHECK(iotjs_jhandler_get_arg_length(jhandler) >= 5);
  JHANDLER_CHECK(iotjs_jhandler_get_arg(jhandler, 0)->IsNumber());
  JHANDLER_CHECK(iotjs_jhandler_get_arg(jhandler, 1)->IsObject());
  JHANDLER_CHECK(iotjs_jhandler_get_arg(jhandler, 2)->IsNumber());
  JHANDLER_CHECK(iotjs_jhandler_get_arg(jhandler, 3)->IsNumber());
  JHANDLER_CHECK(iotjs_jhandler_get_arg(jhandler, 4)->IsNumber());

  Environment* env = Environment::GetEnv();

  int fd = iotjs_jhandler_get_arg(jhandler, 0)->GetInt32();
  int offset = iotjs_jhandler_get_arg(jhandler, 2)->GetInt32();
  int length = iotjs_jhandler_get_arg(jhandler, 3)->GetInt32();
  int position = iotjs_jhandler_get_arg(jhandler, 4)->GetInt32();

  JObject* jbuffer = iotjs_jhandler_get_arg(jhandler, 1);
  BufferWrap* buffer_wrap = BufferWrap::FromJBuffer(*jbuffer);
  char* data = buffer_wrap->buffer();
  int data_length = buffer_wrap->length();
  JHANDLER_CHECK(data != NULL);
  JHANDLER_CHECK(data_length > 0);

  if (offset >= data_length) {
    JHANDLER_THROW_RETURN(RangeError, "offset out of bound");
  }
  if (offset + length > data_length) {
    JHANDLER_THROW_RETURN(RangeError, "length out of bound");
  }

  uv_buf_t uvbuf = uv_buf_init(reinterpret_cast<char*>(data + offset),
                               length);

  if (iotjs_jhandler_get_arg_length(jhandler) > 5 &&
      iotjs_jhandler_get_arg(jhandler, 5)->IsFunction()) {
    FS_ASYNC(env, write, iotjs_jhandler_get_arg(jhandler, 5), fd, &uvbuf, 1,
             position);
  } else {
    FS_SYNC(env, write, fd, &uvbuf, 1, position);
    if (err >= 0)
      iotjs_jhandler_return_number(jhandler, err);
  }
}


JObject MakeStatObject(uv_stat_t* statbuf) {
  Module* module = GetBuiltinModule(MODULE_FS);
  IOTJS_ASSERT(module != NULL);

  JObject* fs = module->module;
  IOTJS_ASSERT(fs != NULL);

  JObject createStat = fs->GetProperty("_createStat");
  IOTJS_ASSERT(createStat.IsFunction());

  JObject jstat;

#define X(statobj, name) \
  JObject name((int32_t)statbuf->st_##name); \
  statobj.SetProperty(#name, name); \

  X(jstat, dev)
  X(jstat, mode)
  X(jstat, nlink)
  X(jstat, uid)
  X(jstat, gid)
  X(jstat, rdev)

#undef X

#define X(statobj, name) \
  JObject name((double)statbuf->st_##name); \
  statobj.SetProperty(#name, name); \

  X(jstat, blksize)
  X(jstat, ino)
  X(jstat, size)
  X(jstat, blocks)

#undef X

  iotjs_jargs_t jargs = iotjs_jargs_create(1);
  iotjs_jargs_append_obj(&jargs, &jstat);

  JResult jstat_res(createStat.Call(JObject::Undefined(), jargs));
  IOTJS_ASSERT(jstat_res.IsOk());

  iotjs_jargs_destroy(&jargs);

  return jstat_res.value();
}


JHANDLER_FUNCTION(Stat) {
  JHANDLER_CHECK(iotjs_jhandler_get_this(jhandler)->IsObject());
  JHANDLER_CHECK(iotjs_jhandler_get_arg_length(jhandler) >= 1);
  JHANDLER_CHECK(iotjs_jhandler_get_arg(jhandler, 0)->IsString());

  Environment* env = Environment::GetEnv();

  iotjs_string_t path = iotjs_jhandler_get_arg(jhandler, 0)->GetString();

  if (iotjs_jhandler_get_arg_length(jhandler) > 1 &&
      iotjs_jhandler_get_arg(jhandler, 1)->IsFunction()) {
    FS_ASYNC(env, stat, iotjs_jhandler_get_arg(jhandler, 1),
             iotjs_string_data(&path));
  } else {
    FS_SYNC(env, stat, iotjs_string_data(&path));
    if (err >= 0) {
      uv_stat_t* s = &(req_wrap.req()->statbuf);
      JObject ret(MakeStatObject(s));
      iotjs_jhandler_return_obj(jhandler, &ret);
    }
  }

  iotjs_string_destroy(&path);
}


JHANDLER_FUNCTION(Mkdir) {
  JHANDLER_CHECK(iotjs_jhandler_get_this(jhandler)->IsObject());
  JHANDLER_CHECK(iotjs_jhandler_get_arg_length(jhandler) >= 2);
  JHANDLER_CHECK(iotjs_jhandler_get_arg(jhandler, 0)->IsString());
  JHANDLER_CHECK(iotjs_jhandler_get_arg(jhandler, 1)->IsNumber());
  Environment* env = Environment::GetEnv();

  iotjs_string_t path = iotjs_jhandler_get_arg(jhandler, 0)->GetString();
  int mode = iotjs_jhandler_get_arg(jhandler, 1)->GetInt32();

  if (iotjs_jhandler_get_arg_length(jhandler) > 2 &&
      iotjs_jhandler_get_arg(jhandler, 2)->IsFunction()) {
    FS_ASYNC(env, mkdir, iotjs_jhandler_get_arg(jhandler, 2),
             iotjs_string_data(&path), mode);
  } else {
    JHANDLER_CHECK(iotjs_jhandler_get_arg(jhandler, 1)->IsNumber());
    FS_SYNC(env, mkdir, iotjs_string_data(&path), mode);
    if (err >= 0)
      iotjs_jhandler_return_undefined(jhandler);
  }

  iotjs_string_destroy(&path);
}


JHANDLER_FUNCTION(Rmdir) {
  JHANDLER_CHECK(iotjs_jhandler_get_this(jhandler)->IsObject());
  JHANDLER_CHECK(iotjs_jhandler_get_arg_length(jhandler) >= 1);
  JHANDLER_CHECK(iotjs_jhandler_get_arg(jhandler, 0)->IsString());
  Environment* env = Environment::GetEnv();

  iotjs_string_t path = iotjs_jhandler_get_arg(jhandler, 0)->GetString();

  if (iotjs_jhandler_get_arg_length(jhandler) > 1 &&
      iotjs_jhandler_get_arg(jhandler, 1)->IsFunction()) {
    FS_ASYNC(env, rmdir, iotjs_jhandler_get_arg(jhandler, 1),
             iotjs_string_data(&path));
  } else {
    FS_SYNC(env, rmdir, iotjs_string_data(&path));
    if (err >= 0)
      iotjs_jhandler_return_undefined(jhandler);
  }

  iotjs_string_destroy(&path);
}


JHANDLER_FUNCTION(Unlink) {
  JHANDLER_CHECK(iotjs_jhandler_get_this(jhandler)->IsObject());
  JHANDLER_CHECK(iotjs_jhandler_get_arg_length(jhandler) >= 1);
  JHANDLER_CHECK(iotjs_jhandler_get_arg(jhandler, 0)->IsString());

  Environment* env = Environment::GetEnv();
  iotjs_string_t path = iotjs_jhandler_get_arg(jhandler, 0)->GetString();

  if (iotjs_jhandler_get_arg_length(jhandler) > 1 &&
      iotjs_jhandler_get_arg(jhandler, 1)->IsFunction()) {
    FS_ASYNC(env, unlink, iotjs_jhandler_get_arg(jhandler, 1),
             iotjs_string_data(&path));
  } else {
    FS_SYNC(env, unlink, iotjs_string_data(&path));
    if (err >= 0)
      iotjs_jhandler_return_undefined(jhandler);
  }

  iotjs_string_destroy(&path);
}


JHANDLER_FUNCTION(Rename) {
  JHANDLER_CHECK(iotjs_jhandler_get_this(jhandler)->IsObject());
  JHANDLER_CHECK(iotjs_jhandler_get_arg_length(jhandler) >= 2);
  JHANDLER_CHECK(iotjs_jhandler_get_arg(jhandler, 0)->IsString());
  JHANDLER_CHECK(iotjs_jhandler_get_arg(jhandler, 1)->IsString());

  Environment* env = Environment::GetEnv();
  iotjs_string_t oldPath = iotjs_jhandler_get_arg(jhandler, 0)->GetString();
  iotjs_string_t newPath = iotjs_jhandler_get_arg(jhandler, 1)->GetString();

  if (iotjs_jhandler_get_arg_length(jhandler) > 2 &&
      iotjs_jhandler_get_arg(jhandler, 2)->IsFunction()) {
    FS_ASYNC(env, rename, iotjs_jhandler_get_arg(jhandler, 2),
             iotjs_string_data(&oldPath), iotjs_string_data(&newPath));
  } else {
    FS_SYNC(env, rename, iotjs_string_data(&oldPath),
            iotjs_string_data(&newPath));
    if (err >= 0)
      iotjs_jhandler_return_undefined(jhandler);
  }

  iotjs_string_destroy(&oldPath);
  iotjs_string_destroy(&newPath);
}


JObject* InitFs() {
  Module* module = GetBuiltinModule(MODULE_FS);
  JObject* fs = module->module;

  if (fs == NULL) {
    fs = new JObject();
    fs->SetMethod("close", Close);
    fs->SetMethod("open", Open);
    fs->SetMethod("read", Read);
    fs->SetMethod("write", Write);
    fs->SetMethod("stat", Stat);
    fs->SetMethod("mkdir", Mkdir);
    fs->SetMethod("rmdir", Rmdir);
    fs->SetMethod("unlink", Unlink);
    fs->SetMethod("rename", Rename);

    module->module = fs;
  }

  return fs;
}


} // namespace iotjs
