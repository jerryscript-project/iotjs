/* Copyright 2015 Samsung Electronics Co., Ltd.
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
#include "iotjs_module_tcp.h"

#include "iotjs_reqwrap.h"
#include "uv.h"

namespace iotjs {


typedef ReqWrap<uv_getaddrinfo_t> GetAddrInfoReqWrap;


static void AfterGetAddrInfo(uv_getaddrinfo_t* req, int status, addrinfo* res) {
  GetAddrInfoReqWrap* req_wrap = reinterpret_cast<GetAddrInfoReqWrap*>(
      req->data);

  JArgList args(3);
  args.Add(JVal::Number(status));

  if (status == 0) {
    char ip[INET6_ADDRSTRLEN];
    const char *addr;

    // Only first address is used
    if (res->ai_family == AF_INET) {
      struct sockaddr_in* sockaddr = reinterpret_cast<struct sockaddr_in*>(
          res->ai_addr);
      addr = reinterpret_cast<char*>(&(sockaddr->sin_addr));
    } else {
      struct sockaddr_in6* sockaddr = reinterpret_cast<struct sockaddr_in6*>(
          res->ai_addr);
      addr = reinterpret_cast<char*>(&(sockaddr->sin6_addr));
    }

    int err = uv_inet_ntop(res->ai_family, addr, ip, INET6_ADDRSTRLEN);
    if (err) {
      ip[0] = 0;
    }

    JObject result(ip);
    args.Add(result);
  }

  uv_freeaddrinfo(res);

  // Make the callback into JavaScript
  MakeCallback(req_wrap->jcallback(), JObject::Null(), args);

  delete req_wrap;
}


JHANDLER_FUNCTION(GetAddrInfo) {
  JHANDLER_CHECK(handler.GetThis()->IsObject());
  JHANDLER_CHECK(handler.GetArgLength() == 4);
  JHANDLER_CHECK(handler.GetArg(0)->IsString());
  JHANDLER_CHECK(handler.GetArg(1)->IsNumber());
  JHANDLER_CHECK(handler.GetArg(2)->IsNumber());
  JHANDLER_CHECK(handler.GetArg(3)->IsFunction());

  String hostname = handler.GetArg(0)->GetString();
  int option = handler.GetArg(1)->GetInt32();
  int flags = handler.GetArg(2)->GetInt32();
  JObject* jcallback = handler.GetArg(3);

  int family;
  if (option == 0) {
    family = AF_UNSPEC;
  } else if (option == 4) {
    family = AF_INET;
  } else if (option == 6) {
    family = AF_INET6;
  } else {
    JHANDLER_THROW_RETURN(TypeError, "bad address family");
  }

  GetAddrInfoReqWrap* req_wrap = new GetAddrInfoReqWrap(*jcallback);

  struct addrinfo hints = {0};
  hints.ai_family = family;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = flags;

  int err = uv_getaddrinfo(Environment::GetEnv()->loop(),
                           req_wrap->req(),
                           AfterGetAddrInfo,
                           hostname.data(),
                           NULL,
                           &hints);

  if (err) {
    delete req_wrap;
  }

  handler.Return(JVal::Number(err));

  return true;
}


JObject* InitDns() {
  Module* module = GetBuiltinModule(MODULE_DNS);
  JObject* dns = module->module;

  if (dns == NULL) {
    dns = new JObject();
    dns->SetMethod("getaddrinfo", GetAddrInfo);
    dns->SetProperty("AI_ADDRCONFIG", JVal::Number(AI_ADDRCONFIG));
    dns->SetProperty("AI_V4MAPPED", JVal::Number(AI_V4MAPPED));

    module->module = dns;
  }

  return dns;
}


} // namespace iotjs
