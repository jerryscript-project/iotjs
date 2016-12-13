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
#include "iotjs_module_tcp.h"

#include "iotjs_handlewrap.h"
#include "iotjs_module_buffer.h"
#include "iotjs_reqwrap.h"


static void iotjs_tcpwrap_destroy(iotjs_tcpwrap_t* tcpwrap);


iotjs_tcpwrap_t* iotjs_tcpwrap_create(const iotjs_jval_t* jtcp) {
  iotjs_tcpwrap_t* tcpwrap = IOTJS_ALLOC(iotjs_tcpwrap_t);
  IOTJS_VALIDATED_STRUCT_CONSTRUCTOR(iotjs_tcpwrap_t, tcpwrap);

  iotjs_handlewrap_initialize(&_this->handlewrap, jtcp,
                              (uv_handle_t*)(&_this->handle),
                              (JFreeHandlerType)iotjs_tcpwrap_destroy);

  const iotjs_environment_t* env = iotjs_environment_get();
  uv_tcp_init(iotjs_environment_loop(env), &_this->handle);

  return tcpwrap;
}


static void iotjs_tcpwrap_destroy(iotjs_tcpwrap_t* tcpwrap) {
  IOTJS_VALIDATED_STRUCT_DESTRUCTOR(iotjs_tcpwrap_t, tcpwrap);
  iotjs_handlewrap_destroy(&_this->handlewrap);
  IOTJS_RELEASE(tcpwrap);
}


iotjs_tcpwrap_t* iotjs_tcpwrap_from_handle(uv_tcp_t* tcp_handle) {
  uv_handle_t* handle = (uv_handle_t*)(tcp_handle);
  iotjs_handlewrap_t* handlewrap = iotjs_handlewrap_from_handle(handle);
  iotjs_tcpwrap_t* tcpwrap = (iotjs_tcpwrap_t*)handlewrap;
  IOTJS_ASSERT(iotjs_tcpwrap_tcp_handle(tcpwrap) == tcp_handle);
  return tcpwrap;
}


iotjs_tcpwrap_t* iotjs_tcpwrap_from_jobject(const iotjs_jval_t* jtcp) {
  iotjs_handlewrap_t* handlewrap = iotjs_handlewrap_from_jobject(jtcp);
  return (iotjs_tcpwrap_t*)handlewrap;
}


uv_tcp_t* iotjs_tcpwrap_tcp_handle(iotjs_tcpwrap_t* tcpwrap) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_tcpwrap_t, tcpwrap);
  uv_handle_t* handle = iotjs_handlewrap_get_uv_handle(&_this->handlewrap);
  return (uv_tcp_t*)handle;
}


iotjs_jval_t* iotjs_tcpwrap_jobject(iotjs_tcpwrap_t* tcpwrap) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_tcpwrap_t, tcpwrap);
  return iotjs_handlewrap_jobject(&_this->handlewrap);
}


#define THIS iotjs_connect_reqwrap_t* connect_reqwrap


static void iotjs_connect_reqwrap_destroy(THIS);


iotjs_connect_reqwrap_t* iotjs_connect_reqwrap_create(
    const iotjs_jval_t* jcallback) {
  iotjs_connect_reqwrap_t* connect_reqwrap =
      IOTJS_ALLOC(iotjs_connect_reqwrap_t);
  IOTJS_VALIDATED_STRUCT_CONSTRUCTOR(iotjs_connect_reqwrap_t, connect_reqwrap);
  iotjs_reqwrap_initialize(&_this->reqwrap, jcallback, (uv_req_t*)&_this->req);
  return connect_reqwrap;
}


static void iotjs_connect_reqwrap_destroy(THIS) {
  IOTJS_VALIDATED_STRUCT_DESTRUCTOR(iotjs_connect_reqwrap_t, connect_reqwrap);
  iotjs_reqwrap_destroy(&_this->reqwrap);
  IOTJS_RELEASE(connect_reqwrap);
}


void iotjs_connect_reqwrap_dispatched(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_connect_reqwrap_t, connect_reqwrap);
  iotjs_connect_reqwrap_destroy(connect_reqwrap);
}


uv_connect_t* iotjs_connect_reqwrap_req(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_connect_reqwrap_t, connect_reqwrap);
  return &_this->req;
}


const iotjs_jval_t* iotjs_connect_reqwrap_jcallback(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_connect_reqwrap_t, connect_reqwrap);
  return iotjs_reqwrap_jcallback(&_this->reqwrap);
}

#undef THIS


#define THIS iotjs_write_reqwrap_t* write_reqwrap


static void iotjs_write_reqwrap_destroy(THIS);


iotjs_write_reqwrap_t* iotjs_write_reqwrap_create(
    const iotjs_jval_t* jcallback) {
  iotjs_write_reqwrap_t* write_reqwrap = IOTJS_ALLOC(iotjs_write_reqwrap_t);
  IOTJS_VALIDATED_STRUCT_CONSTRUCTOR(iotjs_write_reqwrap_t, write_reqwrap);
  iotjs_reqwrap_initialize(&_this->reqwrap, jcallback, (uv_req_t*)&_this->req);
  return write_reqwrap;
}


static void iotjs_write_reqwrap_destroy(THIS) {
  IOTJS_VALIDATED_STRUCT_DESTRUCTOR(iotjs_write_reqwrap_t, write_reqwrap);
  iotjs_reqwrap_destroy(&_this->reqwrap);
  IOTJS_RELEASE(write_reqwrap);
}


void iotjs_write_reqwrap_dispatched(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_write_reqwrap_t, write_reqwrap);
  iotjs_write_reqwrap_destroy(write_reqwrap);
}


uv_write_t* iotjs_write_reqwrap_req(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_write_reqwrap_t, write_reqwrap);
  return &_this->req;
}


const iotjs_jval_t* iotjs_write_reqwrap_jcallback(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_write_reqwrap_t, write_reqwrap);
  return iotjs_reqwrap_jcallback(&_this->reqwrap);
}

#undef THIS


#define THIS iotjs_shutdown_reqwrap_t* shutdown_reqwrap


static void iotjs_shutdown_reqwrap_destroy(THIS);


iotjs_shutdown_reqwrap_t* iotjs_shutdown_reqwrap_create(
    const iotjs_jval_t* jcallback) {
  iotjs_shutdown_reqwrap_t* shutdown_reqwrap =
      IOTJS_ALLOC(iotjs_shutdown_reqwrap_t);
  IOTJS_VALIDATED_STRUCT_CONSTRUCTOR(iotjs_shutdown_reqwrap_t,
                                     shutdown_reqwrap);
  iotjs_reqwrap_initialize(&_this->reqwrap, jcallback, (uv_req_t*)&_this->req);
  return shutdown_reqwrap;
}


static void iotjs_shutdown_reqwrap_destroy(THIS) {
  IOTJS_VALIDATED_STRUCT_DESTRUCTOR(iotjs_shutdown_reqwrap_t, shutdown_reqwrap);
  iotjs_reqwrap_destroy(&_this->reqwrap);
  IOTJS_RELEASE(shutdown_reqwrap);
}


void iotjs_shutdown_reqwrap_dispatched(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_shutdown_reqwrap_t, shutdown_reqwrap);
  iotjs_shutdown_reqwrap_destroy(shutdown_reqwrap);
}


uv_shutdown_t* iotjs_shutdown_reqwrap_req(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_shutdown_reqwrap_t, shutdown_reqwrap);
  return &_this->req;
}


const iotjs_jval_t* iotjs_shutdown_reqwrap_jcallback(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_shutdown_reqwrap_t, shutdown_reqwrap);
  return iotjs_reqwrap_jcallback(&_this->reqwrap);
}

#undef THIS


JHANDLER_FUNCTION(TCP) {
  JHANDLER_CHECK_THIS(object);
  JHANDLER_CHECK_ARGS(0);

  const iotjs_jval_t* jtcp = JHANDLER_GET_THIS(object);
  iotjs_tcpwrap_t* tcp_wrap = iotjs_tcpwrap_create(jtcp);
}


JHANDLER_FUNCTION(Open) {
}


// Socket close result handler.
void AfterClose(uv_handle_t* handle) {
  iotjs_handlewrap_t* wrap = iotjs_handlewrap_from_handle(handle);

  // tcp object.
  const iotjs_jval_t* jtcp = iotjs_handlewrap_jobject(wrap);

  // callback function.
  iotjs_jval_t jcallback = iotjs_jval_get_property(jtcp, "onclose");
  if (iotjs_jval_is_function(&jcallback)) {
    iotjs_make_callback(&jcallback, iotjs_jval_get_undefined(),
                        iotjs_jargs_get_empty());
  }
  iotjs_jval_destroy(&jcallback);
}


// Close socket
JHANDLER_FUNCTION(Close) {
  JHANDLER_CHECK_THIS(object);
  JHANDLER_CHECK_ARGS(0);

  const iotjs_jval_t* jtcp = JHANDLER_GET_THIS(object);
  iotjs_handlewrap_t* wrap = iotjs_handlewrap_from_jobject(jtcp);

  // close uv handle, `AfterClose` will be called after socket closed.
  iotjs_handlewrap_close(wrap, AfterClose);
}


// Socket binding, this function would be called from server socket before
// start listening.
// [0] address
// [1] port
JHANDLER_FUNCTION(Bind) {
  JHANDLER_CHECK_THIS(object);
  JHANDLER_CHECK_ARGS(2, string, number);

  const iotjs_jval_t* jtcp = JHANDLER_GET_THIS(object);
  iotjs_string_t address = JHANDLER_GET_ARG(0, string);
  int port = JHANDLER_GET_ARG(1, number);

  sockaddr_in addr;
  int err = uv_ip4_addr(iotjs_string_data(&address), port, &addr);

  if (err == 0) {
    iotjs_tcpwrap_t* tcp_wrap = iotjs_tcpwrap_from_jobject(jtcp);
    err = uv_tcp_bind(iotjs_tcpwrap_tcp_handle(tcp_wrap),
                      (const sockaddr*)(&addr), 0);
  }

  iotjs_jhandler_return_number(jhandler, err);

  iotjs_string_destroy(&address);
}


// Connection request result handler.
static void AfterConnect(uv_connect_t* req, int status) {
  iotjs_connect_reqwrap_t* req_wrap = (iotjs_connect_reqwrap_t*)(req->data);
  IOTJS_ASSERT(req_wrap != NULL);

  // Take callback function object.
  // function afterConnect(status)
  const iotjs_jval_t* jcallback = iotjs_connect_reqwrap_jcallback(req_wrap);
  IOTJS_ASSERT(iotjs_jval_is_function(jcallback));

  // Only parameter is status code.
  iotjs_jargs_t args = iotjs_jargs_create(1);
  iotjs_jargs_append_number(&args, status);

  // Make callback.
  iotjs_make_callback(jcallback, iotjs_jval_get_undefined(), &args);

  // Destroy args
  iotjs_jargs_destroy(&args);

  // Release request wrapper.
  iotjs_connect_reqwrap_dispatched(req_wrap);
}


// Create a connection using the socket.
// [0] address
// [1] port
// [2] callback
JHANDLER_FUNCTION(Connect) {
  JHANDLER_CHECK_THIS(object);
  JHANDLER_CHECK_ARGS(3, string, number, function);

  const iotjs_jval_t* jtcp = JHANDLER_GET_THIS(object);
  iotjs_string_t address = JHANDLER_GET_ARG(0, string);
  int port = JHANDLER_GET_ARG(1, number);
  const iotjs_jval_t* jcallback = JHANDLER_GET_ARG(2, function);

  sockaddr_in addr;
  int err = uv_ip4_addr(iotjs_string_data(&address), port, &addr);

  if (err == 0) {
    // Get tcp wrapper from javascript socket object.
    iotjs_tcpwrap_t* tcp_wrap = iotjs_tcpwrap_from_jobject(jtcp);

    // Create connection request wrapper.
    iotjs_connect_reqwrap_t* req_wrap = iotjs_connect_reqwrap_create(jcallback);

    // Create connection request.
    err = uv_tcp_connect(iotjs_connect_reqwrap_req(req_wrap),
                         iotjs_tcpwrap_tcp_handle(tcp_wrap),
                         (const sockaddr*)(&addr), AfterConnect);

    if (err) {
      iotjs_connect_reqwrap_dispatched(req_wrap);
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
  iotjs_tcpwrap_t* tcp_wrap = iotjs_tcpwrap_from_handle((uv_tcp_t*)handle);

  // Tcp object
  const iotjs_jval_t* jtcp = iotjs_tcpwrap_jobject(tcp_wrap);

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

    iotjs_jval_t jclient_tcp =
        iotjs_jhelper_call_ok(&jcreate_tcp, iotjs_jval_get_undefined(),
                              iotjs_jargs_get_empty());
    IOTJS_ASSERT(iotjs_jval_is_object(&jclient_tcp));

    iotjs_tcpwrap_t* tcp_wrap =
        (iotjs_tcpwrap_t*)(iotjs_jval_get_object_native_handle(&jclient_tcp));

    uv_stream_t* client_handle =
        (uv_stream_t*)(iotjs_tcpwrap_tcp_handle(tcp_wrap));

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

  const iotjs_jval_t* jtcp = JHANDLER_GET_THIS(object);
  iotjs_tcpwrap_t* tcp_wrap = iotjs_tcpwrap_from_jobject(jtcp);

  int backlog = JHANDLER_GET_ARG(0, number);

  int err = uv_listen((uv_stream_t*)(iotjs_tcpwrap_tcp_handle(tcp_wrap)),
                      backlog, OnConnection);

  iotjs_jhandler_return_number(jhandler, err);
}


void AfterWrite(uv_write_t* req, int status) {
  iotjs_write_reqwrap_t* req_wrap = (iotjs_write_reqwrap_t*)(req->data);
  iotjs_tcpwrap_t* tcp_wrap = (iotjs_tcpwrap_t*)(req->handle->data);
  IOTJS_ASSERT(req_wrap != NULL);
  IOTJS_ASSERT(tcp_wrap != NULL);

  // Take callback function object.
  const iotjs_jval_t* jcallback = iotjs_write_reqwrap_jcallback(req_wrap);

  // Only parameter is status code.
  iotjs_jargs_t args = iotjs_jargs_create(1);
  iotjs_jargs_append_number(&args, status);

  // Make callback.
  iotjs_make_callback(jcallback, iotjs_jval_get_undefined(), &args);

  // Destroy args
  iotjs_jargs_destroy(&args);

  // Release request wrapper.
  iotjs_write_reqwrap_dispatched(req_wrap);
}


JHANDLER_FUNCTION(Write) {
  JHANDLER_CHECK_THIS(object);
  JHANDLER_CHECK_ARGS(2, object, function);

  const iotjs_jval_t* jtcp = JHANDLER_GET_THIS(object);
  iotjs_tcpwrap_t* tcp_wrap = iotjs_tcpwrap_from_jobject(jtcp);

  const iotjs_jval_t* jbuffer = JHANDLER_GET_ARG(0, object);
  iotjs_bufferwrap_t* buffer_wrap = iotjs_bufferwrap_from_jbuffer(jbuffer);
  char* buffer = iotjs_bufferwrap_buffer(buffer_wrap);
  int len = iotjs_bufferwrap_length(buffer_wrap);

  uv_buf_t buf;
  buf.base = buffer;
  buf.len = len;

  const iotjs_jval_t* arg1 = JHANDLER_GET_ARG(1, object);
  iotjs_write_reqwrap_t* req_wrap = iotjs_write_reqwrap_create(arg1);

  int err = uv_write(iotjs_write_reqwrap_req(req_wrap),
                     (uv_stream_t*)(iotjs_tcpwrap_tcp_handle(tcp_wrap)), &buf,
                     1, AfterWrite);

  if (err) {
    iotjs_write_reqwrap_dispatched(req_wrap);
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
  iotjs_tcpwrap_t* tcp_wrap = iotjs_tcpwrap_from_handle((uv_tcp_t*)handle);

  // tcp handle
  const iotjs_jval_t* jtcp = iotjs_tcpwrap_jobject(tcp_wrap);

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
    iotjs_jval_t jbuffer = iotjs_bufferwrap_create_buffer(nread);
    iotjs_bufferwrap_t* buffer_wrap = iotjs_bufferwrap_from_jbuffer(&jbuffer);

    iotjs_bufferwrap_copy(buffer_wrap, buf->base, nread);

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

  const iotjs_jval_t* jtcp = JHANDLER_GET_THIS(object);
  iotjs_tcpwrap_t* tcp_wrap = iotjs_tcpwrap_from_jobject(jtcp);

  int err = uv_read_start((uv_stream_t*)(iotjs_tcpwrap_tcp_handle(tcp_wrap)),
                          OnAlloc, OnRead);

  iotjs_jhandler_return_number(jhandler, err);
}


static void AfterShutdown(uv_shutdown_t* req, int status) {
  iotjs_shutdown_reqwrap_t* req_wrap = (iotjs_shutdown_reqwrap_t*)(req->data);
  iotjs_tcpwrap_t* tcp_wrap = (iotjs_tcpwrap_t*)(req->handle->data);
  IOTJS_ASSERT(req_wrap != NULL);
  IOTJS_ASSERT(tcp_wrap != NULL);

  // function onShutdown(status)
  const iotjs_jval_t* jonshutdown = iotjs_shutdown_reqwrap_jcallback(req_wrap);
  IOTJS_ASSERT(iotjs_jval_is_function(jonshutdown));

  iotjs_jargs_t args = iotjs_jargs_create(1);
  iotjs_jargs_append_number(&args, status);

  iotjs_make_callback(jonshutdown, iotjs_jval_get_undefined(), &args);

  iotjs_jargs_destroy(&args);

  iotjs_shutdown_reqwrap_dispatched(req_wrap);
}


JHANDLER_FUNCTION(Shutdown) {
  JHANDLER_CHECK_THIS(object);
  JHANDLER_CHECK_ARGS(1, function);

  const iotjs_jval_t* jtcp = JHANDLER_GET_THIS(object);
  iotjs_tcpwrap_t* tcp_wrap = iotjs_tcpwrap_from_jobject(jtcp);

  const iotjs_jval_t* arg0 = JHANDLER_GET_ARG(0, object);
  iotjs_shutdown_reqwrap_t* req_wrap = iotjs_shutdown_reqwrap_create(arg0);

  int err = uv_shutdown(iotjs_shutdown_reqwrap_req(req_wrap),
                        (uv_stream_t*)(iotjs_tcpwrap_tcp_handle(tcp_wrap)),
                        AfterShutdown);

  if (err) {
    iotjs_shutdown_reqwrap_dispatched(req_wrap);
  }

  iotjs_jhandler_return_number(jhandler, err);
}


// Enable/Disable keepalive option.
// [0] enable
// [1] delay
JHANDLER_FUNCTION(SetKeepAlive) {
  JHANDLER_CHECK_THIS(object);
  JHANDLER_CHECK_ARGS(2, number, number);

  const iotjs_jval_t* jtcp = JHANDLER_GET_THIS(object);
  int enable = JHANDLER_GET_ARG(0, number);
  unsigned delay = JHANDLER_GET_ARG(1, number);

  iotjs_tcpwrap_t* tcp_wrap = iotjs_tcpwrap_from_jobject(jtcp);
  int err = uv_tcp_keepalive(iotjs_tcpwrap_tcp_handle(tcp_wrap), enable, delay);

  iotjs_jhandler_return_number(jhandler, err);
}


// used in iotjs_module_udp.cpp
void AddressToJS(const iotjs_jval_t* obj, const sockaddr* addr) {
  char ip[INET6_ADDRSTRLEN];
  const sockaddr_in* a4;
  const sockaddr_in6* a6;
  int port;

  switch (addr->sa_family) {
    case AF_INET6: {
      a6 = (const sockaddr_in6*)(addr);
      uv_inet_ntop(AF_INET6, &a6->sin6_addr, ip, sizeof ip);
      port = ntohs(a6->sin6_port);
      iotjs_jval_set_property_string_raw(obj, "address", ip);
      iotjs_jval_set_property_string_raw(obj, "family", "IPv6");
      iotjs_jval_set_property_number(obj, "port", port);
      break;
    }

    case AF_INET: {
      a4 = (const sockaddr_in*)(addr);
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

GetSockNameFunction(tcpwrap, tcp_handle, uv_tcp_getsockname);


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
