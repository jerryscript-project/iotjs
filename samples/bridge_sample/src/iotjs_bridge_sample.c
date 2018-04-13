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
#include "modules/iotjs_module_bridge.h"


char* iotjs_bridge_sample_getSystemInfo(const char* message) {
  return "{'OS':'tizen'}";
}

void thread1_worker(void* return_handle) {
  uv_sleep(500);
  iotjs_bridge_set_msg(return_handle, "{'return':'from thread..'}");
}

void iotjs_bridge_sample_func(const char* command, const char* message,
                              void* return_handle) {
  char* result = 0;
  if (strncmp(command, "getSystemInfo", strlen("getSystemInfo")) == 0) {
    result = iotjs_bridge_sample_getSystemInfo(message);
    if (result == 0) {
      iotjs_bridge_set_err(return_handle, "Can't get the resource path");
    } else {
      iotjs_bridge_set_msg(return_handle, result);
    }
  } else if (strncmp(command, "testThread", strlen("testThread")) == 0) {
    uv_thread_t thread1;
    uv_thread_create(&thread1, thread1_worker, return_handle);
    uv_thread_join(&thread1);
  } else if (strncmp(command, "getResPath", strlen("getResPath")) == 0) {
    iotjs_bridge_set_msg(return_handle, "res/");
  } else {
    iotjs_bridge_set_err(return_handle, "Can't find command");
  }
}

/**
 * Init method called by IoT.js
 */
jerry_value_t InitBridgeSample() {
  char* module_name = "bridge_sample";
  jerry_value_t mymodule = jerry_create_object();
  iotjs_jval_set_property_string_raw(mymodule, "MODULE_NAME", module_name);
  iotjs_bridge_register(module_name, iotjs_bridge_sample_func);
  return mymodule;
}
