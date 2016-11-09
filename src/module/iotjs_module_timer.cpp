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


namespace iotjs {


// This function is called from uv when timeout expires.
static void TimeoutHandler(uv_timer_t* handle) {
  // Find timer wrap from handle.
  iotjs_handlewrap_t* handle_wrap =
      iotjs_handlewrap_from_handle((uv_handle_t*)handle);
  TimerWrap* timer_wrap = reinterpret_cast<TimerWrap*>(handle_wrap);

  // Call the timeout handler.
  timer_wrap->OnTimeout();
}


void TimerWrap::OnTimeout() {
  // Verification.
  IOTJS_ASSERT(iotjs_jval_is_object(jobject()));

  // Call javascript timeout handler function.
  iotjs_jval_t jcallback = iotjs_jval_get_property(jobject(), "handleTimeout");
  iotjs_make_callback(&jcallback, jobject(), iotjs_jargs_get_empty());
  iotjs_jval_destroy(&jcallback);
}


// Start timer.
int TimerWrap::Start(int64_t timeout, int64_t repeat) {
  // Start uv timer.
  return uv_timer_start(&_handle,
                        TimeoutHandler,
                        timeout,
                        repeat);
}


// This function is called from uv after timer close.
static void OnTimerClose(uv_handle_t* handle) {
  // Find timer wrap from handle.
  iotjs_handlewrap_t* handle_wrap = iotjs_handlewrap_from_handle(handle);
  TimerWrap* timer_wrap = reinterpret_cast<TimerWrap*>(handle_wrap);

  // Call the close handler.
  timer_wrap->OnClose();
}


void TimerWrap::OnClose() {
  // If we have javascript timeout callback reference, release it.
}


int TimerWrap::Stop() {
  // Close timer.
  if (!uv_is_closing(iotjs_handlewrap_get_uv_handle(&_handlewrap))) {
    iotjs_handlewrap_close(&_handlewrap, OnTimerClose);
  }

  return 0;
}


JHANDLER_FUNCTION(Start) {
  // Check parameters.
  JHANDLER_CHECK_THIS(object);
  JHANDLER_CHECK_ARGS(2, number, number);

  const iotjs_jval_t* jtimer = JHANDLER_GET_THIS(object);

  // Take timer wrap.
  TimerWrap* timer_wrap = reinterpret_cast<TimerWrap*>(
          iotjs_jval_get_object_native_handle(jtimer));
  IOTJS_ASSERT(timer_wrap != NULL);
  IOTJS_ASSERT(iotjs_jval_is_object(timer_wrap->jobject()));

  // parameters.
  int64_t timeout = JHANDLER_GET_ARG(0, number);
  int64_t repeat = JHANDLER_GET_ARG(1, number);

  // Start timer.
  int res = timer_wrap->Start(timeout, repeat);

  iotjs_jhandler_return_number(jhandler, res);
}


JHANDLER_FUNCTION(Stop) {
  JHANDLER_CHECK_THIS(object);

  const iotjs_jval_t* jtimer = JHANDLER_GET_THIS(object);

  TimerWrap* timer_wrap = reinterpret_cast<TimerWrap*>(
          iotjs_jval_get_object_native_handle(jtimer));
  IOTJS_ASSERT(timer_wrap != NULL);
  IOTJS_ASSERT(iotjs_jval_is_object(timer_wrap->jobject()));

  // Stop timer.
  int res = timer_wrap->Stop();

  iotjs_jhandler_return_number(jhandler, res);
}


JHANDLER_FUNCTION(Timer) {
  JHANDLER_CHECK_THIS(object);

  const iotjs_jval_t* jtimer = JHANDLER_GET_THIS(object);

  TimerWrap* timer_wrap = new TimerWrap(jtimer);
  IOTJS_ASSERT(iotjs_jval_is_object(timer_wrap->jobject()));
  IOTJS_ASSERT(iotjs_jval_get_object_native_handle(jtimer) != 0);
}


iotjs_jval_t InitTimer() {

  iotjs_jval_t timer = iotjs_jval_create_function(Timer);

  iotjs_jval_t prototype = iotjs_jval_create_object();
  iotjs_jval_set_property_jval(&timer, "prototype", &prototype);

  iotjs_jval_set_method(&prototype, "start", Start);
  iotjs_jval_set_method(&prototype, "stop", Stop);

  iotjs_jval_destroy(&prototype);

  return timer;
}


} // namespace iotjs


extern "C" {

iotjs_jval_t InitTimer() {
  return iotjs::InitTimer();
}

} // extern "C"
