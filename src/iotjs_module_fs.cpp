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

#include "iotjs_binding.h"
#include "iotjs_env.h"
#include "iotjs_exception.h"
#include "iotjs_handlewrap.h"
#include "iotjs_module.h"
#include "iotjs_module_fs.h"
#include "iotjs_module_process.h"


namespace iotjs {


class FsReqWrap : public HandleWrap {
 public:
  explicit FsReqWrap()
      : HandleWrap(NULL, reinterpret_cast<uv_handle_t*>(&_data)) {
  }

  ~FsReqWrap() {}

  uv_req_t* req() {
    return reinterpret_cast<uv_req_t*>(&_data);
  }

  uv_fs_t* data() {
    return &_data;
  }

  void Dispatched() {
    req()->data = this;
  }

 private:
  uv_fs_t _data;
};


static void After(uv_fs_t* req) {
  FsReqWrap* req_wrap = static_cast<FsReqWrap*>(req->data);
  assert(req_wrap != NULL);
  assert(req_wrap->data() == req);

  JObject* cb = req_wrap->callback();
  assert(cb != NULL && cb->IsFunction());

  JArgList jarg(2);
  if (req->result < 0) {
    JObject jerror(CreateUVException(req->result, "open"));
    jarg.Add(jerror);
  } else {
    jarg.Add(JObject::Null());
    switch (req->fs_type) {
      case UV_FS_OPEN:
      case UV_FS_READ:
      {
        JObject arg1(static_cast<int32_t>(req->result));
        jarg.Add(arg1);
        break;
      }
      default:
        jarg.Add(JObject::Null());
    }
  }

  JObject res = MakeCallback(*cb, JObject::Null(), jarg);

  uv_fs_req_cleanup(req);

  cb->Unref();
  delete req_wrap;
}


#define FS_ASYNC(env, syscall, pcallback, ...) \
  FsReqWrap* req_wrap = new FsReqWrap(); \
  pcallback->Ref(); \
  req_wrap->set_callback(*pcallback); \
  uv_fs_t* fs_req = req_wrap->data(); \
  int err = uv_fs_ ## syscall(env->loop(), \
                              fs_req, \
                              __VA_ARGS__, \
                              After); \
  req_wrap->Dispatched(); \
  if (err < 0) { \
    fs_req->result = err; \
    After(fs_req); \
  } \
  handler.Return(JObject::Null()); \


#define FS_SYNC(env, syscall, ...) \
  uv_fs_t fs_req; \
  int err = uv_fs_ ## syscall(env->loop(), \
                              &fs_req, \
                              __VA_ARGS__, \
                              NULL); \
  if (err < 0) { \
    JObject jerror(CreateUVException(err, #syscall)); \
    handler.Throw(jerror); \
  } else { \
    JObject ret(err);\
    handler.Return(ret); \
  } \


JHANDLER_FUNCTION(Open, handler) {
  int argc = handler.GetArgLength();

  if (argc < 1) {
    JHANDLER_THROW_RETURN(handler, TypeError, "path required");
  } else if (argc < 2) {
    JHANDLER_THROW_RETURN(handler, TypeError, "flags required");
  } else if (argc < 3) {
    JHANDLER_THROW_RETURN(handler, TypeError, "mode required");
  }

  if (!handler.GetArg(0)->IsString()) {
    JHANDLER_THROW_RETURN(handler, TypeError, "path must be a string");
  } else if (!handler.GetArg(1)->IsNumber()) {
    JHANDLER_THROW_RETURN(handler, TypeError, "flags must be an int");
  } else if (!handler.GetArg(1)->IsNumber()) {
    JHANDLER_THROW_RETURN(handler, TypeError, "mode must be an int");
  }

  Environment* env = Environment::GetEnv();

  char* path = handler.GetArg(0)->GetCString();
  int flags = handler.GetArg(1)->GetInt32();
  int mode = handler.GetArg(2)->GetInt32();

  if (argc > 3 && handler.GetArg(3)->IsFunction()) {
    FS_ASYNC(env, open, handler.GetArg(3), path, flags, mode);
  } else {
    FS_SYNC(env, open, path, flags, mode);
  }

  return !handler.HasThrown();
}


JHANDLER_FUNCTION(Read, handler) {
  int argc = handler.GetArgLength();

  if (argc < 1) {
    JHANDLER_THROW_RETURN(handler, TypeError, "fd required");
  } else if (argc < 2) {
    JHANDLER_THROW_RETURN(handler, TypeError, "buffer required");
  } else if (argc < 3) {
    JHANDLER_THROW_RETURN(handler, TypeError, "offset required");
  } else if (argc < 4) {
    JHANDLER_THROW_RETURN(handler, TypeError, "length required");
  } else if (argc < 5) {
    JHANDLER_THROW_RETURN(handler, TypeError, "position required");
  }

  if (!handler.GetArg(0)->IsNumber()) {
    JHANDLER_THROW_RETURN(handler, TypeError, "fd must be an int");
  } else if (!handler.GetArg(1)->IsObject()) {
    JHANDLER_THROW_RETURN(handler, TypeError, "buffer must be a Buffer");
  } else if (!handler.GetArg(2)->IsNumber()) {
    JHANDLER_THROW_RETURN(handler, TypeError, "offset must be an int");
  } else if (!handler.GetArg(3)->IsNumber()) {
    JHANDLER_THROW_RETURN(handler, TypeError, "length must be an int");
  } else if (!handler.GetArg(4)->IsNumber()) {
    JHANDLER_THROW_RETURN(handler, TypeError, "position must be an int");
  }

  Environment* env = Environment::GetEnv();

  int fd = handler.GetArg(0)->GetInt32();
  int offset = handler.GetArg(2)->GetInt32();
  int length = handler.GetArg(3)->GetInt32();
  int position = handler.GetArg(4)->GetInt32();

  JObject* jbuffer = handler.GetArg(1);
  char* buffer = reinterpret_cast<char*>(jbuffer->GetNative());
  int buffer_length = jbuffer->GetProperty("length").GetInt32();

  if (offset >= buffer_length) {
    JHANDLER_THROW_RETURN(handler, RangeError, "offset out of bound");
  }
  if (offset + length > buffer_length) {
    JHANDLER_THROW_RETURN(handler, RangeError, "length out of bound");
  }

  uv_buf_t uvbuf = uv_buf_init(buffer + offset, length);

  if (argc > 5 && handler.GetArg(5)->IsFunction()) {
    FS_ASYNC(env, read, handler.GetArg(5), fd, &uvbuf, 1, position);
  } else {
    FS_SYNC(env, read, fd, &uvbuf, 1, position);
  }

  return !handler.HasThrown();
}


JObject* InitFs() {
  Module* module = GetBuiltinModule(MODULE_FS);
  JObject* fs = module->module;

  if (fs == NULL) {
    fs = new JObject();
    fs->SetMethod("open", Open);
    fs->SetMethod("read", Read);

    module->module = fs;
  }

  return fs;
}


} // namespace iotjs
