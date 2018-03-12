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


iotjs_getaddrinfo_reqwrap_t* iotjs_getaddrinfo_reqwrap_create(
    const jerry_value_t jcallback) {
  iotjs_getaddrinfo_reqwrap_t* getaddrinfo_reqwrap =
      IOTJS_ALLOC(iotjs_getaddrinfo_reqwrap_t);
  iotjs_reqwrap_initialize(&getaddrinfo_reqwrap->reqwrap, jcallback,
                           (uv_req_t*)&getaddrinfo_reqwrap->req);
  return getaddrinfo_reqwrap;
}


static void iotjs_getaddrinfo_reqwrap_destroy(
    iotjs_getaddrinfo_reqwrap_t* getaddrinfo_reqwrap) {
  iotjs_reqwrap_destroy(&getaddrinfo_reqwrap->reqwrap);
  IOTJS_RELEASE(getaddrinfo_reqwrap);
}


void iotjs_getaddrinfo_reqwrap_dispatched(
    iotjs_getaddrinfo_reqwrap_t* getaddrinfo_reqwrap) {
  iotjs_getaddrinfo_reqwrap_destroy(getaddrinfo_reqwrap);
}


jerry_value_t iotjs_getaddrinfo_reqwrap_jcallback(
    iotjs_getaddrinfo_reqwrap_t* getaddrinfo_reqwrap) {
  return iotjs_reqwrap_jcallback(&getaddrinfo_reqwrap->reqwrap);
}


#if !defined(__NUTTX__) && !defined(__TIZENRT__)
char* getaddrinfo_error_str(int status) {
  switch (status) {
    case UV__EAI_ADDRFAMILY:
      return "EAI_ADDRFAMILY, address family for hostname not supported";
      break;
    case UV__EAI_AGAIN:
      return "EAI_AGAIN, temporary failure in name resolution";
      break;
    case UV__EAI_BADFLAGS:
      return "EAI_BADFLAGS, bad flags";
      break;
    case UV__EAI_FAIL:
      return "EAI_FAIL, Non-recoverable failure in name resolution";
      break;
    case UV__EAI_FAMILY:
      return "EAI_FAMILY, family not supported";
      break;
    case UV__EAI_CANCELED:
      return "EAI_CANCELED, request canceled";
      break;
    case UV__EAI_MEMORY:
      return "EAI_MEMORY, memory allocation failure";
      break;
    case UV__EAI_NODATA:
      return "EAI_NODATA, no address association with hostname";
      break;
    case UV__EAI_NONAME:
      return "EAI_NONAME, name or service not known";
      break;
    case UV__EAI_OVERFLOW:
      return "EAI_OVERFLOW, argument buffer overflow";
      break;
    case UV__EAI_SERVICE:
      return "EAI_SERVICE, service not supported";
      break;
    case UV__EAI_SOCKTYPE:
      return "EAI_SOCKTYPE, socktype not supported";
      break;
    case UV__EAI_PROTOCOL:
      return "EAI_PROTOCOL, unknown error";
      break;
    default:
      return "unknown error";
      break;
  }
}

static void AfterGetAddrInfo(uv_getaddrinfo_t* req, int status,
                             struct addrinfo* res) {
  iotjs_getaddrinfo_reqwrap_t* req_wrap =
      (iotjs_getaddrinfo_reqwrap_t*)(req->data);

  iotjs_jargs_t args = iotjs_jargs_create(3);

  if (status == 0) {
    char ip[INET6_ADDRSTRLEN];
    int family;
    const char* addr;

    // Only first address is used
    if (res->ai_family == AF_INET) {
      struct sockaddr_in* sockaddr = (struct sockaddr_in*)(res->ai_addr);
      addr = (char*)(&(sockaddr->sin_addr));
      family = 4;
    } else {
      struct sockaddr_in6* sockaddr = (struct sockaddr_in6*)(res->ai_addr);
      addr = (char*)(&(sockaddr->sin6_addr));
      family = 6;
    }

    int err = uv_inet_ntop(res->ai_family, addr, ip, INET6_ADDRSTRLEN);
    if (err) {
      ip[0] = 0;
      iotjs_jargs_append_error(&args,
                               "EAFNOSUPPORT, DNS could not resolve hostname");
    } else {
      iotjs_jargs_append_null(&args);
    }

    iotjs_jargs_append_string_raw(&args, ip);
    iotjs_jargs_append_number(&args, family);
  } else {
    iotjs_jargs_append_error(&args, getaddrinfo_error_str(status));
  }

  uv_freeaddrinfo(res);

  // Make the callback into JavaScript
  jerry_value_t jcallback = iotjs_getaddrinfo_reqwrap_jcallback(req_wrap);
  iotjs_make_callback(jcallback, jerry_create_undefined(), &args);

  iotjs_jargs_destroy(&args);

  iotjs_getaddrinfo_reqwrap_dispatched(req_wrap);
}
#endif


JS_FUNCTION(GetAddrInfo) {
  DJS_CHECK_THIS();
  DJS_CHECK_ARGS(4, string, number, number, function);

  iotjs_string_t hostname = JS_GET_ARG(0, string);
  int option = JS_GET_ARG(1, number);
  int flags = JS_GET_ARG(2, number);
  int error = 0;
  const jerry_value_t jcallback = JS_GET_ARG(3, function);

  int family;
  if (option == 0) {
#if defined(__NUTTX__) || defined(__TIZENRT__)
    family = AF_INET;
#else
    family = AF_UNSPEC;
#endif
  } else if (option == 4) {
    family = AF_INET;
  } else if (option == 6) {
    family = AF_INET6;
  } else {
    iotjs_string_destroy(&hostname);
    return JS_CREATE_ERROR(TYPE, "bad address family");
  }

#if defined(__NUTTX__) || defined(__TIZENRT__)
  iotjs_jargs_t args = iotjs_jargs_create(3);
  char ip[INET6_ADDRSTRLEN] = "";
  const char* hostname_data = iotjs_string_data(&hostname);

  if (strcmp(hostname_data, "localhost") == 0) {
    strncpy(ip, "127.0.0.1", strlen("127.0.0.1") + 1);
  } else {
    struct sockaddr_in addr;

    if (inet_pton(family, hostname_data, &(addr.sin_addr)) == 1) {
      inet_ntop(family, &(addr.sin_addr), ip, INET6_ADDRSTRLEN);
    } else {
      error = EAFNOSUPPORT;
    }
  }

  if (error) {
    iotjs_jargs_append_error(&args, "EAFNOSUPPORT, could not resolve hostname");
  } else {
    iotjs_jargs_append_null(&args);
  }

  iotjs_jargs_append_string_raw(&args, ip);
  iotjs_jargs_append_number(&args, option);

  iotjs_make_callback(jcallback, jerry_create_undefined(), &args);
  iotjs_jargs_destroy(&args);
  IOTJS_UNUSED(flags);
#else
  iotjs_getaddrinfo_reqwrap_t* req_wrap =
      iotjs_getaddrinfo_reqwrap_create(jcallback);

  static const struct addrinfo empty_hints;
  struct addrinfo hints = empty_hints;
  hints.ai_family = family;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = flags;

  error = uv_getaddrinfo(iotjs_environment_loop(iotjs_environment_get()),
                         &req_wrap->req, AfterGetAddrInfo,
                         iotjs_string_data(&hostname), NULL, &hints);

  if (error) {
    iotjs_getaddrinfo_reqwrap_dispatched(req_wrap);
  }
#endif


  iotjs_string_destroy(&hostname);

  return jerry_create_number(error);
}


#define SET_CONSTANT(object, constant)                           \
  do {                                                           \
    iotjs_jval_set_property_number(object, #constant, constant); \
  } while (0)


jerry_value_t InitDns() {
  jerry_value_t dns = jerry_create_object();

  iotjs_jval_set_method(dns, IOTJS_MAGIC_STRING_GETADDRINFO, GetAddrInfo);
  SET_CONSTANT(dns, AI_ADDRCONFIG);
  SET_CONSTANT(dns, AI_V4MAPPED);

  return dns;
}
