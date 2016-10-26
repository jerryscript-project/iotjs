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
  JHANDLER_CHECK(iotjs_jhandler_get_this(jhandler)->IsObject());
  JHANDLER_CHECK(iotjs_jhandler_get_arg_length(jhandler) == 0);

  Environment* env = Environment::GetEnv();
  JObject* jtcp = iotjs_jhandler_get_this(jhandler);

  TcpWrap* tcp_wrap = new TcpWrap(env, *jtcp);
  IOTJS_ASSERT(tcp_wrap->jobject().IsObject());
  IOTJS_ASSERT(jtcp->GetNative() != 0);
}


JHANDLER_FUNCTION(Open) {
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
    MakeCallback(jcallback, JObject::Undefined(), iotjs_jargs_empty);
  }
}


// Close socket
JHANDLER_FUNCTION(Close) {
  JHANDLER_CHECK(iotjs_jhandler_get_this(jhandler)->IsObject());

  JObject* jtcp = iotjs_jhandler_get_this(jhandler);
  HandleWrap* wrap = reinterpret_cast<HandleWrap*>(jtcp->GetNative());

  // close uv handle, `AfterClose` will be called after socket closed.
  wrap->Close(AfterClose);
}


// Socket binding, this function would be called from server socket before
// start listening.
// [0] address
// [1] port
JHANDLER_FUNCTION(Bind) {
  JHANDLER_CHECK(iotjs_jhandler_get_this(jhandler)->IsObject());
  JHANDLER_CHECK(iotjs_jhandler_get_arg_length(jhandler) == 2);
  JHANDLER_CHECK(iotjs_jhandler_get_arg(jhandler, 0)->IsString());
  JHANDLER_CHECK(iotjs_jhandler_get_arg(jhandler, 1)->IsNumber());

  iotjs_string_t address = iotjs_jhandler_get_arg(jhandler, 0)->GetString();
  int port = iotjs_jhandler_get_arg(jhandler, 1)->GetInt32();

  sockaddr_in addr;
  int err = uv_ip4_addr(iotjs_string_data(&address), port, &addr);

  if (err == 0) {
    TcpWrap* wrap = TcpWrap::FromJObject(iotjs_jhandler_get_this(jhandler));
    err = uv_tcp_bind(wrap->tcp_handle(),
                      reinterpret_cast<const sockaddr*>(&addr),
                      0);
  }

  iotjs_jhandler_return_number(jhandler, err);

  iotjs_string_destroy(&address);
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
  iotjs_jargs_t args = iotjs_jargs_create(1);
  iotjs_jargs_append_number(&args, status);

  // Make callback.
  MakeCallback(jcallback, JObject::Undefined(), args);

  // Destroy args
  iotjs_jargs_destroy(&args);

  // Release request wrapper.
  delete req_wrap;
}


// Create a connection using the socket.
// [0] address
// [1] port
// [2] callback
JHANDLER_FUNCTION(Connect) {
  JHANDLER_CHECK(iotjs_jhandler_get_this(jhandler)->IsObject());
  JHANDLER_CHECK(iotjs_jhandler_get_arg_length(jhandler) == 3);
  JHANDLER_CHECK(iotjs_jhandler_get_arg(jhandler, 0)->IsString());
  JHANDLER_CHECK(iotjs_jhandler_get_arg(jhandler, 1)->IsNumber());
  JHANDLER_CHECK(iotjs_jhandler_get_arg(jhandler, 2)->IsFunction());

  iotjs_string_t address = iotjs_jhandler_get_arg(jhandler, 0)->GetString();
  int port = iotjs_jhandler_get_arg(jhandler, 1)->GetInt32();
  JObject jcallback = *iotjs_jhandler_get_arg(jhandler, 2);

  sockaddr_in addr;
  int err = uv_ip4_addr(iotjs_string_data(&address), port, &addr);

  if (err == 0) {
    // Get tcp wrapper from javascript socket object.
    TcpWrap* tcp_wrap = TcpWrap::FromJObject(iotjs_jhandler_get_this(jhandler));

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

  iotjs_jhandler_return_number(jhandler, err);

  iotjs_string_destroy(&address);
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
  iotjs_jargs_t args = iotjs_jargs_create(2);
  iotjs_jargs_append_number(&args, status);

  if (status == 0) {
    // Create client socket handle wrapper.
    JObject jcreate_tcp = jtcp.GetProperty("createTCP");
    IOTJS_ASSERT(jcreate_tcp.IsFunction());

    JObject jclient_tcp = jcreate_tcp.CallOk(JObject::Undefined(),
                                             iotjs_jargs_empty);
    IOTJS_ASSERT(jclient_tcp.IsObject());

    TcpWrap* tcp_wrap = reinterpret_cast<TcpWrap*>(jclient_tcp.GetNative());

    uv_stream_t* client_handle =
        reinterpret_cast<uv_stream_t*>(tcp_wrap->tcp_handle());

    int err = uv_accept(handle, client_handle);
    if (err) {
      return;
    }

    iotjs_jargs_append_obj(&args, &jclient_tcp);
  }

  MakeCallback(jonconnection, jtcp, args);

  iotjs_jargs_destroy(&args);
}


JHANDLER_FUNCTION(Listen) {
  JHANDLER_CHECK(iotjs_jhandler_get_this(jhandler)->IsObject());
  JHANDLER_CHECK(iotjs_jhandler_get_arg_length(jhandler) == 1);
  JHANDLER_CHECK(iotjs_jhandler_get_arg(jhandler, 0)->IsNumber());

  TcpWrap* tcp_wrap = TcpWrap::FromJObject(iotjs_jhandler_get_this(jhandler));

  int backlog = iotjs_jhandler_get_arg(jhandler, 0)->GetInt32();

  int err = uv_listen(reinterpret_cast<uv_stream_t*>(tcp_wrap->tcp_handle()),
                      backlog,
                      OnConnection);

  iotjs_jhandler_return_number(jhandler, err);
}


void AfterWrite(uv_write_t* req, int status) {
  WriteReqWrap* req_wrap = reinterpret_cast<WriteReqWrap*>(req->data);
  TcpWrap* tcp_wrap = reinterpret_cast<TcpWrap*>(req->handle->data);
  IOTJS_ASSERT(req_wrap != NULL);
  IOTJS_ASSERT(tcp_wrap != NULL);

  // Take callback function object.
  JObject jcallback = req_wrap->jcallback();

  // Only parameter is status code.
  iotjs_jargs_t args = iotjs_jargs_create(1);
  iotjs_jargs_append_number(&args, status);

  // Make callback.
  MakeCallback(jcallback, JObject::Undefined(), args);

  // Destroy args
  iotjs_jargs_destroy(&args);

  // Release request wrapper.
  delete req_wrap;
}


JHANDLER_FUNCTION(Write) {
  JHANDLER_CHECK(iotjs_jhandler_get_this(jhandler)->IsObject());
  JHANDLER_CHECK(iotjs_jhandler_get_arg_length(jhandler) == 2);
  JHANDLER_CHECK(iotjs_jhandler_get_arg(jhandler, 0)->IsObject());
  JHANDLER_CHECK(iotjs_jhandler_get_arg(jhandler, 1)->IsFunction());

  TcpWrap* tcp_wrap = TcpWrap::FromJObject(iotjs_jhandler_get_this(jhandler));
  IOTJS_ASSERT(tcp_wrap != NULL);

  JObject* jbuffer = iotjs_jhandler_get_arg(jhandler, 0);
  BufferWrap* buffer_wrap = BufferWrap::FromJBuffer(*jbuffer);
  char* buffer = buffer_wrap->buffer();
  int len = buffer_wrap->length();

  uv_buf_t buf;
  buf.base = buffer;
  buf.len = len;

  JObject* arg1 = iotjs_jhandler_get_arg(jhandler, 1);
  WriteReqWrap* req_wrap = new WriteReqWrap(*arg1);

  int err = uv_write(req_wrap->req(),
                     reinterpret_cast<uv_stream_t*>(tcp_wrap->tcp_handle()),
                     &buf,
                     1,
                     AfterWrite
                     );

  if (err) {
    delete req_wrap;
  }

  iotjs_jhandler_return_number(jhandler, err);
}


void OnAlloc(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
  if (suggested_size > IOTJS_MAX_READ_BUFFER_SIZE) {
    suggested_size = IOTJS_MAX_READ_BUFFER_SIZE;
  }

  buf->base = iotjs_buffer_allocate(suggested_size);
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

  iotjs_jargs_t jargs = iotjs_jargs_create(4);
  iotjs_jargs_append_obj(&jargs, &jsocket);
  iotjs_jargs_append_number(&jargs, nread);
  iotjs_jargs_append_bool(&jargs, false);

  if (nread <= 0) {
    if (buf->base != NULL) {
      iotjs_buffer_release(buf->base);
    }
    if (nread < 0) {
      if (nread == UV__EOF) {
        JObject arg(true);
        iotjs_jargs_replace(&jargs, 2, &arg);
      }

      MakeCallback(jonread, JObject::Undefined(), jargs);
    }
    iotjs_jargs_destroy(&jargs);
    return;
  }

  JObject jbuffer(CreateBuffer(static_cast<size_t>(nread)));
  BufferWrap* buffer_wrap = BufferWrap::FromJBuffer(jbuffer);

  buffer_wrap->Copy(buf->base, nread);

  iotjs_jargs_append_obj(&jargs, &jbuffer);
  MakeCallback(jonread, JObject::Undefined(), jargs);

  iotjs_buffer_release(buf->base);
  iotjs_jargs_destroy(&jargs);
}


JHANDLER_FUNCTION(ReadStart) {
  JHANDLER_CHECK(iotjs_jhandler_get_this(jhandler)->IsObject());

  TcpWrap* tcp_wrap = TcpWrap::FromJObject(iotjs_jhandler_get_this(jhandler));
  IOTJS_ASSERT(tcp_wrap != NULL);

  int err = uv_read_start(
      reinterpret_cast<uv_stream_t*>(tcp_wrap->tcp_handle()),
      OnAlloc,
      OnRead);

  iotjs_jhandler_return_number(jhandler, err);
}


static void AfterShutdown(uv_shutdown_t* req, int status) {
  ShutdownReqWrap* req_wrap = reinterpret_cast<ShutdownReqWrap*>(req->data);
  TcpWrap* tcp_wrap = reinterpret_cast<TcpWrap*>(req->handle->data);
  IOTJS_ASSERT(req_wrap != NULL);
  IOTJS_ASSERT(tcp_wrap != NULL);

  // function onShutdown(status)
  JObject jonshutdown(req_wrap->jcallback());
  IOTJS_ASSERT(jonshutdown.IsFunction());

  iotjs_jargs_t args = iotjs_jargs_create(1);
  iotjs_jargs_append_number(&args, status);

  MakeCallback(jonshutdown, JObject::Undefined(), args);

  iotjs_jargs_destroy(&args);

  delete req_wrap;
}


JHANDLER_FUNCTION(Shutdown) {
  JHANDLER_CHECK(iotjs_jhandler_get_this(jhandler)->IsObject());
  JHANDLER_CHECK(iotjs_jhandler_get_arg_length(jhandler) == 1);
  JHANDLER_CHECK(iotjs_jhandler_get_arg(jhandler, 0)->IsFunction());

  TcpWrap* tcp_wrap = TcpWrap::FromJObject(iotjs_jhandler_get_this(jhandler));
  IOTJS_ASSERT(tcp_wrap != NULL);

  JObject* arg0 = iotjs_jhandler_get_arg(jhandler, 0);
  ShutdownReqWrap* req_wrap = new ShutdownReqWrap(*arg0);

  int err = uv_shutdown(req_wrap->req(),
                        reinterpret_cast<uv_stream_t*>(tcp_wrap->tcp_handle()),
                        AfterShutdown);

  if (err) {
    delete req_wrap;
  }

  iotjs_jhandler_return_number(jhandler, err);
}


// Enable/Disable keepalive option.
// [0] enable
// [1] delay
JHANDLER_FUNCTION(SetKeepAlive) {
  JHANDLER_CHECK(iotjs_jhandler_get_this(jhandler)->IsObject());
  JHANDLER_CHECK(iotjs_jhandler_get_arg_length(jhandler) == 2);
  JHANDLER_CHECK(iotjs_jhandler_get_arg(jhandler, 0)->IsNumber());
  JHANDLER_CHECK(iotjs_jhandler_get_arg(jhandler, 1)->IsNumber());

  int enable = iotjs_jhandler_get_arg(jhandler, 0)->GetInt32();
  unsigned delay = (unsigned) iotjs_jhandler_get_arg(jhandler, 1)->GetInt32();

  TcpWrap* wrap = TcpWrap::FromJObject(iotjs_jhandler_get_this(jhandler));
  int err = uv_tcp_keepalive(wrap->tcp_handle(), enable, delay);

  iotjs_jhandler_return_number(jhandler, err);
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
