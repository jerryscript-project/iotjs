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

#include "iotjs_uv_request.h"

#if !defined(__NUTTX__)
char* getaddrinfo_error_str(int status) {
  switch (status) {
    case UV__EAI_ADDRFAMILY:
      return "EAI_ADDRFAMILY, address family for hostname not supported";
    case UV__EAI_AGAIN:
      return "EAI_AGAIN, temporary failure in name resolution";
    case UV__EAI_BADFLAGS:
      return "EAI_BADFLAGS, bad flags";
    case UV__EAI_FAIL:
      return "EAI_FAIL, Non-recoverable failure in name resolution";
    case UV__EAI_FAMILY:
      return "EAI_FAMILY, family not supported";
    case UV__EAI_CANCELED:
      return "EAI_CANCELED, request canceled";
    case UV__EAI_MEMORY:
      return "EAI_MEMORY, memory allocation failure";
    case UV__EAI_NODATA:
      return "EAI_NODATA, no address association with hostname";
    case UV__EAI_NONAME:
      return "EAI_NONAME, name or service not known";
    case UV__EAI_OVERFLOW:
      return "EAI_OVERFLOW, argument buffer overflow";
    case UV__EAI_SERVICE:
      return "EAI_SERVICE, service not supported";
    case UV__EAI_SOCKTYPE:
      return "EAI_SOCKTYPE, socktype not supported";
    case UV__EAI_PROTOCOL:
      return "EAI_PROTOCOL, unknown error";
    default:
      return "unknown error";
  }
}

static void after_get_addr_info(uv_getaddrinfo_t* req, int status,
                                struct addrinfo* res) {
  size_t argc = 0;
  jerry_value_t args[3] = { 0 };

  if (status == 0 && res != NULL) {
    char ip[INET6_ADDRSTRLEN];
    int family;
    const char* addr;
    struct addrinfo* info;

    /* search for the first AF_INET entry */
    for (info = res; info != NULL; info = info->ai_next) {
      if (info->ai_family == AF_INET) {
        break;
      }
    }

    if (info == NULL) {
      /* Did not find an AF_INET addr, using the first one */
      info = res;
    }

    IOTJS_ASSERT(info != NULL);

    if (info->ai_family == AF_INET) {
      struct sockaddr_in* sockaddr = (struct sockaddr_in*)(info->ai_addr);
      addr = (char*)(&(sockaddr->sin_addr));
      family = 4;
    } else {
      struct sockaddr_in6* sockaddr = (struct sockaddr_in6*)(info->ai_addr);
      addr = (char*)(&(sockaddr->sin6_addr));
      family = 6;
    }

    int err = uv_inet_ntop(info->ai_family, addr, ip, INET6_ADDRSTRLEN);
    if (err) {
      ip[0] = 0;
      args[argc++] = iotjs_jval_create_error_without_error_flag(
          "EAFNOSUPPORT, DNS could not resolve hostname");
    } else {
      args[argc++] = jerry_create_null();
    }

    args[argc++] = jerry_create_string_from_utf8((const jerry_char_t*)ip);
    args[argc++] = jerry_create_number(family);
  } else {
    args[argc++] = iotjs_jval_create_error_without_error_flag(
        getaddrinfo_error_str(status));
  }

  uv_freeaddrinfo(res);

  // Make the callback into JavaScript
  jerry_value_t jcallback = *IOTJS_UV_REQUEST_JSCALLBACK(req);
  iotjs_invoke_callback(jcallback, jerry_create_undefined(), args, argc);

  for (size_t i = 0; i < argc; i++) {
    jerry_release_value(args[i]);
  }

  iotjs_uv_request_destroy((uv_req_t*)req);
}
#endif


JS_FUNCTION(get_address_info) {
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

#if defined(__NUTTX__)
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

  size_t argc = 0;
  jerry_value_t args[3] = { 0 };

  if (error) {
    args[argc++] = iotjs_jval_create_error_without_error_flag(
        "EAFNOSUPPORT, could not resolve hostname");
  } else {
    args[argc++] = jerry_create_null();
  }

  args[argc++] = jerry_create_string_from_utf8((const jerry_char_t*)ip);
  args[argc++] = jerry_create_number(option);

  iotjs_invoke_callback(jcallback, jerry_create_undefined(), args, argc);
  for (size_t i = 0; i < argc; i++) {
    jerry_release_value(args[i]);
  }
  IOTJS_UNUSED(flags);
#else
  uv_req_t* req_addr =
      iotjs_uv_request_create(sizeof(uv_getaddrinfo_t), jcallback, 0);

  static const struct addrinfo empty_hints;
  struct addrinfo hints = empty_hints;
  hints.ai_family = family;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = flags;

  error = uv_getaddrinfo(iotjs_environment_loop(iotjs_environment_get()),
                         (uv_getaddrinfo_t*)req_addr, after_get_addr_info,
                         iotjs_string_data(&hostname), NULL, &hints);

  if (error) {
    iotjs_uv_request_destroy(req_addr);
  }
#endif


  iotjs_string_destroy(&hostname);

  return jerry_create_number(error);
}


#define SET_CONSTANT(object, constant)                           \
  do {                                                           \
    iotjs_jval_set_property_number(object, #constant, constant); \
  } while (0)


jerry_value_t iotjs_init_dns(void) {
  jerry_value_t dns = jerry_create_object();

  iotjs_jval_set_method(dns, IOTJS_MAGIC_STRING_GETADDRINFO, get_address_info);
  SET_CONSTANT(dns, AI_ADDRCONFIG);
  SET_CONSTANT(dns, AI_V4MAPPED);

  return dns;
}
