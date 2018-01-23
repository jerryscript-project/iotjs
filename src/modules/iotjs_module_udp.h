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


#ifndef IOTJS_MODULE_UDP_H
#define IOTJS_MODULE_UDP_H


#include "iotjs_def.h"
#include "iotjs_handlewrap.h"
#include "iotjs_reqwrap.h"


typedef struct {
  iotjs_handlewrap_t handlewrap;
  uv_udp_t handle;
} iotjs_udpwrap_t;


iotjs_udpwrap_t* iotjs_udpwrap_create(jerry_value_t judp);

iotjs_udpwrap_t* iotjs_udpwrap_from_handle(uv_udp_t* handle);
iotjs_udpwrap_t* iotjs_udpwrap_from_jobject(jerry_value_t judp);

uv_udp_t* iotjs_udpwrap_udp_handle(iotjs_udpwrap_t* udpwrap);


typedef struct {
  iotjs_reqwrap_t reqwrap;
  uv_udp_send_t req;
  size_t msg_size;
} iotjs_send_reqwrap_t;

iotjs_send_reqwrap_t* iotjs_send_reqwrap_create(jerry_value_t jcallback,
                                                const size_t msg_size);


#endif /* IOTJS_MODULE_UDP_H */
