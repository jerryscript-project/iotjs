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

typedef enum {
  CALL_STATUS_ERROR = 0,
  CALL_STATUS_INIT,
  CALL_STATUS_CALLED,
  CALL_STATUS_SETMSG,
} iotjs_bridge_status_t;

typedef struct _iotjs_bridge_object_t iotjs_bridge_object_t;
typedef struct {
  jerry_value_t jobject;
  jerry_value_t jcallback;
  uv_mutex_t call_lock;
  uv_work_t req;
  uv_async_t* async;
  iotjs_string_t module;
  iotjs_string_t command;
  iotjs_string_t message;
  iotjs_string_t ret_msg;
  iotjs_bridge_status_t status;
  iotjs_bridge_object_t* bridgeobj;
} iotjs_bridge_call_t;

struct _iotjs_bridge_object_t {
  jerry_value_t jobject;
  iotjs_bridge_call_t** calls;
  size_t calls_alloc; // allocated size of calls
};

typedef struct {
  char* module_name;
  iotjs_bridge_func callback;
} relation_info_t;

static relation_info_t* g_module_list = 0;
static unsigned int g_module_count = 0;

IOTJS_DEFINE_NATIVE_HANDLE_INFO_THIS_MODULE(bridge_object);

static unsigned int iotjs_bridge_init() {
  if (g_module_list == 0) {
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

static int iotjs_bridge_call(const char* module_name, const char* command,
                             const char* message, void* handle) {
  int ret = -1;
  for (int i = 0; i < (int)iotjs_module_count; i++) {
    if (g_module_list[i].module_name != 0) {
      if (strncmp(g_module_list[i].module_name, module_name,
                  strlen(module_name) + 1) == 0) {
        g_module_list[i].callback(command, message, handle);
        ret = 0;
        break;
      }
    }
  }
  return ret;
}

void iotjs_bridge_set_err(void* handle, char* err) {
  iotjs_bridge_call_t* bridgecall = (iotjs_bridge_call_t*)handle;
  IOTJS_ASSERT(iotjs_string_is_empty(&bridgecall->ret_msg));

  if (err == NULL) {
    err = "internal error";
  }
  if (!jerry_value_is_undefined(bridgecall->jcallback)) {
    uv_mutex_lock(&bridgecall->call_lock);
  }
  bridgecall->ret_msg = iotjs_string_create_with_size(err, strlen(err) + 1);
  bridgecall->status = CALL_STATUS_ERROR;

  if (bridgecall->async != NULL) {
    IOTJS_ASSERT(bridgecall->async->data == bridgecall);
    uv_async_send(bridgecall->async);
  }
  if (!jerry_value_is_undefined(bridgecall->jcallback)) {
    uv_mutex_unlock(&bridgecall->call_lock);
  }
}

void iotjs_bridge_set_msg(void* handle, char* msg) {
  iotjs_bridge_call_t* bridgecall = (iotjs_bridge_call_t*)handle;
  IOTJS_ASSERT(iotjs_string_is_empty(&bridgecall->ret_msg));
  size_t size = 0;
  if (msg == NULL) {
    msg = "";
  } else {
    size = strlen(msg) + 1;
  }
  if (size > MAX_RETURN_MESSAGE) {
    iotjs_bridge_set_err(handle, "The message exceeds the maximum");
  } else {
    if (!jerry_value_is_undefined(bridgecall->jcallback)) {
      uv_mutex_lock(&bridgecall->call_lock);
    }
    bridgecall->ret_msg = iotjs_string_create_with_size(msg, size);
    bridgecall->status = CALL_STATUS_SETMSG;

    if (bridgecall->async != NULL) {
      IOTJS_ASSERT(bridgecall->async->data == bridgecall);
      uv_async_send(bridgecall->async);
    }
    if (!jerry_value_is_undefined(bridgecall->jcallback)) {
      uv_mutex_unlock(&bridgecall->call_lock);
    }
  }
}

static iotjs_bridge_call_t* iotjs_bridge_call_init(
    iotjs_bridge_call_t* bridgecall, const jerry_value_t bridge,
    const jerry_value_t jcallback, iotjs_string_t module,
    iotjs_string_t command, iotjs_string_t message) {
  if (bridge) {
    bridgecall->jobject = jerry_acquire_value(bridge);
  } else {
    bridgecall->jobject = jerry_create_undefined();
  }
  if (!jerry_value_is_null(jcallback)) {
    bridgecall->jcallback = jerry_acquire_value(jcallback);
    bridgecall->req.data = (void*)bridgecall;
    uv_mutex_init(&bridgecall->call_lock);
  } else {
    bridgecall->jcallback = jerry_create_undefined();
  }
  bridgecall->async = NULL;
  bridgecall->module = module;
  bridgecall->command = command;
  bridgecall->message = message;
  bridgecall->ret_msg = iotjs_string_create();
  bridgecall->status = CALL_STATUS_INIT;
  bridgecall->bridgeobj = NULL;

  return bridgecall;
}

static void iotjs_bridge_call_destroy(iotjs_bridge_call_t* bridgecall) {
  if (!jerry_value_is_undefined(bridgecall->jobject)) {
    jerry_release_value(bridgecall->jobject);
  }
  if (!jerry_value_is_undefined(bridgecall->jcallback)) {
    uv_mutex_destroy(&bridgecall->call_lock);
    jerry_release_value(bridgecall->jcallback);
  }
  if (bridgecall->async) {
    uv_close((uv_handle_t*)bridgecall->async, NULL);
    IOTJS_RELEASE(bridgecall->async);
  }
  iotjs_string_destroy(&bridgecall->module);
  iotjs_string_destroy(&bridgecall->command);
  iotjs_string_destroy(&bridgecall->message);
  iotjs_string_destroy(&bridgecall->ret_msg);
  bridgecall->bridgeobj = NULL;
  IOTJS_RELEASE(bridgecall);
}

static iotjs_bridge_object_t* iotjs_bridge_get_object(jerry_value_t obj_val) {
  iotjs_bridge_object_t* bridgeobj = NULL;
  bool is_ok = false;
  is_ok = jerry_get_object_native_pointer(obj_val, (void**)&bridgeobj, NULL);
  if (!is_ok) {
    bridgeobj = IOTJS_ALLOC(iotjs_bridge_object_t);
    bridgeobj->jobject = obj_val;
    bridgeobj->calls = NULL;
    bridgeobj->calls_alloc = 0;
    jerry_set_object_native_pointer(obj_val, bridgeobj,
                                    &this_module_native_info);
  }
  IOTJS_ASSERT(bridgeobj != NULL);
  IOTJS_ASSERT(bridgeobj->jobject == obj_val);
  return bridgeobj;
}

static void iotjs_bridge_object_destroy(iotjs_bridge_object_t* bridgeobj) {
  if (bridgeobj->calls_alloc == 0) {
    if (bridgeobj->calls != NULL) {
      iotjs_bridge_call_destroy((iotjs_bridge_call_t*)bridgeobj->calls);
    }
  } else {
    for (size_t i = 0; i < bridgeobj->calls_alloc; i++) {
      if (bridgeobj->calls[i] != NULL) {
        iotjs_bridge_call_destroy(bridgeobj->calls[i]);
      }
    }
    IOTJS_ASSERT(bridgeobj->calls);
    iotjs_buffer_release((char*)bridgeobj->calls);
  }
  IOTJS_RELEASE(bridgeobj);
}

static int iotjs_bridge_add_call(iotjs_bridge_object_t* bridgeobj,
                                 iotjs_bridge_call_t* callobj) {
  IOTJS_ASSERT(bridgeobj);
  IOTJS_ASSERT(callobj);
  callobj->bridgeobj = bridgeobj;
  if (bridgeobj->calls_alloc == 0) {
    if (bridgeobj->calls == NULL) {
      bridgeobj->calls = (iotjs_bridge_call_t**)callobj;
    } else {
      iotjs_bridge_call_t* prev_obj = (iotjs_bridge_call_t*)bridgeobj->calls;
      bridgeobj->calls = (iotjs_bridge_call_t**)iotjs_buffer_allocate(
          sizeof(iotjs_bridge_call_t*) * 4);
      bridgeobj->calls_alloc = 4;
      bridgeobj->calls[0] = prev_obj;
      bridgeobj->calls[1] = callobj;
    }
  } else {
    for (size_t i = 0; i < bridgeobj->calls_alloc; i++) {
      if (bridgeobj->calls[i] == 0) {
        bridgeobj->calls[i] = callobj;
        return bridgeobj->calls_alloc;
      }
    }
    size_t prev_size = sizeof(iotjs_bridge_call_t*) * bridgeobj->calls_alloc;
    bridgeobj->calls =
        (iotjs_bridge_call_t**)iotjs_buffer_reallocate((char*)bridgeobj->calls,
                                                       prev_size * 2);
    bridgeobj->calls[bridgeobj->calls_alloc] = callobj;
    bridgeobj->calls_alloc *= 2;
  }
  return bridgeobj->calls_alloc;
}

static int iotjs_bridge_remove_call(iotjs_bridge_call_t* callobj) {
  iotjs_bridge_object_t* bridgeobj = callobj->bridgeobj;

  if (bridgeobj->calls_alloc == 0) {
    if (bridgeobj->calls != NULL) {
      iotjs_bridge_call_destroy((iotjs_bridge_call_t*)bridgeobj->calls);
      bridgeobj->calls = NULL;
    }
  } else {
    for (size_t i = 0; i < bridgeobj->calls_alloc; i++) {
      if (bridgeobj->calls[i] == callobj) {
        iotjs_bridge_call_destroy(bridgeobj->calls[i]);
        bridgeobj->calls[i] = NULL;
      }
    }
  }
  return 0;
}

static void iotjs_bridge_js_call(iotjs_bridge_call_t* bridgecall) {
  jerry_value_t jargs[2] = { 0 };
  if (bridgecall->status == CALL_STATUS_ERROR) { // internal error
    jargs[0] = iotjs_jval_create_error_without_error_flag(
        iotjs_string_data(&bridgecall->ret_msg));
    jargs[1] = jerry_create_null();
  } else {
    jargs[0] = jerry_create_null();
    jargs[1] = jerry_create_string_from_utf8(
        (const jerry_char_t*)iotjs_string_data(&bridgecall->ret_msg));
  }
  jerry_value_t jcallback = bridgecall->jcallback;
  if (jerry_value_is_function(jcallback)) {
    iotjs_invoke_callback(jcallback, jerry_create_undefined(), jargs, 2);
  }
  jerry_release_value(jargs[0]);
  jerry_release_value(jargs[1]);
}

static void aysnc_callback(uv_async_t* async) {
  iotjs_bridge_call_t* bridgecall = (iotjs_bridge_call_t*)async->data;
  iotjs_bridge_js_call(bridgecall);
  iotjs_bridge_remove_call(bridgecall);
}

void after_worker(uv_work_t* req, int status) {
  iotjs_bridge_call_t* bridgecall = (iotjs_bridge_call_t*)req->data;
  uv_mutex_lock(&bridgecall->call_lock);
  if ((bridgecall->status == CALL_STATUS_ERROR) ||
      (bridgecall->status == CALL_STATUS_SETMSG)) {
    iotjs_bridge_js_call(bridgecall);
    uv_mutex_unlock(&bridgecall->call_lock);
    iotjs_bridge_remove_call(bridgecall);
  } else {
    uv_loop_t* loop = iotjs_environment_loop(iotjs_environment_get());
    uv_async_t* async = IOTJS_ALLOC(uv_async_t);
    bridgecall->async = async;
    async->data = (void*)bridgecall;
    uv_async_init(loop, async, aysnc_callback);
    uv_mutex_unlock(&bridgecall->call_lock);
  }
}

void bridge_worker(uv_work_t* req) {
  iotjs_bridge_call_t* bridgecall = (iotjs_bridge_call_t*)req->data;
  bridgecall->status = CALL_STATUS_CALLED;
  int ret = iotjs_bridge_call(iotjs_string_data(&bridgecall->module),
                              iotjs_string_data(&bridgecall->command),
                              iotjs_string_data(&bridgecall->message),
                              (void*)bridgecall);
  if (ret < 0) {
    iotjs_bridge_set_err(bridgecall, "Can't find the module");
  }
}

/**
 * send async message
 */
JS_FUNCTION(MessageAsync) {
  DJS_CHECK_THIS();
  DJS_CHECK_ARGS(3, string, string, string);
  DJS_CHECK_ARG_IF_EXIST(3, function);

  jerry_value_t bridge_module = JS_GET_THIS();
  iotjs_string_t module_name = JS_GET_ARG(0, string);
  iotjs_string_t module_command = JS_GET_ARG(1, string);
  iotjs_string_t command_message = JS_GET_ARG(2, string);
  jerry_value_t jcallback = JS_GET_ARG_IF_EXIST(3, function);

  if (!jerry_value_is_null(jcallback)) { // async call
    uv_loop_t* loop = iotjs_environment_loop(iotjs_environment_get());
    iotjs_bridge_object_t* bridgeobj = iotjs_bridge_get_object(bridge_module);
    iotjs_bridge_call_t* bridgecall = IOTJS_ALLOC(iotjs_bridge_call_t);
    iotjs_bridge_call_init(bridgecall, bridge_module, jcallback, module_name,
                           module_command, command_message);
    iotjs_bridge_add_call(bridgeobj, bridgecall);

    uv_queue_work(loop, &bridgecall->req, bridge_worker, after_worker);

  } else { // sync call
    jerry_value_t jmsg;
    iotjs_bridge_call_t bridgecall_local;
    iotjs_bridge_call_t* bridgecall = &bridgecall_local;

    iotjs_bridge_call_init(bridgecall, 0, 0, module_name, module_command,
                           command_message);
    int ret = iotjs_bridge_call(iotjs_string_data(&module_name),
                                iotjs_string_data(&module_command),
                                iotjs_string_data(&command_message),
                                (void*)bridgecall);
    if (ret < 0) {
      iotjs_bridge_set_err(bridgecall, "Can't find the module");
    }
    if (bridgecall->status == CALL_STATUS_ERROR) { // error..
      if (iotjs_string_is_empty(&bridgecall->ret_msg)) {
        jmsg = JS_CREATE_ERROR(COMMON, (jerry_char_t*)"Unknown native error..");
      } else {
        jmsg = JS_CREATE_ERROR(COMMON, iotjs_string_data(&bridgecall->ret_msg));
      }
    } else {
      if (iotjs_string_is_empty(&bridgecall->ret_msg)) {
        jmsg = jerry_create_string((jerry_char_t*)"");
      } else {
        jmsg = jerry_create_string(
            (jerry_char_t*)iotjs_string_data(&bridgecall->ret_msg));
      }
    }
    iotjs_string_destroy(&bridgecall->module);
    iotjs_string_destroy(&bridgecall->command);
    iotjs_string_destroy(&bridgecall->message);
    iotjs_string_destroy(&bridgecall->ret_msg);
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
