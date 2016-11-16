/* Copyright 2015-2016 Samsung Electronics Co., Ltd.
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


typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;
typedef struct sockaddr_in6 sockaddr_in6;
typedef struct sockaddr_storage sockaddr_storage;


void AddressToJS(const iotjs_jval_t* obj, const sockaddr* addr);


#define GetSockNameFunction(wraptype, handletype, function) \
  static void DoGetSockName(iotjs_jhandler_t* jhandler) { \
    JHANDLER_CHECK_ARGS(1, object); \
    \
    iotjs_##wraptype##_t* wrap = \
        iotjs_##wraptype##_from_jobject(JHANDLER_GET_THIS(object)); \
    IOTJS_ASSERT(wrap != NULL); \
    \
    sockaddr_storage storage; \
    int addrlen = sizeof(storage); \
    sockaddr* const addr = (sockaddr*)(&storage); \
    int err = function(iotjs_##wraptype##_##handletype(wrap), addr, &addrlen); \
    if (err == 0) \
      AddressToJS(JHANDLER_GET_ARG(0, object), addr); \
    iotjs_jhandler_return_number(jhandler, err); \
  }


#endif /* IOTJS_MODULE_TCP_H */
