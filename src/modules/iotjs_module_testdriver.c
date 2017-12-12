/* Copyright 2016-present Samsung Electronics Co., Ltd. and other contributors
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


// Only for test driver
JS_FUNCTION(IsAliveExceptFor) {
  JS_CHECK(jargc == 1);
  const iotjs_environment_t* env = iotjs_environment_get();
  uv_loop_t* loop = iotjs_environment_loop(env);

  if (jerry_value_is_null(jargv[0])) {
    int alive = uv_loop_alive(loop);

    return jerry_create_boolean(alive);
  } else {
    JS_CHECK(jerry_value_is_object(jargv[0]));

    jerry_value_t jtimer =
        iotjs_jval_get_property(jargv[0], IOTJS_MAGIC_STRING_HANDLER);

    iotjs_timerwrap_t* timer_wrap = iotjs_timerwrap_from_jobject(jtimer);
    jerry_release_value(jtimer);

    bool has_active_reqs = uv_loop_has_active_reqs(loop);
    bool has_closing_handler = loop->closing_handles != NULL;

    bool ret = true;
    bool alive = !has_active_reqs && !has_closing_handler;
    if (alive) {
      unsigned int active_handlers = loop->active_handles;
      if (active_handlers == 1u) {
        const uv_timer_t* timer_handle = iotjs_timerwrap_handle(timer_wrap);
        int timer_alive = uv_is_active((uv_handle_t*)timer_handle);

        if (timer_alive) {
          // If the timer handler we set for test driver is alive,
          // then it can be safely terminated.
          ret = false;
        }
      }
    }

    return jerry_create_boolean(ret);
  }
}


jerry_value_t InitTestdriver() {
  jerry_value_t testdriver = jerry_create_object();
  iotjs_jval_set_method(testdriver, IOTJS_MAGIC_STRING_ISALIVEEXCEPTFOR,
                        IsAliveExceptFor);

  return testdriver;
}
