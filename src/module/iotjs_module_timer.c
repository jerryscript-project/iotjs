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


iotjs_timerwrap_t* iotjs_timerwrap_create(const iotjs_jval_t* jtimer) {
  iotjs_timerwrap_t* timerwrap = IOTJS_ALLOC(iotjs_timerwrap_t);
  IOTJS_VALIDATED_STRUCT_CONSTRUCTOR(iotjs_timerwrap_t, timerwrap);

  iotjs_handlewrap_initialize(&_this->handlewrap, jtimer,
                              (uv_handle_t*)(&_this->handle),
                              (JFreeHandlerType)iotjs_timerwrap_destroy);

  // Initialize timer handler.
  const iotjs_environment_t* env = iotjs_environment_get();
  uv_timer_init(iotjs_environment_loop(env), &_this->handle);

  return timerwrap;
}


static void iotjs_timerwrap_destroy(iotjs_timerwrap_t* timerwrap) {
  IOTJS_VALIDATED_STRUCT_DESTRUCTOR(iotjs_timerwrap_t, timerwrap);
  iotjs_handlewrap_destroy(&_this->handlewrap);

  IOTJS_RELEASE(timerwrap);
}


// This function is called from uv when timeout expires.
static void TimeoutHandler(uv_timer_t* handle) {
  // Find timer wrap from handle.
  iotjs_timerwrap_t* timer_wrap = iotjs_timerwrap_from_handle(handle);

  // Call the timeout handler.
  iotjs_timerwrap_on_timeout(timer_wrap);
}


int iotjs_timerwrap_start(iotjs_timerwrap_t* timerwrap, int64_t timeout,
                          int64_t repeat) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_timerwrap_t, timerwrap);

  // Start uv timer.
  return uv_timer_start(&_this->handle, TimeoutHandler, timeout, repeat);
}


int iotjs_timerwrap_stop(iotjs_timerwrap_t* timerwrap) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_timerwrap_t, timerwrap);

  if (!uv_is_closing(iotjs_handlewrap_get_uv_handle(&_this->handlewrap))) {
    iotjs_handlewrap_close(&_this->handlewrap, NULL);
  }

  return 0;
}


static void iotjs_timerwrap_on_timeout(iotjs_timerwrap_t* timerwrap) {
  const IOTJS_VALIDATED_STRUCT_METHOD(iotjs_timerwrap_t, timerwrap);

  // Call javascript timeout handler function.
  const iotjs_jval_t* jobject = iotjs_timerwrap_jobject(timerwrap);
  iotjs_jval_t jcallback = iotjs_jval_get_property(jobject, "handleTimeout");
  iotjs_make_callback(&jcallback, jobject, iotjs_jargs_get_empty());
  iotjs_jval_destroy(&jcallback);
}


uv_timer_t* iotjs_timerwrap_handle(iotjs_timerwrap_t* timerwrap) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_timerwrap_t, timerwrap);
  return &_this->handle;
}


iotjs_jval_t* iotjs_timerwrap_jobject(iotjs_timerwrap_t* timerwrap) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_timerwrap_t, timerwrap);
  iotjs_jval_t* jobject = iotjs_handlewrap_jobject(&_this->handlewrap);
  IOTJS_ASSERT(iotjs_jval_is_object(jobject));
  return jobject;
}


iotjs_timerwrap_t* iotjs_timerwrap_from_handle(uv_timer_t* timer_handle) {
  uv_handle_t* handle = (uv_handle_t*)(timer_handle);
  iotjs_handlewrap_t* handlewrap = iotjs_handlewrap_from_handle(handle);
  iotjs_timerwrap_t* timerwrap = (iotjs_timerwrap_t*)handlewrap;
  IOTJS_ASSERT(iotjs_timerwrap_handle(timerwrap) == timer_handle);
  return timerwrap;
}


iotjs_timerwrap_t* iotjs_timerwrap_from_jobject(const iotjs_jval_t* jtimer) {
  iotjs_handlewrap_t* handlewrap = iotjs_handlewrap_from_jobject(jtimer);
  return (iotjs_timerwrap_t*)handlewrap;
}


JHANDLER_FUNCTION(Start) {
  // Check parameters.
  JHANDLER_CHECK_THIS(object);
  JHANDLER_CHECK_ARGS(2, number, number);

  const iotjs_jval_t* jtimer = JHANDLER_GET_THIS(object);

  // Take timer wrap.
  iotjs_timerwrap_t* timer_wrap = iotjs_timerwrap_from_jobject(jtimer);

  // parameters.
  int64_t timeout = JHANDLER_GET_ARG(0, number);
  int64_t repeat = JHANDLER_GET_ARG(1, number);

  // Start timer.
  int res = iotjs_timerwrap_start(timer_wrap, timeout, repeat);

  iotjs_jhandler_return_number(jhandler, res);
}


JHANDLER_FUNCTION(Stop) {
  JHANDLER_CHECK_THIS(object);

  const iotjs_jval_t* jtimer = JHANDLER_GET_THIS(object);

  iotjs_timerwrap_t* timer_wrap = iotjs_timerwrap_from_jobject(jtimer);

  // Stop timer.
  int res = iotjs_timerwrap_stop(timer_wrap);

  iotjs_jhandler_return_number(jhandler, res);
}


JHANDLER_FUNCTION(Timer) {
  JHANDLER_CHECK_THIS(object);

  const iotjs_jval_t* jtimer = JHANDLER_GET_THIS(object);

  iotjs_timerwrap_t* timer_wrap = iotjs_timerwrap_create(jtimer);
  IOTJS_ASSERT(iotjs_jval_is_object(iotjs_timerwrap_jobject(timer_wrap)));
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
