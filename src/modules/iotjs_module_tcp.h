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


#ifndef IOTJS_MODULE_TCP_H
#define IOTJS_MODULE_TCP_H


#include "iotjs_binding.h"
#include "iotjs_handlewrap.h"
#include "iotjs_reqwrap.h"


typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;
typedef struct sockaddr_in6 sockaddr_in6;
typedef struct sockaddr_storage sockaddr_storage;


typedef struct {
  iotjs_handlewrap_t handlewrap;
  uv_tcp_t handle;
} iotjs_tcpwrap_t;


iotjs_tcpwrap_t* iotjs_tcpwrap_create(jerry_value_t jtcp);

iotjs_tcpwrap_t* iotjs_tcpwrap_from_handle(uv_tcp_t* handle);
iotjs_tcpwrap_t* iotjs_tcpwrap_from_jobject(jerry_value_t jtcp);

uv_tcp_t* iotjs_tcpwrap_tcp_handle(iotjs_tcpwrap_t* tcpwrap);

void AddressToJS(jerry_value_t obj, const sockaddr* addr);


#endif /* IOTJS_MODULE_TCP_H */
