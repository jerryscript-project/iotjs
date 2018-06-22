/* Copyright 2015-present Samsung Electronics Co., Ltd. and other contributors
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


static void iotjs_timerwrap_destroy(iotjs_timerwrap_t* timerwrap);
static void iotjs_timerwrap_on_timeout(iotjs_timerwrap_t* timerwrap);
IOTJS_DEFINE_NATIVE_HANDLE_INFO_THIS_MODULE(timerwrap);


iotjs_timerwrap_t* iotjs_timerwrap_create(const jerry_value_t jtimer) {
  iotjs_timerwrap_t* timerwrap = IOTJS_ALLOC(iotjs_timerwrap_t);
  uv_timer_t* uv_timer = IOTJS_ALLOC(uv_timer_t);

  iotjs_handlewrap_initialize(&timerwrap->handlewrap, jtimer,
                              (uv_handle_t*)(uv_timer),
                              &this_module_native_info);

  // Initialize timer handler.
  const iotjs_environment_t* env = iotjs_environment_get();
  uv_timer_init(iotjs_environment_loop(env), uv_timer);

  return timerwrap;
}


static void iotjs_timerwrap_destroy(iotjs_timerwrap_t* timerwrap) {
  iotjs_handlewrap_destroy(&timerwrap->handlewrap);

  IOTJS_RELEASE(timerwrap);
}

static void TimoutHandlerDestroy(uv_handle_t* handle) {
  IOTJS_RELEASE(handle);
}

// This function is called from uv when timeout expires.
static void TimeoutHandler(uv_timer_t* handle) {
  // Find timer wrap from handle.
  iotjs_timerwrap_t* timer_wrap = iotjs_timerwrap_from_handle(handle);

  // Call the timeout handler.
  iotjs_timerwrap_on_timeout(timer_wrap);
}


int iotjs_timerwrap_start(iotjs_timerwrap_t* timerwrap, uint64_t timeout,
                          uint64_t repeat) {
  // Start uv timer.
  uv_timer_t* uv_timer =
      (uv_timer_t*)iotjs_handlewrap_get_uv_handle(&timerwrap->handlewrap);
  return uv_timer_start(uv_timer, TimeoutHandler, timeout, repeat);
}


int iotjs_timerwrap_stop(iotjs_timerwrap_t* timerwrap) {
  if (!uv_is_closing(iotjs_handlewrap_get_uv_handle(&timerwrap->handlewrap))) {
    iotjs_handlewrap_close(&timerwrap->handlewrap, TimoutHandlerDestroy);
  }

  return 0;
}


static void iotjs_timerwrap_on_timeout(iotjs_timerwrap_t* timerwrap) {
  // Call javascript timeout handler function.
  jerry_value_t jobject = iotjs_timerwrap_jobject(timerwrap);
  jerry_value_t jcallback =
      iotjs_jval_get_property(jobject, IOTJS_MAGIC_STRING_HANDLETIMEOUT);
  iotjs_invoke_callback(jcallback, jobject, NULL, 0);
  jerry_release_value(jcallback);
}


uv_timer_t* iotjs_timerwrap_handle(iotjs_timerwrap_t* timerwrap) {
  return (uv_timer_t*)iotjs_handlewrap_get_uv_handle(&timerwrap->handlewrap);
}


jerry_value_t iotjs_timerwrap_jobject(iotjs_timerwrap_t* timerwrap) {
  jerry_value_t jobject = iotjs_handlewrap_jobject(&timerwrap->handlewrap);
  IOTJS_ASSERT(jerry_value_is_object(jobject));
  return jobject;
}


iotjs_timerwrap_t* iotjs_timerwrap_from_handle(uv_timer_t* timer_handle) {
  uv_handle_t* handle = (uv_handle_t*)(timer_handle);
  iotjs_handlewrap_t* handlewrap = iotjs_handlewrap_from_handle(handle);
  iotjs_timerwrap_t* timerwrap = (iotjs_timerwrap_t*)handlewrap;
  IOTJS_ASSERT(iotjs_timerwrap_handle(timerwrap) == timer_handle);
  return timerwrap;
}


iotjs_timerwrap_t* iotjs_timerwrap_from_jobject(const jerry_value_t jtimer) {
  iotjs_handlewrap_t* handlewrap = iotjs_handlewrap_from_jobject(jtimer);
  return (iotjs_timerwrap_t*)handlewrap;
}


JS_FUNCTION(Start) {
  // Check parameters.
  JS_DECLARE_THIS_PTR(timerwrap, timer_wrap);
  DJS_CHECK_ARGS(2, number, number);

  // parameters.
  uint64_t timeout = JS_GET_ARG(0, number);
  uint64_t repeat = JS_GET_ARG(1, number);

  // Start timer.
  int res = iotjs_timerwrap_start(timer_wrap, timeout, repeat);

  return jerry_create_number(res);
}


JS_FUNCTION(Stop) {
  JS_DECLARE_THIS_PTR(timerwrap, timer_wrap);
  // Stop timer.
  int res = iotjs_timerwrap_stop(timer_wrap);

  return jerry_create_number(res);
}


JS_FUNCTION(Timer) {
  JS_CHECK_THIS();

  const jerry_value_t jtimer = JS_GET_THIS();

  iotjs_timerwrap_t* timer_wrap = iotjs_timerwrap_create(jtimer);

  jerry_value_t jobject = iotjs_timerwrap_jobject(timer_wrap);
  IOTJS_ASSERT(jerry_value_is_object(jobject));
  IOTJS_ASSERT(iotjs_jval_get_object_native_handle(jtimer) != 0);

  return jerry_create_undefined();
}


jerry_value_t InitTimer() {
  jerry_value_t timer = jerry_create_external_function(Timer);

  jerry_value_t prototype = jerry_create_object();
  iotjs_jval_set_property_jval(timer, IOTJS_MAGIC_STRING_PROTOTYPE, prototype);

  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_START, Start);
  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_STOP, Stop);

  jerry_release_value(prototype);

  return timer;
}
