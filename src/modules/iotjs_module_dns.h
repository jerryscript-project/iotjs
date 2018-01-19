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


#ifndef IOTJS_MODULE_DNS_H
#define IOTJS_MODULE_DNS_H


#include "iotjs_def.h"
#include "iotjs_reqwrap.h"


typedef struct {
  iotjs_reqwrap_t reqwrap;
  uv_getaddrinfo_t req;
} iotjs_getaddrinfo_reqwrap_t;

iotjs_getaddrinfo_reqwrap_t* iotjs_getaddrinfo_reqwrap_create(
    const jerry_value_t jcallback);

void iotjs_getaddrinfo_reqwrap_dispatched(
    iotjs_getaddrinfo_reqwrap_t* getaddrinfo_reqwrap);

jerry_value_t iotjs_getaddrinfo_reqwrap_jcallback(
    iotjs_getaddrinfo_reqwrap_t* getaddrinfo_reqwrap);


#endif /* IOTJS_MODULE_DNS_H */
