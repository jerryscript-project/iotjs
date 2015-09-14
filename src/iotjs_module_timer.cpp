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
#include "iotjs_module_timer.h"

#include "iotjs_handlewrap.h"


namespace iotjs {


class TimerWrap : public HandleWrap {
 public:
  explicit TimerWrap(Environment* env, JObject& jtimer)
      : HandleWrap(jtimer, reinterpret_cast<uv_handle_t*>(&_handle))
      , _timeout(0)
      , _repeat(0)
      , _jcallback(NULL) {
    uv_timer_init(env->loop(), &_handle);
  }

  virtual ~TimerWrap() {
    if (_jcallback != NULL) {
      delete _jcallback;
    }
  }

  static TimerWrap* FromHandle(uv_timer_t* handle) {
    TimerWrap* timer_wrap = static_cast<TimerWrap*>(handle->data);
    IOTJS_ASSERT(handle == timer_wrap->handle_ptr());
    return timer_wrap;
  }

  uv_timer_t* handle_ptr() {
    return &_handle;
  }

  void set_timeout(int64_t timeout) {
    _timeout = timeout;
  }

  void set_repeat(int64_t repeat) {
    _repeat = repeat;
  }

  void set_callback(JObject& jcallback) {
    IOTJS_ASSERT(_jcallback == NULL);
    IOTJS_ASSERT(jcallback.IsFunction());

    JRawValueType raw_value = jcallback.raw_value();
    _jcallback = new JObject(&raw_value, false);
  }

  void OnTimeout() {
    if (_jcallback != NULL) {
      IOTJS_ASSERT(jobject().IsObject());
      IOTJS_ASSERT(_jcallback->IsFunction());
      MakeCallback(*_jcallback, jobject(), JArgList::Empty());
    }
  }

 protected:
  uv_timer_t _handle;
  int64_t _timeout;
  int64_t _repeat;
  JObject* _jcallback;
};


static void timerHandleTimeout(uv_timer_t* handle) {
  TimerWrap* timer_wrap = TimerWrap::FromHandle(handle);
  IOTJS_ASSERT(timer_wrap->jobject().IsObject());
  if (timer_wrap) {
    timer_wrap->OnTimeout();
  }
}


JHANDLER_FUNCTION(Start) {
  JHANDLER_CHECK(handler.GetThis()->IsObject());
  JHANDLER_CHECK(handler.GetArgLength() >= 3);
  JHANDLER_CHECK(handler.GetArg(0)->IsNumber());
  JHANDLER_CHECK(handler.GetArg(1)->IsNumber());
  JHANDLER_CHECK(handler.GetArg(2)->IsFunction());

  JObject* jtimer = handler.GetThis();

  int64_t timeout = handler.GetArg(0)->GetInt64();
  int64_t repeat = handler.GetArg(1)->GetInt64();
  JObject* jcallback = handler.GetArg(2);

  TimerWrap* timer_wrap = reinterpret_cast<TimerWrap*>(jtimer->GetNative());
  IOTJS_ASSERT(timer_wrap != NULL);
  IOTJS_ASSERT(timer_wrap->jobject().IsObject());

  timer_wrap->set_timeout(timeout);
  timer_wrap->set_repeat(repeat);
  timer_wrap->set_callback(*jcallback);

  int err;
  err = uv_timer_start(timer_wrap->handle_ptr(),
                       timerHandleTimeout,
                       timeout,
                       repeat);

  JObject ret(err);
  handler.Return(ret);

  return true;
}


JHANDLER_FUNCTION(Stop) {
  JHANDLER_CHECK(handler.GetThis()->IsObject());

  JObject* jtimer = handler.GetThis();

  TimerWrap* timer_wrap = reinterpret_cast<TimerWrap*>(jtimer->GetNative());
  IOTJS_ASSERT(timer_wrap != NULL);

  int err = uv_timer_stop(timer_wrap->handle_ptr());

  JObject ret(err);
  handler.Return(ret);

  return true;
}


JHANDLER_FUNCTION(Timer) {
  JHANDLER_CHECK(handler.GetThis()->IsObject());

  Environment* env = Environment::GetEnv();
  JObject* jtimer = handler.GetThis();

  TimerWrap* timer_wrap = new TimerWrap(env, *jtimer);
  IOTJS_ASSERT(timer_wrap->jobject().IsObject());
  IOTJS_ASSERT(jtimer->GetNative() != 0);;

  return true;
}


JObject* InitTimer() {
  Module* module = GetBuiltinModule(MODULE_TIMER);
  JObject* timer = module->module;

  if (timer == NULL) {
    timer = new JObject(Timer);

    JObject prototype;
    timer->SetProperty("prototype", prototype);
    prototype.SetMethod("start", Start);
    prototype.SetMethod("stop", Stop);

    module->module = timer;
  }

  return timer;
}


} // namespace iotjs
