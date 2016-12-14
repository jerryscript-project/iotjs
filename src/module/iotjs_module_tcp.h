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
} IOTJS_VALIDATED_STRUCT(iotjs_tcpwrap_t);


iotjs_tcpwrap_t* iotjs_tcpwrap_create(const iotjs_jval_t* jtcp);

iotjs_tcpwrap_t* iotjs_tcpwrap_from_handle(uv_tcp_t* handle);
iotjs_tcpwrap_t* iotjs_tcpwrap_from_jobject(const iotjs_jval_t* jtcp);

uv_tcp_t* iotjs_tcpwrap_tcp_handle(iotjs_tcpwrap_t* tcpwrap);
iotjs_jval_t* iotjs_tcpwrap_jobject(iotjs_tcpwrap_t* tcpwrap);


typedef struct {
  iotjs_reqwrap_t reqwrap;
  uv_connect_t req;
} IOTJS_VALIDATED_STRUCT(iotjs_connect_reqwrap_t);

#define THIS iotjs_connect_reqwrap_t* connect_reqwrap
iotjs_connect_reqwrap_t* iotjs_connect_reqwrap_create(
    const iotjs_jval_t* jcallback);
void iotjs_connect_reqwrap_dispatched(THIS);
uv_connect_t* iotjs_connect_reqwrap_req(THIS);
const iotjs_jval_t* iotjs_connect_reqwrap_jcallback(THIS);
#undef THIS


typedef struct {
  iotjs_reqwrap_t reqwrap;
  uv_write_t req;
} IOTJS_VALIDATED_STRUCT(iotjs_write_reqwrap_t);

#define THIS iotjs_write_reqwrap_t* write_reqwrap
iotjs_write_reqwrap_t* iotjs_write_reqwrap_create(
    const iotjs_jval_t* jcallback);
void iotjs_write_reqwrap_dispatched(THIS);
uv_write_t* iotjs_write_reqwrap_req(THIS);
const iotjs_jval_t* iotjs_write_reqwrap_jcallback(THIS);
#undef THIS


typedef struct {
  iotjs_reqwrap_t reqwrap;
  uv_shutdown_t req;
} IOTJS_VALIDATED_STRUCT(iotjs_shutdown_reqwrap_t);

#define THIS iotjs_shutdown_reqwrap_t* shutdown_reqwrap
iotjs_shutdown_reqwrap_t* iotjs_shutdown_reqwrap_create(
    const iotjs_jval_t* jcallback);
void iotjs_shutdown_reqwrap_dispatched(THIS);
uv_shutdown_t* iotjs_shutdown_reqwrap_req(THIS);
const iotjs_jval_t* iotjs_shutdown_reqwrap_jcallback(THIS);
#undef THIS


void AddressToJS(const iotjs_jval_t* obj, const sockaddr* addr);


#define GetSockNameFunction(wraptype, handletype, function)                    \
  static void DoGetSockName(iotjs_jhandler_t* jhandler) {                      \
    JHANDLER_CHECK_ARGS(1, object);                                            \
                                                                               \
    iotjs_##wraptype##_t* wrap =                                               \
        iotjs_##wraptype##_from_jobject(JHANDLER_GET_THIS(object));            \
    IOTJS_ASSERT(wrap != NULL);                                                \
                                                                               \
    sockaddr_storage storage;                                                  \
    int addrlen = sizeof(storage);                                             \
    sockaddr* const addr = (sockaddr*)(&storage);                              \
    int err = function(iotjs_##wraptype##_##handletype(wrap), addr, &addrlen); \
    if (err == 0)                                                              \
      AddressToJS(JHANDLER_GET_ARG(0, object), addr);                          \
    iotjs_jhandler_return_number(jhandler, err);                               \
  }


#endif /* IOTJS_MODULE_TCP_H */
