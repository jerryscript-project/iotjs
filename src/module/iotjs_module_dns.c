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

#include "iotjs_def.h"

#include "iotjs_module_dns.h"

#include "iotjs_reqwrap.h"
#include "uv.h"


#define THIS iotjs_getaddrinfo_reqwrap_t* getaddrinfo_reqwrap

iotjs_getaddrinfo_reqwrap_t* iotjs_getaddrinfo_reqwrap_create(
    const iotjs_jval_t* jcallback) {
  iotjs_getaddrinfo_reqwrap_t* getaddrinfo_reqwrap =
      IOTJS_ALLOC(iotjs_getaddrinfo_reqwrap_t);
  IOTJS_VALIDATED_STRUCT_CONSTRUCTOR(iotjs_getaddrinfo_reqwrap_t,
                                     getaddrinfo_reqwrap);
  iotjs_reqwrap_initialize(&_this->reqwrap, jcallback, (uv_req_t*)&_this->req);
  return getaddrinfo_reqwrap;
}


static void iotjs_getaddrinfo_reqwrap_destroy(THIS) {
  IOTJS_VALIDATED_STRUCT_DESTRUCTOR(iotjs_getaddrinfo_reqwrap_t,
                                    getaddrinfo_reqwrap);
  iotjs_reqwrap_destroy(&_this->reqwrap);
  IOTJS_RELEASE(getaddrinfo_reqwrap);
}


void iotjs_getaddrinfo_reqwrap_dispatched(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_getaddrinfo_reqwrap_t,
                                getaddrinfo_reqwrap);
  iotjs_getaddrinfo_reqwrap_destroy(getaddrinfo_reqwrap);
}


uv_getaddrinfo_t* iotjs_getaddrinfo_reqwrap_req(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_getaddrinfo_reqwrap_t,
                                getaddrinfo_reqwrap);
  return &_this->req;
}


const iotjs_jval_t* iotjs_getaddrinfo_reqwrap_jcallback(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_getaddrinfo_reqwrap_t,
                                getaddrinfo_reqwrap);
  return iotjs_reqwrap_jcallback(&_this->reqwrap);
}

#undef THIS


#if !defined(__NUTTX__)
static void AfterGetAddrInfo(uv_getaddrinfo_t* req, int status,
                             struct addrinfo* res) {
  iotjs_getaddrinfo_reqwrap_t* req_wrap =
      (iotjs_getaddrinfo_reqwrap_t*)(req->data);

  iotjs_jargs_t args = iotjs_jargs_create(3);
  iotjs_jargs_append_number(&args, status);

  if (status == 0) {
    char ip[INET6_ADDRSTRLEN];
    const char* addr;

    // Only first address is used
    if (res->ai_family == AF_INET) {
      struct sockaddr_in* sockaddr = (struct sockaddr_in*)(res->ai_addr);
      addr = (char*)(&(sockaddr->sin_addr));
    } else {
      struct sockaddr_in6* sockaddr = (struct sockaddr_in6*)(res->ai_addr);
      addr = (char*)(&(sockaddr->sin6_addr));
    }

    int err = uv_inet_ntop(res->ai_family, addr, ip, INET6_ADDRSTRLEN);
    if (err) {
      ip[0] = 0;
    }

    iotjs_jargs_append_string_raw(&args, ip);
  }

  uv_freeaddrinfo(res);

  // Make the callback into JavaScript
  iotjs_make_callback(iotjs_getaddrinfo_reqwrap_jcallback(req_wrap),
                      iotjs_jval_get_undefined(), &args);

  iotjs_jargs_destroy(&args);

  iotjs_getaddrinfo_reqwrap_dispatched(req_wrap);
}
#endif


JHANDLER_FUNCTION(GetAddrInfo) {
  JHANDLER_CHECK_THIS(object);
  JHANDLER_CHECK_ARGS(4, string, number, number, function);

  iotjs_string_t hostname = JHANDLER_GET_ARG(0, string);
  int option = JHANDLER_GET_ARG(1, number);
  int flags = JHANDLER_GET_ARG(2, number);
  const iotjs_jval_t* jcallback = JHANDLER_GET_ARG(3, function);

  int family;
  if (option == 0) {
    family = AF_UNSPEC;
  } else if (option == 4) {
    family = AF_INET;
  } else if (option == 6) {
    family = AF_INET6;
  } else {
    JHANDLER_THROW(TYPE, "bad address family");
    return;
  }

#if defined(__NUTTX__)
  iotjs_jargs_t args = iotjs_jargs_create(3);
  int err = 0;
  char ip[INET6_ADDRSTRLEN];
  const char* hostname_data = iotjs_string_data(&hostname);

  if (strcmp(hostname_data, "localhost") == 0) {
    strcpy(ip, "127.0.0.1");
  } else {
    struct sockaddr_in addr;
    int result = inet_pton(AF_INET, hostname_data, &(addr.sin_addr));

    if (result != 1) {
      err = errno;
    } else {
      inet_ntop(AF_INET, &(addr.sin_addr), ip, INET6_ADDRSTRLEN);
    }
  }

  iotjs_jargs_append_number(&args, err);
  iotjs_jargs_append_string_raw(&args, ip);
  iotjs_jargs_append_number(&args, family);

  iotjs_make_callback(jcallback, iotjs_jval_get_undefined(), &args);
  iotjs_jargs_destroy(&args);
#else
  iotjs_getaddrinfo_reqwrap_t* req_wrap =
      iotjs_getaddrinfo_reqwrap_create(jcallback);

  struct addrinfo hints = { 0 };
  hints.ai_family = family;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = flags;

  int err =
      uv_getaddrinfo(iotjs_environment_loop(iotjs_environment_get()),
                     iotjs_getaddrinfo_reqwrap_req(req_wrap), AfterGetAddrInfo,
                     iotjs_string_data(&hostname), NULL, &hints);

  if (err) {
    iotjs_getaddrinfo_reqwrap_dispatched(req_wrap);
  }
#endif

  iotjs_jhandler_return_number(jhandler, err);

  iotjs_string_destroy(&hostname);
}


#define SET_CONSTANT(object, constant)                           \
  do {                                                           \
    iotjs_jval_set_property_number(object, #constant, constant); \
  } while (0)


iotjs_jval_t InitDns() {
  iotjs_jval_t dns = iotjs_jval_create_object();

  iotjs_jval_set_method(&dns, "getaddrinfo", GetAddrInfo);
  SET_CONSTANT(&dns, AI_ADDRCONFIG);
  SET_CONSTANT(&dns, AI_V4MAPPED);

  return dns;
}
