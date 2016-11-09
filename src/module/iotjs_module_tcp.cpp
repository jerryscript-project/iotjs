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


#include "iotjs_def.h"
#include "iotjs_module_tcp.h"

#include "iotjs_module_buffer.h"
#include "iotjs_handlewrap.h"
#include "iotjs_reqwrap.h"


namespace iotjs {


class TcpWrap : public HandleWrap {
 public:
  explicit TcpWrap(const iotjs_environment_t* env,
                   const iotjs_jval_t* jtcp)
      : HandleWrap(jtcp, reinterpret_cast<uv_handle_t*>(&_handle)) {
    uv_tcp_init(iotjs_environment_loop(env), &_handle);
  }

  static TcpWrap* FromJObject(const iotjs_jval_t* jtcp) {
    TcpWrap* wrap = reinterpret_cast<TcpWrap*>(
            iotjs_jval_get_object_native_handle(jtcp));
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
  JHANDLER_CHECK_THIS(object);
  JHANDLER_CHECK_ARGS(0);

  const iotjs_environment_t* env = iotjs_environment_get();
  const iotjs_jval_t* jtcp = JHANDLER_GET_THIS(object);

  TcpWrap* tcp_wrap = new TcpWrap(env, jtcp);
  IOTJS_ASSERT(iotjs_jval_is_object(tcp_wrap->jobject()));
  IOTJS_ASSERT(iotjs_jval_get_object_native_handle(jtcp) != 0);
}


JHANDLER_FUNCTION(Open) {
}


// Socket close result handler.
void AfterClose(uv_handle_t* handle) {
  HandleWrap* tcp_wrap = HandleWrap::FromHandle(handle);
  IOTJS_ASSERT(tcp_wrap != NULL);

  // tcp object.
  const iotjs_jval_t* jtcp = tcp_wrap->jobject();
  IOTJS_ASSERT(iotjs_jval_is_object(jtcp));

  // callback function.
  iotjs_jval_t jcallback = iotjs_jval_get_property(jtcp, "onclose");
  if (iotjs_jval_is_function(&jcallback)) {
    iotjs_make_callback(&jcallback,
                        iotjs_jval_get_undefined(), iotjs_jargs_get_empty());
  }
  iotjs_jval_destroy(&jcallback);
}


void DoClose(iotjs_jhandler_t* jhandler) {
  JHANDLER_CHECK_THIS(object);
  JHANDLER_CHECK_ARGS(0);

  const iotjs_jval_t* jtcp = JHANDLER_GET_THIS(object);
  HandleWrap* wrap = reinterpret_cast<HandleWrap*>(
          iotjs_jval_get_object_native_handle(jtcp));

  // close uv handle, `AfterClose` will be called after socket closed.
  wrap->Close(AfterClose);
}


// Close socket
JHANDLER_FUNCTION(Close) {
  DoClose(jhandler);
}


// Socket binding, this function would be called from server socket before
// start listening.
// [0] address
// [1] port
JHANDLER_FUNCTION(Bind) {
  JHANDLER_CHECK_THIS(object);
  JHANDLER_CHECK_ARGS(2, string, number);

  iotjs_string_t address = JHANDLER_GET_ARG(0, string);
  int port = JHANDLER_GET_ARG(1, number);

  sockaddr_in addr;
  int err = uv_ip4_addr(iotjs_string_data(&address), port, &addr);

  if (err == 0) {
    TcpWrap* wrap = TcpWrap::FromJObject(JHANDLER_GET_THIS(object));
    err = uv_tcp_bind(wrap->tcp_handle(),
                      reinterpret_cast<const sockaddr*>(&addr), 0);
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
  // function afterConnect(status)
  const iotjs_jval_t* jcallback = req_wrap->jcallback();
  IOTJS_ASSERT(iotjs_jval_is_function(jcallback));

  // Only parameter is status code.
  iotjs_jargs_t args = iotjs_jargs_create(1);
  iotjs_jargs_append_number(&args, status);

  // Make callback.
  iotjs_make_callback(jcallback, iotjs_jval_get_undefined(), &args);

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
  JHANDLER_CHECK_THIS(object);
  JHANDLER_CHECK_ARGS(3, string, number, function);

  iotjs_string_t address = JHANDLER_GET_ARG(0, string);
  int port = JHANDLER_GET_ARG(1, number);
  const iotjs_jval_t* jcallback = JHANDLER_GET_ARG(2, function);

  sockaddr_in addr;
  int err = uv_ip4_addr(iotjs_string_data(&address), port, &addr);

  if (err == 0) {
    // Get tcp wrapper from javascript socket object.
    TcpWrap* tcp_wrap = TcpWrap::FromJObject(JHANDLER_GET_THIS(object));

    // Create connection request wrapper.
    ConnectReqWrap* req_wrap = new ConnectReqWrap(jcallback);

    // Create connection request.
    err = uv_tcp_connect(req_wrap->req(), tcp_wrap->tcp_handle(),
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
  const iotjs_jval_t* jtcp = tcp_wrap->jobject();
  IOTJS_ASSERT(iotjs_jval_is_object(jtcp));

  // `onconnection` callback.
  iotjs_jval_t jonconnection = iotjs_jval_get_property(jtcp, "onconnection");
  IOTJS_ASSERT(iotjs_jval_is_function(&jonconnection));

  // The callback takes two parameter
  // [0] status
  // [1] client tcp object
  iotjs_jargs_t args = iotjs_jargs_create(2);
  iotjs_jargs_append_number(&args, status);

  if (status == 0) {
    // Create client socket handle wrapper.
    iotjs_jval_t jcreate_tcp = iotjs_jval_get_property(jtcp, "createTCP");
    IOTJS_ASSERT(iotjs_jval_is_function(&jcreate_tcp));

    iotjs_jval_t jclient_tcp = iotjs_jhelper_call_ok(&jcreate_tcp,
                                                     iotjs_jval_get_undefined(),
                                                     iotjs_jargs_get_empty());
    IOTJS_ASSERT(iotjs_jval_is_object(&jclient_tcp));

    TcpWrap* tcp_wrap = reinterpret_cast<TcpWrap*>(
            iotjs_jval_get_object_native_handle(&jclient_tcp));

    uv_stream_t* client_handle =
        reinterpret_cast<uv_stream_t*>(tcp_wrap->tcp_handle());

    int err = uv_accept(handle, client_handle);
    if (err) {
      return;
    }

    iotjs_jargs_append_jval(&args, &jclient_tcp);
    iotjs_jval_destroy(&jcreate_tcp);
    iotjs_jval_destroy(&jclient_tcp);
  }

  iotjs_make_callback(&jonconnection, jtcp, &args);

  iotjs_jval_destroy(&jonconnection);
  iotjs_jargs_destroy(&args);
}


JHANDLER_FUNCTION(Listen) {
  JHANDLER_CHECK_THIS(object);
  JHANDLER_CHECK_ARGS(1, number);

  TcpWrap* tcp_wrap = TcpWrap::FromJObject(JHANDLER_GET_THIS(object));

  int backlog = JHANDLER_GET_ARG(0, number);

  int err = uv_listen(reinterpret_cast<uv_stream_t*>(tcp_wrap->tcp_handle()),
                      backlog, OnConnection);

  iotjs_jhandler_return_number(jhandler, err);
}


void AfterWrite(uv_write_t* req, int status) {
  WriteReqWrap* req_wrap = reinterpret_cast<WriteReqWrap*>(req->data);
  TcpWrap* tcp_wrap = reinterpret_cast<TcpWrap*>(req->handle->data);
  IOTJS_ASSERT(req_wrap != NULL);
  IOTJS_ASSERT(tcp_wrap != NULL);

  // Take callback function object.
  const iotjs_jval_t* jcallback = req_wrap->jcallback();

  // Only parameter is status code.
  iotjs_jargs_t args = iotjs_jargs_create(1);
  iotjs_jargs_append_number(&args, status);

  // Make callback.
  iotjs_make_callback(jcallback, iotjs_jval_get_undefined(), &args);

  // Destroy args
  iotjs_jargs_destroy(&args);

  // Release request wrapper.
  delete req_wrap;
}


JHANDLER_FUNCTION(Write) {
  JHANDLER_CHECK_THIS(object);
  JHANDLER_CHECK_ARGS(2, object, function);

  TcpWrap* tcp_wrap = TcpWrap::FromJObject(JHANDLER_GET_THIS(object));
  IOTJS_ASSERT(tcp_wrap != NULL);

  const iotjs_jval_t* jbuffer = JHANDLER_GET_ARG(0, object);
  BufferWrap* buffer_wrap = BufferWrap::FromJBuffer(jbuffer);
  char* buffer = buffer_wrap->buffer();
  int len = buffer_wrap->length();

  uv_buf_t buf;
  buf.base = buffer;
  buf.len = len;

  const iotjs_jval_t* arg1 = JHANDLER_GET_ARG(1, object);
  WriteReqWrap* req_wrap = new WriteReqWrap(arg1);

  int err = uv_write(req_wrap->req(),
                     reinterpret_cast<uv_stream_t*>(tcp_wrap->tcp_handle()),
                     &buf, 1, AfterWrite);

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
  const iotjs_jval_t* jtcp = tcp_wrap->jobject();
  IOTJS_ASSERT(iotjs_jval_is_object(jtcp));

  // socket object
  iotjs_jval_t jsocket = iotjs_jval_get_property(jtcp, "owner");
  IOTJS_ASSERT(iotjs_jval_is_object(&jsocket));

  // onread callback
  iotjs_jval_t jonread = iotjs_jval_get_property(jtcp, "onread");
  IOTJS_ASSERT(iotjs_jval_is_function(&jonread));

  iotjs_jargs_t jargs = iotjs_jargs_create(4);
  iotjs_jargs_append_jval(&jargs, &jsocket);
  iotjs_jargs_append_number(&jargs, nread);
  iotjs_jargs_append_bool(&jargs, false);

  if (nread <= 0) {
    if (buf->base != NULL) {
      iotjs_buffer_release(buf->base);
    }
    if (nread < 0) {
      if (nread == UV__EOF) {
        iotjs_jargs_replace(&jargs, 2, iotjs_jval_get_boolean(true));
      }

      iotjs_make_callback(&jonread, iotjs_jval_get_undefined(), &jargs);
    }
  } else {

    iotjs_jval_t jbuffer = CreateBuffer(static_cast<size_t>(nread));
    BufferWrap* buffer_wrap = BufferWrap::FromJBuffer(&jbuffer);

    buffer_wrap->Copy(buf->base, nread);

    iotjs_jargs_append_jval(&jargs, &jbuffer);
    iotjs_make_callback(&jonread, iotjs_jval_get_undefined(), &jargs);

    iotjs_jval_destroy(&jbuffer);
    iotjs_buffer_release(buf->base);
  }

  iotjs_jargs_destroy(&jargs);
  iotjs_jval_destroy(&jonread);
  iotjs_jval_destroy(&jsocket);
}


JHANDLER_FUNCTION(ReadStart) {
  JHANDLER_CHECK_THIS(object);

  TcpWrap* tcp_wrap = TcpWrap::FromJObject(JHANDLER_GET_THIS(object));
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
  const iotjs_jval_t* jonshutdown = req_wrap->jcallback();
  IOTJS_ASSERT(iotjs_jval_is_function(jonshutdown));

  iotjs_jargs_t args = iotjs_jargs_create(1);
  iotjs_jargs_append_number(&args, status);

  iotjs_make_callback(jonshutdown, iotjs_jval_get_undefined(), &args);

  iotjs_jargs_destroy(&args);

  delete req_wrap;
}


JHANDLER_FUNCTION(Shutdown) {
  JHANDLER_CHECK_THIS(object);
  JHANDLER_CHECK_ARGS(1, function);


  iotjs_jhandler_get_arg(jhandler, 0);
  TcpWrap* tcp_wrap = TcpWrap::FromJObject(JHANDLER_GET_THIS(object));
  IOTJS_ASSERT(tcp_wrap != NULL);

  const iotjs_jval_t* arg0 = JHANDLER_GET_ARG(0, object);
  ShutdownReqWrap* req_wrap = new ShutdownReqWrap(arg0);

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
  JHANDLER_CHECK_THIS(object);
  JHANDLER_CHECK_ARGS(2, number, number);

  int enable = JHANDLER_GET_ARG(0, number);
  unsigned delay = JHANDLER_GET_ARG(1, number);

  TcpWrap* wrap = TcpWrap::FromJObject(JHANDLER_GET_THIS(object));
  int err = uv_tcp_keepalive(wrap->tcp_handle(), enable, delay);

  iotjs_jhandler_return_number(jhandler, err);
}


// used in iotjs_module_udp.cpp
void AddressToJS(const iotjs_jval_t* obj, const sockaddr* addr) {
  char ip[INET6_ADDRSTRLEN];
  const sockaddr_in *a4;
  const sockaddr_in6 *a6;
  int port;

  switch (addr->sa_family) {
    case AF_INET6: {
      a6 = reinterpret_cast<const sockaddr_in6*>(addr);
      uv_inet_ntop(AF_INET6, &a6->sin6_addr, ip, sizeof ip);
      port = ntohs(a6->sin6_port);
      iotjs_jval_set_property_string_raw(obj, "address", ip);
      iotjs_jval_set_property_string_raw(obj, "family", "IPv6");
      iotjs_jval_set_property_number(obj, "port", port);
      break;
    }

    case AF_INET: {
      a4 = reinterpret_cast<const sockaddr_in*>(addr);
      uv_inet_ntop(AF_INET, &a4->sin_addr, ip, sizeof ip);
      port = ntohs(a4->sin_port);
      iotjs_jval_set_property_string_raw(obj, "address", ip);
      iotjs_jval_set_property_string_raw(obj, "family", "IPv4");
      iotjs_jval_set_property_number(obj, "port", port);
      break;
    }

    default: {
      iotjs_jval_set_property_string_raw(obj, "address", "");
      break;
    }
  }
}

GetSockNameFunction(TcpWrap, tcp_handle, uv_tcp_getsockname);


JHANDLER_FUNCTION(GetSockeName) {
  DoGetSockName(jhandler);
}

iotjs_jval_t InitTcp() {
  iotjs_jval_t tcp = iotjs_jval_create_function(TCP);

  iotjs_jval_t prototype = iotjs_jval_create_object();
  iotjs_jval_set_property_jval(&tcp, "prototype", &prototype);

  iotjs_jval_set_method(&prototype, "open", Open);
  iotjs_jval_set_method(&prototype, "close", Close);
  iotjs_jval_set_method(&prototype, "connect", Connect);
  iotjs_jval_set_method(&prototype, "bind", Bind);
  iotjs_jval_set_method(&prototype, "listen", Listen);
  iotjs_jval_set_method(&prototype, "write", Write);
  iotjs_jval_set_method(&prototype, "readStart", ReadStart);
  iotjs_jval_set_method(&prototype, "shutdown", Shutdown);
  iotjs_jval_set_method(&prototype, "setKeepAlive", SetKeepAlive);
  iotjs_jval_set_method(&prototype, "getsockname", GetSockeName);

  iotjs_jval_destroy(&prototype);

  return tcp;
}


} // namespace iotjs


extern "C" {

iotjs_jval_t InitTcp() {
  return iotjs::InitTcp();
}

} // extern "C"
