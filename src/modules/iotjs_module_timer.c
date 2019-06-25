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
#include "iotjs_uv_handle.h"


static const jerry_object_native_info_t this_module_native_info = { NULL };


void iotjs_timer_object_init(jerry_value_t jtimer) {
  uv_handle_t* handle = iotjs_uv_handle_create(sizeof(uv_timer_t), jtimer,
                                               &this_module_native_info, 0);

  const iotjs_environment_t* env = iotjs_environment_get();
  uv_timer_init(iotjs_environment_loop(env), (uv_timer_t*)handle);
}


static void timeout_handler(uv_timer_t* handle) {
  IOTJS_ASSERT(handle != NULL);

  jerry_value_t jobject = IOTJS_UV_HANDLE_DATA(handle)->jobject;
  jerry_value_t jcallback =
      iotjs_jval_get_property(jobject, IOTJS_MAGIC_STRING_HANDLETIMEOUT);
  iotjs_invoke_callback(jcallback, jobject, NULL, 0);
  jerry_release_value(jcallback);
}


JS_FUNCTION(timer_start) {
  // Check parameters.
  JS_DECLARE_PTR(jthis, uv_timer_t, timer_handle);
  DJS_CHECK_ARGS(2, number, number);

  // parameters.
  uint64_t timeout = JS_GET_ARG(0, number);
  uint64_t repeat = JS_GET_ARG(1, number);

  // Start timer.
  int res = uv_timer_start(timer_handle, timeout_handler, timeout, repeat);

  return jerry_create_number(res);
}


JS_FUNCTION(timer_stop) {
  JS_DECLARE_PTR(jthis, uv_handle_t, timer_handle);
  // Stop timer.

  if (!uv_is_closing(timer_handle)) {
    iotjs_uv_handle_close(timer_handle, NULL);
  }

  return jerry_create_number(0);
}


JS_FUNCTION(timer_constructor) {
  DJS_CHECK_THIS();

  const jerry_value_t jtimer = JS_GET_THIS();

  iotjs_timer_object_init(jtimer);
  return jerry_create_undefined();
}


jerry_value_t iotjs_init_timer(void) {
  jerry_value_t timer = jerry_create_external_function(timer_constructor);

  jerry_value_t prototype = jerry_create_object();
  iotjs_jval_set_property_jval(timer, IOTJS_MAGIC_STRING_PROTOTYPE, prototype);

  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_START, timer_start);
  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_STOP, timer_stop);

  jerry_release_value(prototype);

  return timer;
}
