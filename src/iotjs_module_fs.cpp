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
#include "iotjs_module.h"
#include "iotjs_module_fs.h"


namespace iotjs {

FsReqWrap::FsReqWrap(JObject* jcallback)
    : _callback(jcallback) {
}

FsReqWrap::~FsReqWrap() {
  delete _callback;
}


static void After(uv_fs_t* req) {
  FsReqWrap* req_wrap = static_cast<FsReqWrap*>(req->data);
  assert(req_wrap != NULL);
  assert(req_wrap->data() == req);

  jerry_api_value_t argv[1];
  uint16_t argc = 1;

  if (req->result < 0) {
    argv[0] = JVal::Int(-req->result);
  } else {
    argv[0] = JVal::Null();
    switch (req->fs_type) {
      case UV_FS_OPEN:
        argv[0] = JVal::Int(req->result);
        break;
    }
  }
  JObject* cb = req_wrap->callback();
  assert(cb->IsFunction());
  jerry_api_value_t res;
  jerry_api_call_function(cb->val().v_object, NULL, &res, argv, argc);

  uv_fs_req_cleanup(req);
  delete req_wrap;
}

static bool Open(const jerry_api_object_t *function_obj_p,
                 const jerry_api_value_t *this_p,
                 jerry_api_value_t *ret_val_p,
                 const jerry_api_value_t args_p [],
                 const uint16_t args_cnt) {
  if (args_cnt < 1) {
    JERRY_THROW("type error: path required");
  }
  if (args_cnt < 2) {
    JERRY_THROW("type error: flags required");
  }
  if (args_cnt < 3) {
    JERRY_THROW("type error: mode required");
  }
  if (!JVAL_IS_STRING(&args_p[0])) {
    JERRY_THROW("type error: path must be a string");
  }
  if (!JVAL_IS_NUMBER(&args_p[1])) {
    JERRY_THROW("type error: flags must be an int");
  }
  if (!JVAL_IS_NUMBER(&args_p[2])) {
    JERRY_THROW("type error: mode must be an int");
  }

  Environment* env = Environment::GetEnv();

  char* path = DupJerryString(&args_p[0]);
  int flags = JVAL_TO_INT32(&args_p[1]);
  int mode = JVAL_TO_INT32(&args_p[2]);

  if (args_cnt > 3 && JVAL_IS_FUNCTION(&args_p[3])) {
    JObject* jcallback = new JObject(
        const_cast<jerry_api_value_t*>(&args_p[3]));

    jcallback->Ref();

    FsReqWrap* req_wrap = new FsReqWrap(jcallback);
    uv_fs_t* fs_req = req_wrap->data();

    int err = uv_fs_open(env->loop(), fs_req, path, flags, mode, After);
    req_wrap->Dispatched();
    if (err < 0) {
      fs_req->result = err;
      After(fs_req);
    }
    *ret_val_p = JVal::Null();
  } else {
    uv_fs_t fs_req;
    int err = uv_fs_open(env->loop(), &fs_req, path, flags, mode, NULL);
    if (err < 0) {
      JERRY_THROW("open failed!");
    } else {
      *ret_val_p = JVal::Int(err);
    }
  }
  ReleaseCharBuffer(path);

  return true;
}


JObject* InitFs() {
  Module* module = GetBuiltinModule(MODULE_FS);
  JObject* fs = module->module;

  if (fs == NULL) {
    fs = new JObject();
    fs->CreateMethod("open", Open);

    module->module = fs;
  }

  return fs;
}

} // namespace iotjs
