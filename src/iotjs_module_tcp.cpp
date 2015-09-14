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

#include "iotjs_module_buffer.h"
#include "iotjs_handlewrap.h"
#include "iotjs_reqwrap.h"


namespace iotjs {


class TcpWrap : public HandleWrap {
 public:
  explicit TcpWrap(Environment* env,
                   JObject& jtcp)
      : HandleWrap(jtcp, reinterpret_cast<uv_handle_t*>(&_handle)) {
    uv_tcp_init(env->loop(), &_handle);
  }

  static TcpWrap* FromJObject(JObject* jtcp) {
    TcpWrap* wrap = reinterpret_cast<TcpWrap*>(jtcp->GetNative());
    IOTJS_ASSERT(wrap != NULL);
    return wrap;
  }

  uv_tcp_t* tcp_handle() {
    return &_handle;
  }

 protected:
  uv_tcp_t _handle;
};


typedef ReqWrap<uv_connect_t> ConnectReqWrap;
typedef ReqWrap<uv_write_t> WriteReqWrap;
typedef ReqWrap<uv_shutdown_t> ShutdownReqWrap;


JHANDLER_FUNCTION(TCP) {
  JHANDLER_CHECK(handler.GetThis()->IsObject());
  JHANDLER_CHECK(handler.GetArgLength() == 0);

  Environment* env = Environment::GetEnv();
  JObject* jtcp = handler.GetThis();

  TcpWrap* tcp_wrap = new TcpWrap(env, *jtcp);
  IOTJS_ASSERT(tcp_wrap->jobject().IsObject());
  IOTJS_ASSERT(jtcp->GetNative() != 0);

  return true;
}


JHANDLER_FUNCTION(Open) {
  return true;
}


// Socket close result handler.
static void AfterClose(uv_handle_t* handle) {
  HandleWrap* tcp_wrap = HandleWrap::FromHandle(handle);
  IOTJS_ASSERT(tcp_wrap != NULL);

  // tcp object.
  JObject jtcp = tcp_wrap->jobject();
  IOTJS_ASSERT(jtcp.IsObject());

  // callback function.
  JObject jcallback = jtcp.GetProperty("onclose");
  if (jcallback.IsFunction()) {
    MakeCallback(jcallback, JObject::Null(), JArgList::Empty());
  }
}


// Close socket
JHANDLER_FUNCTION(Close) {
  JHANDLER_CHECK(handler.GetThis()->IsObject());

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
JHANDLER_FUNCTION(Bind) {
  JHANDLER_CHECK(handler.GetThis()->IsObject());
  JHANDLER_CHECK(handler.GetArgLength() == 2);
  JHANDLER_CHECK(handler.GetArg(0)->IsString());
  JHANDLER_CHECK(handler.GetArg(1)->IsNumber());

  String address = handler.GetArg(0)->GetString();
  int port = handler.GetArg(1)->GetInt32();

  sockaddr_in addr;
  int err = uv_ip4_addr(address.data(), port, &addr);

  if (err == 0) {
    TcpWrap* wrap = TcpWrap::FromJObject(handler.GetThis());
    err = uv_tcp_bind(wrap->tcp_handle(),
                      reinterpret_cast<const sockaddr*>(&addr),
                      0);
  }

  handler.Return(JVal::Number(err));

  return true;
}


// Connection request result handler.
static void AfterConnect(uv_connect_t* req, int status) {
  ConnectReqWrap* req_wrap = reinterpret_cast<ConnectReqWrap*>(req->data);
  TcpWrap* tcp_wrap = reinterpret_cast<TcpWrap*>(req->handle->data);
  IOTJS_ASSERT(req_wrap != NULL);
  IOTJS_ASSERT(tcp_wrap != NULL);

  // Take callback function object.
  //  function afterConnect(status)
  JObject jcallback = req_wrap->jcallback();
  IOTJS_ASSERT(jcallback.IsFunction());

  // Only parameter is status code.
  JArgList args(1);
  args.Add(JVal::Number(status));

  // Make callback.
  MakeCallback(jcallback, JObject::Null(), args);

  // Release request wrapper.
  delete req_wrap;
}


// Create a connection using the socket.
// [0] address
// [1] port
// [2] callback
JHANDLER_FUNCTION(Connect) {
  JHANDLER_CHECK(handler.GetThis()->IsObject());
  JHANDLER_CHECK(handler.GetArgLength() == 3);
  JHANDLER_CHECK(handler.GetArg(0)->IsString());
  JHANDLER_CHECK(handler.GetArg(1)->IsNumber());
  JHANDLER_CHECK(handler.GetArg(2)->IsFunction());

  String address = handler.GetArg(0)->GetString();
  int port = handler.GetArg(1)->GetInt32();
  JObject jcallback = *handler.GetArg(2);

  sockaddr_in addr;
  int err = uv_ip4_addr(address.data(), port, &addr);

  if (err == 0) {
    // Get tcp wrapper from javascript socket object.
    TcpWrap* tcp_wrap = TcpWrap::FromJObject(handler.GetThis());

    // Create connection request wrapper.
    ConnectReqWrap* req_wrap = new ConnectReqWrap(jcallback);

    // Create connection request.
    err = uv_tcp_connect(req_wrap->req(),
                         tcp_wrap->tcp_handle(),
                         reinterpret_cast<const sockaddr*>(&addr),
                         AfterConnect);

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
  IOTJS_ASSERT(tcp_wrap->tcp_handle() == reinterpret_cast<uv_tcp_t*>(handle));

  // Tcp object
  JObject jtcp = tcp_wrap->jobject();
  IOTJS_ASSERT(jtcp.IsObject());

  // `onconnection` callback.
  JObject jonconnection = jtcp.GetProperty("onconnection");
  IOTJS_ASSERT(jonconnection.IsFunction());

  // The callback takes two parameter
  // [0] status
  // [1] client tcp object
  JArgList args(2);
  args.Add(JVal::Number(status));

  if (status == 0) {
    // Create client socket handle wrapper.
    JObject jcreate_tcp = jtcp.GetProperty("createTCP");
    IOTJS_ASSERT(jcreate_tcp.IsFunction());

    JObject jclient_tcp = jcreate_tcp.CallOk(JObject::Null(),
                                             JArgList::Empty());
    IOTJS_ASSERT(jclient_tcp.IsObject());

    TcpWrap* tcp_wrap = reinterpret_cast<TcpWrap*>(jclient_tcp.GetNative());

    uv_stream_t* client_handle =
        reinterpret_cast<uv_stream_t*>(tcp_wrap->tcp_handle());

    int err = uv_accept(handle, client_handle);
    if (err) {
      return;
    }

    args.Add(jclient_tcp);
  }

  MakeCallback(jonconnection, jtcp, args);
}


JHANDLER_FUNCTION(Listen) {
  JHANDLER_CHECK(handler.GetThis()->IsObject());
  JHANDLER_CHECK(handler.GetArgLength() == 1);
  JHANDLER_CHECK(handler.GetArg(0)->IsNumber());

  TcpWrap* tcp_wrap = TcpWrap::FromJObject(handler.GetThis());

  int backlog = handler.GetArg(0)->GetInt32();

  int err = uv_listen(reinterpret_cast<uv_stream_t*>(tcp_wrap->tcp_handle()),
                      backlog,
                      OnConnection);

  handler.Return(JVal::Number(err));

  return true;
}


void AfterWrite(uv_write_t* req, int status) {
  WriteReqWrap* req_wrap = reinterpret_cast<WriteReqWrap*>(req->data);
  TcpWrap* tcp_wrap = reinterpret_cast<TcpWrap*>(req->handle->data);
  IOTJS_ASSERT(req_wrap != NULL);
  IOTJS_ASSERT(tcp_wrap != NULL);

  // Take callback function object.
  JObject jcallback = req_wrap->jcallback();

  // Only parameter is status code.
  JArgList args(1);
  args.Add(JVal::Number(status));

  // Make callback.
  MakeCallback(jcallback, JObject::Null(), args);

  // Release request wrapper.
  delete req_wrap;
}


JHANDLER_FUNCTION(Write) {
  JHANDLER_CHECK(handler.GetThis()->IsObject());
  JHANDLER_CHECK(handler.GetArgLength() == 2);
  JHANDLER_CHECK(handler.GetArg(0)->IsObject());
  JHANDLER_CHECK(handler.GetArg(1)->IsFunction());

  TcpWrap* tcp_wrap = TcpWrap::FromJObject(handler.GetThis());
  IOTJS_ASSERT(tcp_wrap != NULL);

  JObject* jbuffer = handler.GetArg(0);
  BufferWrap* buffer_wrap = BufferWrap::FromJBuffer(*jbuffer);
  char* buffer = buffer_wrap->buffer();
  int len = buffer_wrap->length();

  uv_buf_t buf;
  buf.base = buffer;
  buf.len = len;

  WriteReqWrap* req_wrap = new WriteReqWrap(*handler.GetArg(1));

  int err = uv_write(req_wrap->req(),
                     reinterpret_cast<uv_stream_t*>(tcp_wrap->tcp_handle()),
                     &buf,
                     1,
                     AfterWrite
                     );

  if (err) {
    delete req_wrap;
  }

  handler.Return(JVal::Number(err));

  return true;
}


void OnAlloc(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
  if (suggested_size > IOTJS_MAX_READ_BUFFER_SIZE) {
    suggested_size = IOTJS_MAX_READ_BUFFER_SIZE;
  }

  buf->base = AllocBuffer(suggested_size);
  buf->len = suggested_size;
}


void OnRead(uv_stream_t* handle, ssize_t nread, const uv_buf_t* buf) {
  TcpWrap* tcp_wrap = reinterpret_cast<TcpWrap*>(handle->data);
  IOTJS_ASSERT(tcp_wrap != NULL);

  // tcp handle
  JObject jtcp = tcp_wrap->jobject();
  IOTJS_ASSERT(jtcp.IsObject());

  // socket object
  JObject jsocket = jtcp.GetProperty("owner");
  IOTJS_ASSERT(jsocket.IsObject());

  // onread callback
  JObject jonread = jtcp.GetProperty("onread");
  IOTJS_ASSERT(jonread.IsFunction());

  JArgList jargs(4);
  jargs.Add(jsocket);
  jargs.Add(JVal::Number((int)nread));
  jargs.Add(JVal::Bool(false));

  if (nread <= 0) {
    if (buf->base != NULL) {
      ReleaseBuffer(buf->base);
    }
    if (nread < 0) {
      if (nread == UV__EOF) {
        jargs.Set(2, JVal::Bool(true));
      }
      MakeCallback(jonread, JObject::Null(), jargs);
    }
    return;
  }

  JObject jbuffer(CreateBuffer(static_cast<size_t>(nread)));
  BufferWrap* buffer_wrap = BufferWrap::FromJBuffer(jbuffer);

  buffer_wrap->Copy(buf->base, nread);

  jargs.Add(jbuffer);
  MakeCallback(jonread, JObject::Null(), jargs);

  ReleaseBuffer(buf->base);
}


JHANDLER_FUNCTION(ReadStart) {
  JHANDLER_CHECK(handler.GetThis()->IsObject());

  TcpWrap* tcp_wrap = TcpWrap::FromJObject(handler.GetThis());
  IOTJS_ASSERT(tcp_wrap != NULL);

  int err = uv_read_start(
      reinterpret_cast<uv_stream_t*>(tcp_wrap->tcp_handle()),
      OnAlloc,
      OnRead);

  handler.Return(JVal::Number(err));

  return true;
}


static void AfterShutdown(uv_shutdown_t* req, int status) {
  ShutdownReqWrap* req_wrap = reinterpret_cast<ShutdownReqWrap*>(req->data);
  TcpWrap* tcp_wrap = reinterpret_cast<TcpWrap*>(req->handle->data);
  IOTJS_ASSERT(req_wrap != NULL);
  IOTJS_ASSERT(tcp_wrap != NULL);

  // function onShutdown(status)
  JObject jonshutdown(req_wrap->jcallback());
  IOTJS_ASSERT(jonshutdown.IsFunction());

  JArgList args(1);
  args.Add(JVal::Number(status));

  MakeCallback(jonshutdown, JObject::Null(), args);

  delete req_wrap;
}


JHANDLER_FUNCTION(Shutdown) {
  JHANDLER_CHECK(handler.GetThis()->IsObject());
  JHANDLER_CHECK(handler.GetArgLength() == 1);
  JHANDLER_CHECK(handler.GetArg(0)->IsFunction());

  TcpWrap* tcp_wrap = TcpWrap::FromJObject(handler.GetThis());
  IOTJS_ASSERT(tcp_wrap != NULL);

  ShutdownReqWrap* req_wrap = new ShutdownReqWrap(*handler.GetArg(0));

  int err = uv_shutdown(req_wrap->req(),
                        reinterpret_cast<uv_stream_t*>(tcp_wrap->tcp_handle()),
                        AfterShutdown);

  if (err) {
    delete req_wrap;
  }

  handler.Return(JVal::Number(err));

  return true;
}


// Enable/Disable keepalive option.
// [0] enable
// [1] delay
JHANDLER_FUNCTION(SetKeepAlive) {
  JHANDLER_CHECK(handler.GetThis()->IsObject());
  JHANDLER_CHECK(handler.GetArgLength() == 2);
  JHANDLER_CHECK(handler.GetArg(0)->IsNumber());
  JHANDLER_CHECK(handler.GetArg(1)->IsNumber());

  int enable = handler.GetArg(0)->GetInt32();
  unsigned int delay = (unsigned int) handler.GetArg(1)->GetInt32();

  TcpWrap* wrap = TcpWrap::FromJObject(handler.GetThis());
  int err = uv_tcp_keepalive(wrap->tcp_handle(), enable, delay);

  handler.Return(JVal::Number(err));

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
    prototype.SetMethod("close", Close);
    prototype.SetMethod("connect", Connect);
    prototype.SetMethod("bind", Bind);
    prototype.SetMethod("listen", Listen);
    prototype.SetMethod("write", Write);
    prototype.SetMethod("readStart", ReadStart);
    prototype.SetMethod("shutdown", Shutdown);
    prototype.SetMethod("setKeepAlive", SetKeepAlive);

    module->module = tcp;
  }

  return tcp;
}


} // namespace iotjs
