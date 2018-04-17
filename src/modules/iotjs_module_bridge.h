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

#ifndef IOTJS_BRIDGE_H
#define IOTJS_BRIDGE_H

#define MAX_RETURN_MESSAGE 512 * 2

/*
 */
typedef void (*iotjs_bridge_func)(const char* command, const char* message,
                                  void* return_handle);

int iotjs_bridge_register(char* module_name, iotjs_bridge_func callback);

void iotjs_bridge_set_err(void* return_handle, char* err);
void iotjs_bridge_set_msg(void* return_handle, char* msg);

#endif
