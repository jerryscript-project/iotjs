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

#include <assert.h>

#include "uv.h"

#include "iotjs_util.h"
#include "iotjs_binding.h"
#include "iotjs_env.h"
#include "iotjs_module.h"
#include "iotjs_module_timer.h"


namespace iotjs {

class TimerWrap {
 public:
  explicit TimerWrap(JObject* othis);
  virtual ~TimerWrap();

  static TimerWrap* FromHandle(uv_timer_t* handle);
  uv_timer_t* handle_ptr() { return &_handle; }

  void OnTimeout();

  void set_timeout(int64_t timeout) { _timeout = timeout; }
  void set_repeat(int64_t repeat) { _repeat = repeat; }
  void set_callback(JObject* cb) { _callback = cb; _callback->Ref(); }

 protected:
  uv_timer_t _handle;
  JObject* _obj;
  JObject* _callback;
  int64_t _timeout;
  int64_t _repeat;
};


TimerWrap::TimerWrap(JObject* othis)
    : _obj(othis)
    , _callback(NULL) {
  _obj->Ref();
  _obj->SetNative((uintptr_t)this);

  Environment* env = Environment::GetEnv();
  uv_timer_init(env->loop(), &_handle);
  _handle.data = this;
}

TimerWrap::~TimerWrap() {
  _obj->SetNative((uintptr_t)NULL);
  delete _obj;
  if (_callback) {
    delete _callback;
  }
}

TimerWrap* TimerWrap::FromHandle(uv_timer_t* handle) {
  TimerWrap* timer_wrap = static_cast<TimerWrap*>(handle->data);
  assert(handle == timer_wrap->handle_ptr());
  return timer_wrap;
}


void TimerWrap::OnTimeout() {
  if (_callback && _callback->IsFunction())
    _callback->Call(_obj, NULL, 0);

  // Todo: remove delete line after binding object destruction with native obj
  if (_repeat == 0)
    delete this;
}

static void timerHandleTimeout(uv_timer_t* handle) {
  TimerWrap* timer_wrap = TimerWrap::FromHandle(handle);
  if (timer_wrap)
    timer_wrap->OnTimeout();
}


static bool timerStart(const jerry_api_object_t *function_obj_p,
                       const jerry_api_value_t *this_p,
                       jerry_api_value_t *ret_val_p,
                       const jerry_api_value_t args_p [],
                       const uint16_t args_cnt) {
  assert(args_cnt >=3);
  assert(JVAL_IS_FUNCTION(&args_p[2]));
  if (!(args_cnt >= 3)) return false;
  if (!JVAL_IS_FUNCTION(&args_p[2])) return false;

  int64_t timeout = JVAL_TO_INT64(&args_p[0]);
  int64_t repeat = JVAL_TO_INT64(&args_p[1]);
  JObject othis(this_p->v_object, false);

  TimerWrap* timer_wrap;
  timer_wrap = reinterpret_cast<TimerWrap*>(othis.GetNative());
  if (timer_wrap == NULL) {
    JERRY_THROW("err timer start: invalid timer obj");
    return false;
  }
  jerry_api_value_t* cb = const_cast<jerry_api_value_t*>(&args_p[2]);
  JObject* jcallback = new JObject(cb);

  timer_wrap->set_timeout(timeout);
  timer_wrap->set_repeat(repeat);
  timer_wrap->set_callback(jcallback);

  int err;
  err = uv_timer_start(timer_wrap->handle_ptr(), timerHandleTimeout,
                       timeout, repeat);
  *ret_val_p = JVal::Int(err);

  return true;
}

static bool timerStop(const jerry_api_object_t *function_obj_p,
                       const jerry_api_value_t *this_p,
                       jerry_api_value_t *ret_val_p,
                       const jerry_api_value_t args_p [],
                       const uint16_t args_cnt) {
  JObject othis(this_p->v_object, false);
  TimerWrap* timer_wrap;
  timer_wrap = reinterpret_cast<TimerWrap*>(othis.GetNative());
  if (timer_wrap == NULL) {
    JERRY_THROW("err timer stop: invalid timer obj");
    return false;
  }

  int err = uv_timer_stop(timer_wrap->handle_ptr());
  *ret_val_p = JVal::Int(err);

  // Todo: remove delete line after binding object destruction with native obj
  delete timer_wrap;

  return true;
}

static bool timerConstruct(const jerry_api_object_t *function_obj_p,
                           const jerry_api_value_t *this_p,
                           jerry_api_value_t *ret_val_p,
                           const jerry_api_value_t args_p [],
                           const uint16_t args_cnt) {
  // this must be an object
  assert(this_p->type == JERRY_API_DATA_TYPE_OBJECT);
  // Todo: check this function is called using new

  JObject* othis = new JObject(this_p);
  TimerWrap* timer_wrap = new TimerWrap(othis);
  if (timer_wrap == NULL) {
    JERRY_THROW("out of memory, new timer");
    return false;
  }

  return true;
}

JObject* InitTimer() {
  Module* module = GetBuiltinModule(MODULE_TIMER);
  JObject* timer = module->module;

  if (timer == NULL) {
    timer = new JObject(timerConstruct);

    JObject prototype;
    timer->SetProperty("prototype", &prototype);
    prototype.CreateMethod("start", timerStart);
    prototype.CreateMethod("stop", timerStop);

    module->module = timer;
  }

  return timer;
}

} // namespace iotjs
