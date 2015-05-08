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
#include "iotjs_handlewrap.h"
#include "iotjs_module.h"
#include "iotjs_module_fs.h"


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

  JArgList jarg(1);
  if (req->result < 0) {
    JObject arg0(static_cast<int32_t>(-req->result));
    jarg.Add(arg0);
  } else {
    switch (req->fs_type) {
      case UV_FS_OPEN:
        {
          JObject arg0(static_cast<int32_t>(req->result));
          jarg.Add(arg0);
        }
        break;

      default:
        jarg.Add(JObject::Null());
    }
  }

  JObject res = cb->Call(JObject::Null(), jarg);

  uv_fs_req_cleanup(req);

  delete req_wrap;
}


JHANDLER_FUNCTION(Open, handler) {
  int argc = handler.GetArgLength();

  if (argc < 1) {
    JERRY_THROW("type error: path required");
  }
  if (argc < 2) {
    JERRY_THROW("type error: flags required");
  }
  if (argc < 3) {
    JERRY_THROW("type error: mode required");
  }
  if (!handler.GetArg(0)->IsString()) {
    JERRY_THROW("type error: path must be a string");
  }
  if (!handler.GetArg(1)->IsNumber()) {
    JERRY_THROW("type error: flags must be an int");
  }
  if (!handler.GetArg(1)->IsNumber()) {
    JERRY_THROW("type error: mode must be an int");
  }

  Environment* env = Environment::GetEnv();

  char* path = handler.GetArg(0)->GetCString();
  int flags = handler.GetArg(1)->GetInt32();
  int mode = handler.GetArg(2)->GetInt32();

  if (argc > 3 && handler.GetArg(3)->IsFunction()) {
    JObject* jcallback = handler.GetArg(3);

    FsReqWrap* req_wrap = new FsReqWrap();
    req_wrap->set_callback(*jcallback);

    uv_fs_t* fs_req = req_wrap->data();

    int err = uv_fs_open(env->loop(), fs_req, path, flags, mode, After);
    req_wrap->Dispatched();
    if (err < 0) {
      fs_req->result = err;
      After(fs_req);
    }

    handler.Return(JObject::Null());
  } else {
    uv_fs_t fs_req;
    int err = uv_fs_open(env->loop(), &fs_req, path, flags, mode, NULL);
    if (err < 0) {
      JERRY_THROW("open failed!");
    } else {
      JObject ret(err);
      handler.Return(ret);
    }
  }
  JObject::ReleaseCString(path);

  return true;
}


JObject* InitFs() {
  Module* module = GetBuiltinModule(MODULE_FS);
  JObject* fs = module->module;

  if (fs == NULL) {
    fs = new JObject();
    fs->SetMethod("open", Open);

    module->module = fs;
  }

  return fs;
}

} // namespace iotjs
