/* Copyright 2015 Samsung Electronics Co., Ltd.
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

  JArgList jarg(2);
  if (req->result < 0) {
    JObject jerror(CreateUVException(req->result, "open"));
    jarg.Add(jerror);
  } else {
    jarg.Add(JObject::Null());
    switch (req->fs_type) {
      case UV_FS_CLOSE:
      {
        break;
      }
      case UV_FS_OPEN:
      case UV_FS_READ:
      case UV_FS_WRITE:
      {
        JObject arg1(static_cast<int>(req->result));
        jarg.Add(arg1);
        break;
      }
      case UV_FS_STAT: {
        uv_stat_t s = (req->statbuf);
        JObject ret(MakeStatObject(&s));
        jarg.Add(ret);
        break;
      }
      default:
        jarg.Add(JObject::Null());
    }
  }

  JObject res = MakeCallback(cb, JObject::Null(), jarg);

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
  handler.Return(JObject::Null()); \


#define FS_SYNC(env, syscall, ...) \
  FsReqWrap req_wrap(JObject::Null()); \
  int err = uv_fs_ ## syscall(env->loop(), \
                              req_wrap.req(), \
                              __VA_ARGS__, \
                              NULL); \
  if (err < 0) { \
    JObject jerror(CreateUVException(err, #syscall)); \
    handler.Throw(jerror); \
    return false; \
  } \


JHANDLER_FUNCTION(Close) {
  JHANDLER_CHECK(handler.GetThis()->IsObject());
  JHANDLER_CHECK(handler.GetArgLength() >= 1);
  JHANDLER_CHECK(handler.GetArg(0)->IsNumber());

  Environment* env = Environment::GetEnv();

  int fd = handler.GetArg(0)->GetInt32();

  if (handler.GetArgLength() > 1 && handler.GetArg(1)->IsFunction()) {
    FS_ASYNC(env, close, handler.GetArg(1), fd);
  } else {
    FS_SYNC(env, close, fd);
  }

  return !handler.HasThrown();
}


JHANDLER_FUNCTION(Open) {
  JHANDLER_CHECK(handler.GetThis()->IsObject());
  JHANDLER_CHECK(handler.GetArgLength() >= 3);
  JHANDLER_CHECK(handler.GetArg(0)->IsString());
  JHANDLER_CHECK(handler.GetArg(1)->IsNumber());
  JHANDLER_CHECK(handler.GetArg(2)->IsNumber());

  Environment* env = Environment::GetEnv();

  String path = handler.GetArg(0)->GetString();
  int flags = handler.GetArg(1)->GetInt32();
  int mode = handler.GetArg(2)->GetInt32();

  if (handler.GetArgLength() > 3 && handler.GetArg(3)->IsFunction()) {
    FS_ASYNC(env, open, handler.GetArg(3), path.data(), flags, mode);
  } else {
    FS_SYNC(env, open, path.data(), flags, mode);
    handler.Return(JVal::Number(err));
  }

  return !handler.HasThrown();
}


JHANDLER_FUNCTION(Read) {
  JHANDLER_CHECK(handler.GetThis()->IsObject());
  JHANDLER_CHECK(handler.GetArgLength() >= 5);
  JHANDLER_CHECK(handler.GetArg(0)->IsNumber());
  JHANDLER_CHECK(handler.GetArg(1)->IsObject());
  JHANDLER_CHECK(handler.GetArg(2)->IsNumber());
  JHANDLER_CHECK(handler.GetArg(3)->IsNumber());
  JHANDLER_CHECK(handler.GetArg(4)->IsNumber());

  Environment* env = Environment::GetEnv();

  int fd = handler.GetArg(0)->GetInt32();
  int offset = handler.GetArg(2)->GetInt32();
  int length = handler.GetArg(3)->GetInt32();
  int position = handler.GetArg(4)->GetInt32();

  JObject* jbuffer = handler.GetArg(1);
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

  if (handler.GetArgLength() > 5 && handler.GetArg(5)->IsFunction()) {
    FS_ASYNC(env, read, handler.GetArg(5), fd, &uvbuf, 1, position);
  } else {
    FS_SYNC(env, read, fd, &uvbuf, 1, position);
    handler.Return(JVal::Number(err));
  }

  return !handler.HasThrown();
}


JHANDLER_FUNCTION(Write) {
  JHANDLER_CHECK(handler.GetThis()->IsObject());
  JHANDLER_CHECK(handler.GetArgLength() >= 5);
  JHANDLER_CHECK(handler.GetArg(0)->IsNumber());
  JHANDLER_CHECK(handler.GetArg(1)->IsObject());
  JHANDLER_CHECK(handler.GetArg(2)->IsNumber());
  JHANDLER_CHECK(handler.GetArg(3)->IsNumber());
  JHANDLER_CHECK(handler.GetArg(4)->IsNumber());

  Environment* env = Environment::GetEnv();

  int fd = handler.GetArg(0)->GetInt32();
  int offset = handler.GetArg(2)->GetInt32();
  int length = handler.GetArg(3)->GetInt32();
  int position = handler.GetArg(4)->GetInt32();

  JObject* jbuffer = handler.GetArg(1);
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

  if (handler.GetArgLength() > 5 && handler.GetArg(5)->IsFunction()) {
    FS_ASYNC(env, write, handler.GetArg(5), fd, &uvbuf, 1, position);
  } else {
    FS_SYNC(env, write, fd, &uvbuf, 1, position);
    handler.Return(JVal::Number(err));
  }

  return !handler.HasThrown();
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

  JArgList jargs(1);
  jargs.Add(jstat);

  JResult jstat_res(createStat.Call(JObject::Null(), jargs));
  IOTJS_ASSERT(jstat_res.IsOk());

  return jstat_res.value();
}


JHANDLER_FUNCTION(Stat) {
  JHANDLER_CHECK(handler.GetThis()->IsObject());
  JHANDLER_CHECK(handler.GetArgLength() >= 1);
  JHANDLER_CHECK(handler.GetArg(0)->IsString());

  Environment* env = Environment::GetEnv();

  String path = handler.GetArg(0)->GetString();

  if (handler.GetArgLength() > 1 && handler.GetArg(1)->IsFunction()) {
    FS_ASYNC(env, stat, handler.GetArg(1), path.data());
  } else {
    FS_SYNC(env, stat, path.data());
    uv_stat_t* s = &(req_wrap.req()->statbuf);
    JObject ret(MakeStatObject(s));
    handler.Return(ret);
  }

  return true;
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

    module->module = fs;
  }

  return fs;
}


} // namespace iotjs
