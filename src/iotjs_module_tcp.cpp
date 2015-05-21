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


#include "iotjs_module_tcp.h"

#include "iotjs_env.h"
#include "iotjs_module.h"
#include "iotjs_module_process.h"
#include "iotjs_handlewrap.h"
#include "iotjs_reqwrap.h"

#include <uv.h>


namespace iotjs {


class TcpWrap : public HandleWrap {
 public:
  explicit TcpWrap(Environment* env, JObject& jtcp)
      : HandleWrap(jtcp, reinterpret_cast<uv_handle_t*>(&_handle)) {
    uv_tcp_init(env->loop(), &_handle);
  }

  static TcpWrap* FromJObject(JObject* jobj) {
    TcpWrap* wrap = reinterpret_cast<TcpWrap*>(jobj->GetNative());
    assert(wrap != NULL);
    return wrap;
  }

  uv_tcp_t* tcp_handle() {
    return &_handle;
  }

 protected:
  uv_tcp_t _handle;
};


class ConnectReqWrap : public ReqWrap {
 public:
  explicit ConnectReqWrap(JObject& jcallback)
      : ReqWrap(jcallback, reinterpret_cast<uv_req_t*>(&_data)) {
  }

  uv_connect_t* connect_req() {
    return &_data;
  }

 protected:
  uv_connect_t _data;
};


JHANDLER_FUNCTION(TCP, handler) {
  assert(handler.GetThis()->IsObject());

  Environment* env = Environment::GetEnv();
  JObject* jtcp = handler.GetThis();

  TcpWrap* tcp_wrap = new TcpWrap(env, *jtcp);
  assert(tcp_wrap->jobject()->IsObject());
  assert(jtcp->GetNative() != 0);

  return true;
}


JHANDLER_FUNCTION(Open, handler) {
  return true;
}


// Socket close result handler.
static void AfterClose(uv_handle_t* handle) {
  HandleWrap* wrap = HandleWrap::FromHandle(handle);
  assert(wrap != NULL);

  JObject* jtcp = wrap->jobject();
  assert(jtcp != NULL);
  assert(jtcp->IsObject());

  JObject jsocket = jtcp->GetProperty("_socket");
  assert(jsocket.IsObject());

  JObject jonclose = jsocket.GetProperty("_onclose");
  assert(jonclose.IsFunction());

  MakeCallback(jonclose, jsocket, JArgList::Empty());
}


// Close socket
JHANDLER_FUNCTION(Close, handler) {
  assert(handler.GetThis()->IsObject());

  JObject* jtcp = handler.GetThis();
  HandleWrap* wrap = reinterpret_cast<HandleWrap*>(jtcp->GetNative());

  // close uv handle, `AfterClose` will be called after socket closed.
  wrap->Close(AfterClose);

  return true;
}


// Socket binding, this function would be called from server socket before
// start listening.
// [0] address
// [1] port
JHANDLER_FUNCTION(Bind, handler) {
  assert(handler.GetThis()->IsObject());
  assert(handler.GetArgLength() == 2);
  assert(handler.GetArg(0)->IsString());
  assert(handler.GetArg(1)->IsNumber());

  LocalString address(handler.GetArg(0)->GetCString());
  int port = handler.GetArg(1)->GetInt32();

  sockaddr_in addr;
  int err = uv_ip4_addr(address, port, &addr);

  if (err == 0) {
    TcpWrap* wrap = TcpWrap::FromJObject(handler.GetThis());
    err = uv_tcp_bind(wrap->tcp_handle(),
                      reinterpret_cast<const sockaddr*>(&addr),
                      0);
  }

  JObject ret(err);
  handler.Return(ret);

  return true;
}


// Connection request result handler.
static void AfterConnect(uv_connect_t* req, int status) {
  ConnectReqWrap* req_wrap = reinterpret_cast<ConnectReqWrap*>(req->data);
  TcpWrap* tcp_wrap = reinterpret_cast<TcpWrap*>(req->handle->data);
  assert(req_wrap != NULL);
  assert(tcp_wrap != NULL);

  // Take callback function object.
  JObject* jcallback = req_wrap->jcallback();

  // Only parameter is status code.
  JArgList args(1);
  args.Add(JVal::Int(status));

  // Make callback.
  MakeCallback(*jcallback, *tcp_wrap->jobject(), args);

  // Release request wrapper.
  delete req_wrap;
}


// Create a connection using the socket.
// [0] address
// [1] port
// [2] callback
JHANDLER_FUNCTION(Connect, handler) {
  assert(handler.GetThis()->IsObject());
  assert(handler.GetArgLength() == 3);
  assert(handler.GetArg(0)->IsString());
  assert(handler.GetArg(1)->IsNumber());
  assert(handler.GetArg(2)->IsFunction());

  LocalString address(handler.GetArg(0)->GetCString());
  int port = handler.GetArg(1)->GetInt32();
  JObject jcallback = *handler.GetArg(2);

  sockaddr_in addr;
  int err = uv_ip4_addr(address, port, &addr);

  if (err == 0) {
    // Get tcp wrapper from javascript socket object.
    TcpWrap* tcp_wrap = TcpWrap::FromJObject(handler.GetThis());

    // Create connection request wrapper.
    ConnectReqWrap* req_wrap = new ConnectReqWrap(jcallback);

    // Create connection request.
    err = uv_tcp_connect(req_wrap->connect_req(),
                         tcp_wrap->tcp_handle(),
                         reinterpret_cast<const sockaddr*>(&addr),
                         AfterConnect);

    req_wrap->Dispatched();

    if (err) {
      delete req_wrap;
    }
  }

  JObject ret(err);
  handler.Return(ret);

  return true;
}


// A client socket wants to connect to this server.
// Parameters:
//   * uv_stream_t* handle - server handle
//   * int status - status code
static void OnConnection(uv_stream_t* handle, int status) {
  // Server tcp wrapper.
  TcpWrap* tcp_wrap = reinterpret_cast<TcpWrap*>(handle->data);
  assert(tcp_wrap->tcp_handle() == reinterpret_cast<uv_tcp_t*>(handle));

  // Server tcp object.
  JObject* jtcp = tcp_wrap->jobject();
  assert(jtcp != NULL);
  assert(jtcp->IsObject());
  assert(reinterpret_cast<HandleWrap*>(tcp_wrap) ==
         reinterpret_cast<HandleWrap*>(jtcp->GetNative()));

  // Server object.
  JObject jserver = jtcp->GetProperty("_socket");
  assert(jserver.IsObject());

  // `onconnection` callback.
  JObject jonconnection = jserver.GetProperty("_onconnection");
  assert(jonconnection.IsFunction());

  // The callback takes two parameter
  // [0] status
  // [1] client tcp object
  JArgList args(2);
  args.Add(JVal::Int(status));

  if (status == 0) {
    // Create client socket handle wrapper.
    JObject jfunc_create_tcp = jserver.GetProperty("_createTCP");
    assert(jfunc_create_tcp.IsFunction());

    JObject jclient_tcp = jfunc_create_tcp.Call(jserver, JArgList::Empty());
    assert(jclient_tcp.IsObject());

    TcpWrap* client_wrap = new TcpWrap(Environment::GetEnv(), jclient_tcp);
    uv_stream_t* client_handle =
        reinterpret_cast<uv_stream_t*>(client_wrap->tcp_handle());

    int err = uv_accept(handle, client_handle);
    if (err) {
      return;
    }

    args.Add(jclient_tcp);
  }

  MakeCallback(jonconnection, jserver, args);
}


JHANDLER_FUNCTION(Listen, handler) {
  assert(handler.GetThis()->IsObject());

  TcpWrap* tcp_wrap = TcpWrap::FromJObject(handler.GetThis());

  int backlog = handler.GetArg(0)->GetInt32();

  int err = uv_listen(reinterpret_cast<uv_stream_t*>(tcp_wrap->tcp_handle()),
                      backlog,
                      OnConnection);

  handler.Return(JVal::Int(err));

  return true;
}


JObject* InitTcp() {
  Module* module = GetBuiltinModule(MODULE_TCP);
  JObject* tcp = module->module;

  if (tcp == NULL) {
    tcp = new JObject(TCP);

    JObject prototype;
    tcp->SetProperty("prototype", prototype);

    prototype.SetMethod("open", Open);
    prototype.SetMethod("bind", Bind);
    prototype.SetMethod("close", Close);
    prototype.SetMethod("connect", Connect);
    prototype.SetMethod("listen", Listen);


    module->module = tcp;
  }

  return tcp;
}


} // namespace iotjs
