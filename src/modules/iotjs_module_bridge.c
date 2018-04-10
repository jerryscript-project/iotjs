/* Copyright 2018-present Samsung Electronics Co., Ltd. and other contributors
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
#include "iotjs_module_bridge.h"
#include "iotjs_reqwrap.h"
#include <stdio.h>


typedef struct {
  iotjs_reqwrap_t reqwrap;
  uv_work_t req;
  iotjs_string_t module;
  iotjs_string_t command;
  iotjs_string_t message;
  iotjs_string_t ret_msg;
  int err_flag;
} iotjs_module_msg_reqwrap_t;

typedef struct {
  char* module_name;
  iotjs_bridge_func callback;
} relation_info_t;

static relation_info_t* g_module_list = 0;
static unsigned int g_module_count = 0;

unsigned int iotjs_bridge_init() {
  if (g_module_list == 0) {
    // printf("__FUNCTION___ : %s(%d) module_count: %d \n",
    //     __FUNCTION__, __LINE__, iotjs_module_count);
    g_module_list = (relation_info_t*)iotjs_buffer_allocate(
        sizeof(relation_info_t) * iotjs_module_count);
    IOTJS_ASSERT(g_module_list);
  }
  return iotjs_module_count;
}

int iotjs_bridge_register(char* module_name, iotjs_bridge_func callback) {
  int empty_slot = -1;
  iotjs_bridge_init();
  for (int i = 0; i < (int)iotjs_module_count; i++) {
    if (g_module_list[i].module_name == 0) {
      if (empty_slot == -1)
        empty_slot = i;
    } else {
      if (strncmp(g_module_list[i].module_name, module_name,
                  strlen(module_name)) == 0) {
        return i;
      }
    }
  }
  if (empty_slot != -1) {
    g_module_list[empty_slot].module_name =
        iotjs_buffer_allocate(strlen(module_name) + 1);
    IOTJS_ASSERT(g_module_list[empty_slot].module_name);
    strncpy(g_module_list[empty_slot].module_name, module_name,
            strlen(module_name));
    g_module_list[empty_slot].callback = callback;
    g_module_count++;
  }
  return empty_slot;
}

int iotjs_bridge_call(const char* module_name, const char* command,
                      const char* message, char** return_message) {
  int ret = -1;
  for (int i = 0; i < (int)iotjs_module_count; i++) {
    if (g_module_list[i].module_name != 0) {
      if (strncmp(g_module_list[i].module_name, module_name,
                  strlen(module_name) + 1) == 0) {
        ret = g_module_list[i].callback(command, message, return_message);
        break;
      }
    }
  }
  return ret;
}

int iotjs_bridge_set_return(char** return_message, char* result) {
  if (result == NULL) {
    *return_message = NULL;
  } else {
    *return_message =
        iotjs_buffer_allocate(sizeof(char) * (strlen(result) + 1));
    IOTJS_ASSERT(*return_message);
    strncpy(*return_message, result, strlen(result) + 1);
  }
  return 0;
}

static iotjs_module_msg_reqwrap_t* iotjs_module_msg_reqwrap_create(
    const jerry_value_t jcallback, iotjs_string_t module,
    iotjs_string_t command, iotjs_string_t message) {
  iotjs_module_msg_reqwrap_t* module_msg_reqwrap =
      IOTJS_ALLOC(iotjs_module_msg_reqwrap_t);

  iotjs_reqwrap_initialize(&module_msg_reqwrap->reqwrap, jcallback,
                           (uv_req_t*)&module_msg_reqwrap->req);

  module_msg_reqwrap->module = module;
  module_msg_reqwrap->command = command;
  module_msg_reqwrap->message = message;
  module_msg_reqwrap->ret_msg = iotjs_string_create();
  module_msg_reqwrap->err_flag = 0;
  return module_msg_reqwrap;
}

static void after_worker(uv_work_t* work_req, int status) {
  iotjs_module_msg_reqwrap_t* req_wrap =
      (iotjs_module_msg_reqwrap_t*)iotjs_reqwrap_from_request(
          (uv_req_t*)work_req);
  iotjs_jargs_t jargs = iotjs_jargs_create(2);

  if (status) {
    iotjs_jargs_append_error(&jargs, "System error");
  } else {
    // internal error
    if (req_wrap->err_flag) {
      iotjs_jargs_append_error(&jargs, iotjs_string_data(&req_wrap->ret_msg));
      iotjs_jargs_append_null(&jargs);
    } else {
      iotjs_jargs_append_null(&jargs);
      iotjs_jargs_append_string_raw(&jargs,
                                    iotjs_string_data(&req_wrap->ret_msg));
    }
  }

  jerry_value_t jcallback = iotjs_reqwrap_jcallback(&req_wrap->reqwrap);
  if (jerry_value_is_function(jcallback)) {
    iotjs_make_callback(jcallback, jerry_create_undefined(), &jargs);
  }

  iotjs_string_destroy(&req_wrap->ret_msg);
  iotjs_jargs_destroy(&jargs);
  iotjs_reqwrap_destroy(&req_wrap->reqwrap);
  IOTJS_RELEASE(req_wrap);
}

static void module_msg_worker(uv_work_t* work_req) {
  iotjs_module_msg_reqwrap_t* req_wrap =
      (iotjs_module_msg_reqwrap_t*)iotjs_reqwrap_from_request(
          (uv_req_t*)work_req);

  char* return_message = NULL;
  int return_value = -1;

  return_value =
      iotjs_bridge_call(iotjs_string_data(&req_wrap->module),
                        iotjs_string_data(&req_wrap->command),
                        iotjs_string_data(&req_wrap->message), &return_message);

  if (return_value < 0) { // error..
    req_wrap->err_flag = 1;
  }

  if (return_message != NULL) {
    int message_size = strlen(return_message);
    if (message_size > MAX_RETURN_MESSAGE) {
      req_wrap->err_flag = 1;
      req_wrap->ret_msg =
          iotjs_string_create_with_size("invalid return_message",
                                        strlen("invalid return_message") + 1);
    } else {
      req_wrap->ret_msg =
          iotjs_string_create_with_buffer(return_message,
                                          strlen(return_message));
    }
  }

  iotjs_string_destroy(&req_wrap->module);
  iotjs_string_destroy(&req_wrap->command);
  iotjs_string_destroy(&req_wrap->message);
}

/**
 * send async message
 */
JS_FUNCTION(MessageAsync) {
  DJS_CHECK_THIS();
  DJS_CHECK_ARGS(3, string, string, string);
  DJS_CHECK_ARG_IF_EXIST(3, function);

  iotjs_string_t module_name = JS_GET_ARG(0, string);
  iotjs_string_t module_command = JS_GET_ARG(1, string);
  iotjs_string_t command_message = JS_GET_ARG(2, string);
  jerry_value_t jcallback = JS_GET_ARG_IF_EXIST(3, function);

  if (!jerry_value_is_null(jcallback)) { // async call
    uv_loop_t* loop = iotjs_environment_loop(iotjs_environment_get());
    iotjs_module_msg_reqwrap_t* req_wrap =
        iotjs_module_msg_reqwrap_create(jcallback, module_name, module_command,
                                        command_message);
    uv_queue_work(loop, &req_wrap->req, module_msg_worker, after_worker);
  } else { // sync call
    jerry_value_t jmsg;
    int return_value;
    char* return_message = NULL;

    return_value =
        iotjs_bridge_call(iotjs_string_data(&module_name),
                          iotjs_string_data(&module_command),
                          iotjs_string_data(&command_message), &return_message);

    if (return_value < 0) { // error..
      if (return_message != NULL) {
        jmsg = JS_CREATE_ERROR(COMMON, return_message);
        iotjs_buffer_release(return_message);
      } else {
        jmsg = JS_CREATE_ERROR(COMMON, (jerry_char_t*)"Unknown native error..");
      }
    } else {
      if (return_message != NULL) {
        jmsg = jerry_create_string((jerry_char_t*)return_message);
        iotjs_buffer_release(return_message);
      } else {
        jmsg = jerry_create_string((jerry_char_t*)"");
      }
    }
    iotjs_string_destroy(&module_name);
    iotjs_string_destroy(&module_command);
    iotjs_string_destroy(&command_message);
    return jmsg;
  }

  return jerry_create_string((jerry_char_t*)"");
}

/**
 * Init method called by IoT.js
 */
jerry_value_t InitBridge() {
  jerry_value_t messagModule = jerry_create_object();
  iotjs_jval_set_method(messagModule, "send", MessageAsync);
  iotjs_bridge_init();
  return messagModule;
}
