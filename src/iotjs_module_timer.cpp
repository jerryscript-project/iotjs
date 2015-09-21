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
      , _jcallback(NULL) {
    // Initialze timer handler.
    uv_timer_init(env->loop(), &_handle);
  }

  // Timer timeout callback handler.
  void OnTimeout();

  // Timer close callback handler.
  void OnClose();

  // Start timer.
  int Start(int64_t timeout, int64_t repeat, JObject& jcallback);

  // Stop & close timer.
  int Stop();

  // Retreive javascript callback function.
  JObject* jcallback() { return _jcallback; }

 protected:
  // timer handle.
  uv_timer_t _handle;

  // Javascript callback function.
  JObject* _jcallback;
};


// This function is called from uv when timeout expires.
static void TimeoutHandler(uv_timer_t* handle) {
  // Find timer wrap from handle.
  HandleWrap* handle_wrap = HandleWrap::FromHandle((uv_handle_t*)handle);
  TimerWrap* timer_wrap = reinterpret_cast<TimerWrap*>(handle_wrap);

  // Call the timeout handler.
  timer_wrap->OnTimeout();
}


void TimerWrap::OnTimeout() {
  // Verification.
  IOTJS_ASSERT(jobject().IsObject());
  IOTJS_ASSERT(_jcallback != NULL);
  IOTJS_ASSERT(_jcallback->IsFunction());

  // Call javascirpt timeout callback function.
  MakeCallback(*_jcallback, jobject(), JArgList::Empty());
}


// Start timer.
int TimerWrap::Start(int64_t timeout, int64_t repeat, JObject& jcallback) {
  // We should not have javascript callback handler yet.
  IOTJS_ASSERT(_jcallback == NULL);
  IOTJS_ASSERT(jcallback.IsFunction());

  // Create new Javascirpt function reference for the callback function.
  _jcallback = new JObject(jcallback);

  // Start uv timer.
  return uv_timer_start(&_handle,
                        TimeoutHandler,
                        timeout,
                        repeat);
}


// This function is called from uv after timer close.
static void OnTimerClose(uv_handle_t* handle) {
  // Find timer wrap from handle.
  HandleWrap* handle_wrap = HandleWrap::FromHandle(handle);
  TimerWrap* timer_wrap = reinterpret_cast<TimerWrap*>(handle_wrap);

  // Call the close handler.
  timer_wrap->OnClose();
}


void TimerWrap::OnClose() {
  // If we have javascript timeout callback reference, release it.
  if (_jcallback != NULL) {
    delete _jcallback;
    _jcallback = NULL;
  }
}


int TimerWrap::Stop() {
  // Close timer.
  if (!uv_is_closing(__handle)) {
    Close(OnTimerClose);
  }

  return 0;
}


JHANDLER_FUNCTION(Start) {
  // Check parameters.
  JHANDLER_CHECK(handler.GetThis()->IsObject());
  JHANDLER_CHECK(handler.GetArgLength() >= 3);
  JHANDLER_CHECK(handler.GetArg(0)->IsNumber());
  JHANDLER_CHECK(handler.GetArg(1)->IsNumber());
  JHANDLER_CHECK(handler.GetArg(2)->IsFunction());

  JObject* jtimer = handler.GetThis();

  // Take timer wrap.
  TimerWrap* timer_wrap = reinterpret_cast<TimerWrap*>(jtimer->GetNative());
  IOTJS_ASSERT(timer_wrap != NULL);
  IOTJS_ASSERT(timer_wrap->jobject().IsObject());

  // parameters.
  int64_t timeout = handler.GetArg(0)->GetInt64();
  int64_t repeat = handler.GetArg(1)->GetInt64();
  JObject* jcallback = handler.GetArg(2);

  // We do not permit double start.
  JHANDLER_CHECK(timer_wrap->jcallback() == NULL);

  // Start timer.
  int res = timer_wrap->Start(timeout, repeat, *jcallback);

  JObject ret(res);
  handler.Return(ret);

  return true;
}


JHANDLER_FUNCTION(Stop) {
  JHANDLER_CHECK(handler.GetThis()->IsObject());

  JObject* jtimer = handler.GetThis();

  TimerWrap* timer_wrap = reinterpret_cast<TimerWrap*>(jtimer->GetNative());
  IOTJS_ASSERT(timer_wrap != NULL);
  IOTJS_ASSERT(timer_wrap->jobject().IsObject());

  // Stop timer.
  int res = timer_wrap->Stop();

  JObject ret(res);
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
