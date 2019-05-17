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

#include "iotjs_module_buffer.h"
#include "iotjs_uv_handle.h"
#include "iotjs_uv_request.h"

static const jerry_object_native_info_t this_module_native_info = { NULL };


void iotjs_tcp_object_init(jerry_value_t jtcp) {
  // uv_tcp_t* can be handled as uv_handle_t* or even as uv_stream_t*
  uv_handle_t* handle = iotjs_uv_handle_create(sizeof(uv_tcp_t), jtcp,
                                               &this_module_native_info, 0);

  const iotjs_environment_t* env = iotjs_environment_get();
  uv_tcp_init(iotjs_environment_loop(env), (uv_tcp_t*)handle);
}


static void iotjs_tcp_report_req_result(uv_req_t* req, int status) {
  IOTJS_ASSERT(req != NULL);
  // Take callback function object.
  jerry_value_t jcallback = *IOTJS_UV_REQUEST_JSCALLBACK(req);

  // Only parameter is status code.
  jerry_value_t jstatus = jerry_create_number(status);

  // Make callback.
  iotjs_invoke_callback(jcallback, jerry_create_undefined(), &jstatus, 1);

  // Destroy args
  jerry_release_value(jstatus);

  // Release request.
  iotjs_uv_request_destroy(req);
}


JS_FUNCTION(tcp_constructor) {
  DJS_CHECK_THIS();

  jerry_value_t jtcp = JS_GET_THIS();
  iotjs_tcp_object_init(jtcp);
  return jerry_create_undefined();
}


// Socket close result handler.
void after_close(uv_handle_t* handle) {
  jerry_value_t jtcp = IOTJS_UV_HANDLE_DATA(handle)->jobject;

  // callback function.
  jerry_value_t jcallback =
      iotjs_jval_get_property(jtcp, IOTJS_MAGIC_STRING_ONCLOSE);
  if (jerry_value_is_function(jcallback)) {
    iotjs_invoke_callback(jcallback, jerry_create_undefined(), NULL, 0);
  }
  jerry_release_value(jcallback);
}


// Close socket
JS_FUNCTION(tcp_close) {
  JS_DECLARE_PTR(jthis, uv_handle_t, uv_handle);

  iotjs_uv_handle_close(uv_handle, after_close);
  return jerry_create_undefined();
}


// Socket binding, this function would be called from server socket before
// start listening.
// [0] address
// [1] port
JS_FUNCTION(tcp_bind) {
  JS_DECLARE_PTR(jthis, uv_tcp_t, tcp_handle);

  DJS_CHECK_ARGS(2, string, number);

  iotjs_string_t address = JS_GET_ARG(0, string);
  int port = JS_GET_ARG(1, number);

  sockaddr_in addr;
  int err = uv_ip4_addr(iotjs_string_data(&address), port, &addr);

  if (err == 0) {
    err = uv_tcp_bind(tcp_handle, (const sockaddr*)(&addr), 0);
  }

  iotjs_string_destroy(&address);

  return jerry_create_number(err);
}


// Connection request result handler.
static void after_connect(uv_connect_t* req, int status) {
  iotjs_tcp_report_req_result((uv_req_t*)req, status);
}

// Create a connection using the socket.
// [0] address
// [1] port
// [2] callback
JS_FUNCTION(tcp_connect) {
  JS_DECLARE_PTR(jthis, uv_tcp_t, tcp_handle);

  DJS_CHECK_ARGS(3, string, number, function);

  iotjs_string_t address = JS_GET_ARG(0, string);
  int port = JS_GET_ARG(1, number);
  jerry_value_t jcallback = JS_GET_ARG(2, function);

  sockaddr_in addr;
  int err = uv_ip4_addr(iotjs_string_data(&address), port, &addr);

  if (err == 0) {
    // Create connection request and configure request data.
    uv_req_t* req_connect =
        iotjs_uv_request_create(sizeof(uv_connect_t), jcallback, 0);

    // Create connection request.
    err = uv_tcp_connect((uv_connect_t*)req_connect, tcp_handle,
                         (const sockaddr*)(&addr), after_connect);

    if (err) {
      iotjs_uv_request_destroy(req_connect);
    }
  }

  iotjs_string_destroy(&address);

  return jerry_create_number(err);
}


// A client socket wants to connect to this server.
// Parameters:
//   * uv_stream_t* handle - server handle
//   * int status - status code
static void on_connection(uv_stream_t* handle, int status) {
  jerry_value_t jtcp = IOTJS_UV_HANDLE_DATA(handle)->jobject;

  // `onconnection` callback.
  jerry_value_t jonconnection =
      iotjs_jval_get_property(jtcp, IOTJS_MAGIC_STRING_ONCONNECTION);
  IOTJS_ASSERT(jerry_value_is_function(jonconnection));

  // The callback takes two parameter
  // [0] status
  // [1] client tcp object
  size_t argc = 1;
  jerry_value_t args[2] = { jerry_create_number(status), 0 };

  if (status == 0) {
    // Create client socket handle wrapper.
    jerry_value_t jcreate_tcp =
        iotjs_jval_get_property(jtcp, IOTJS_MAGIC_STRING_CREATETCP);
    IOTJS_ASSERT(jerry_value_is_function(jcreate_tcp));

    jerry_value_t jclient_tcp =
        jerry_call_function(jcreate_tcp, jerry_create_undefined(), NULL, 0);
    IOTJS_ASSERT(!jerry_value_is_error(jclient_tcp));
    IOTJS_ASSERT(jerry_value_is_object(jclient_tcp));

    void* client_handle = NULL;
    bool has_client_handle =
        jerry_get_object_native_pointer(jclient_tcp, &client_handle,
                                        &this_module_native_info);


    if (!has_client_handle || uv_accept(handle, (uv_stream_t*)client_handle)) {
      jerry_release_value(args[0]);
      return;
    }

    args[argc++] = jclient_tcp;
    jerry_release_value(jcreate_tcp);
  }

  iotjs_invoke_callback(jonconnection, jtcp, args, argc);

  jerry_release_value(jonconnection);
  for (size_t i = 0; i < argc; i++) {
    jerry_release_value(args[i]);
  }
}


JS_FUNCTION(tcp_listen) {
  JS_DECLARE_PTR(jthis, uv_tcp_t, tcp_handle);
  DJS_CHECK_ARGS(1, number);

  int backlog = JS_GET_ARG(0, number);
  int err = uv_listen((uv_stream_t*)tcp_handle, backlog, on_connection);

  return jerry_create_number(err);
}


void AfterWrite(uv_write_t* req, int status) {
  iotjs_tcp_report_req_result((uv_req_t*)req, status);
}


JS_FUNCTION(tcp_write) {
  JS_DECLARE_PTR(jthis, uv_stream_t, tcp_handle);
  DJS_CHECK_ARGS(2, object, function);

  const jerry_value_t jbuffer = JS_GET_ARG(0, object);
  iotjs_bufferwrap_t* buffer_wrap = iotjs_bufferwrap_from_jbuffer(jbuffer);
  size_t len = iotjs_bufferwrap_length(buffer_wrap);

  uv_buf_t buf;
  buf.base = buffer_wrap->buffer;
  buf.len = len;

  jerry_value_t arg1 = JS_GET_ARG(1, object);
  uv_req_t* req_write = iotjs_uv_request_create(sizeof(uv_write_t), arg1, 0);

  int err = uv_write((uv_write_t*)req_write, tcp_handle, &buf, 1, AfterWrite);

  if (err) {
    iotjs_uv_request_destroy((uv_req_t*)req_write);
  }

  return jerry_create_number(err);
}


static void on_alloc(uv_handle_t* handle, size_t suggested_size,
                     uv_buf_t* buf) {
  if (suggested_size > IOTJS_MAX_READ_BUFFER_SIZE) {
    suggested_size = IOTJS_MAX_READ_BUFFER_SIZE;
  }

  buf->base = iotjs_buffer_allocate(suggested_size);
  buf->len = suggested_size;
}


static void on_read(uv_stream_t* handle, ssize_t nread, const uv_buf_t* buf) {
  jerry_value_t jtcp = IOTJS_UV_HANDLE_DATA(handle)->jobject;

  // socket object
  jerry_value_t jsocket =
      iotjs_jval_get_property(jtcp, IOTJS_MAGIC_STRING_OWNER);
  IOTJS_ASSERT(jerry_value_is_object(jsocket));

  // onread callback
  jerry_value_t jonread =
      iotjs_jval_get_property(jtcp, IOTJS_MAGIC_STRING_ONREAD);
  IOTJS_ASSERT(jerry_value_is_function(jonread));

  size_t argc = 3;
  jerry_value_t jargs[4] = { jsocket, jerry_create_number(nread),
                             jerry_create_boolean(false), 0 };

  if (nread <= 0) {
    iotjs_buffer_release(buf->base);

    if (nread < 0) {
      if (nread == UV__EOF) {
        jargs[2] = jerry_create_boolean(true);
      }

      iotjs_invoke_callback(jonread, jerry_create_undefined(), jargs, argc);
    }
  } else {
    jerry_value_t jbuffer = iotjs_bufferwrap_create_buffer((size_t)nread);
    iotjs_bufferwrap_t* buffer_wrap = iotjs_bufferwrap_from_jbuffer(jbuffer);

    iotjs_bufferwrap_copy(buffer_wrap, buf->base, (size_t)nread);

    jargs[argc++] = jbuffer;
    iotjs_invoke_callback(jonread, jerry_create_undefined(), jargs, argc);

    iotjs_buffer_release(buf->base);
  }

  for (uint8_t i = 0; i < argc; i++) {
    jerry_release_value(jargs[i]);
  }
  jerry_release_value(jonread);
}


JS_FUNCTION(tcp_read_start) {
  JS_DECLARE_PTR(jthis, uv_stream_t, tcp_handle);

  int err = uv_read_start(tcp_handle, on_alloc, on_read);

  return jerry_create_number(err);
}


static void AfterShutdown(uv_shutdown_t* req, int status) {
  iotjs_tcp_report_req_result((uv_req_t*)req, status);
}


JS_FUNCTION(tcp_shutdown) {
  JS_DECLARE_PTR(jthis, uv_stream_t, tcp_handle);

  DJS_CHECK_ARGS(1, function);

  jerry_value_t arg0 = JS_GET_ARG(0, object);
  uv_shutdown_t* req_shutdown =
      (uv_shutdown_t*)iotjs_uv_request_create(sizeof(uv_shutdown_t), arg0, 0);

  int err = uv_shutdown(req_shutdown, tcp_handle, AfterShutdown);

  if (err) {
    iotjs_uv_request_destroy((uv_req_t*)req_shutdown);
  }

  return jerry_create_number(err);
}


// Enable/Disable keepalive option.
// [0] enable
// [1] delay
JS_FUNCTION(tcp_set_keep_alive) {
  JS_DECLARE_PTR(jthis, uv_tcp_t, tcp_handle);

  DJS_CHECK_ARGS(2, number, number);

  int enable = JS_GET_ARG(0, number);
  unsigned delay = JS_GET_ARG(1, number);

  int err = uv_tcp_keepalive(tcp_handle, enable, delay);

  return jerry_create_number(err);
}

JS_FUNCTION(tcp_err_name) {
  DJS_CHECK_THIS();
  DJS_CHECK_ARGS(1, number);

  int errorcode = JS_GET_ARG(0, number);
  return jerry_create_string_from_utf8(
      (const jerry_char_t*)uv_err_name(errorcode));
}

// used in iotjs_module_udp.cpp
void address_to_js(jerry_value_t obj, const sockaddr* addr) {
  char ip[INET6_ADDRSTRLEN];
  const sockaddr_in* a4;
  const sockaddr_in6* a6;
  int port;

  switch (addr->sa_family) {
    case AF_INET6: {
      a6 = (const sockaddr_in6*)(addr);
      uv_inet_ntop(AF_INET6, &a6->sin6_addr, ip, sizeof ip);
      port = ntohs(a6->sin6_port);
      iotjs_jval_set_property_string_raw(obj, IOTJS_MAGIC_STRING_ADDRESS, ip);
      iotjs_jval_set_property_string_raw(obj, IOTJS_MAGIC_STRING_FAMILY,
                                         IOTJS_MAGIC_STRING_IPV6);
      iotjs_jval_set_property_number(obj, IOTJS_MAGIC_STRING_PORT, port);
      break;
    }

    case AF_INET: {
      a4 = (const sockaddr_in*)(addr);
      uv_inet_ntop(AF_INET, &a4->sin_addr, ip, sizeof ip);
      port = ntohs(a4->sin_port);
      iotjs_jval_set_property_string_raw(obj, IOTJS_MAGIC_STRING_ADDRESS, ip);
      iotjs_jval_set_property_string_raw(obj, IOTJS_MAGIC_STRING_FAMILY,
                                         IOTJS_MAGIC_STRING_IPV4);
      iotjs_jval_set_property_number(obj, IOTJS_MAGIC_STRING_PORT, port);
      break;
    }

    default: {
      iotjs_jval_set_property_string_raw(obj, IOTJS_MAGIC_STRING_ADDRESS, "");
      break;
    }
  }
}


JS_FUNCTION(tcp_get_socket_name) {
  JS_DECLARE_PTR(jthis, uv_tcp_t, tcp_handle);

  DJS_CHECK_ARGS(1, object);

  sockaddr_storage storage;
  int addrlen = sizeof(storage);
  sockaddr* const addr = (sockaddr*)(&storage);
  int err = uv_tcp_getsockname(tcp_handle, addr, &addrlen);
  if (err == 0)
    address_to_js(JS_GET_ARG(0, object), addr);
  return jerry_create_number(err);
}

jerry_value_t iotjs_init_tcp(void) {
  jerry_value_t tcp = jerry_create_external_function(tcp_constructor);

  jerry_value_t prototype = jerry_create_object();

  iotjs_jval_set_property_jval(tcp, IOTJS_MAGIC_STRING_PROTOTYPE, prototype);
  iotjs_jval_set_method(tcp, IOTJS_MAGIC_STRING_ERRNAME, tcp_err_name);

  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_CLOSE, tcp_close);
  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_CONNECT, tcp_connect);
  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_BIND, tcp_bind);
  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_LISTEN, tcp_listen);
  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_WRITE, tcp_write);
  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_READSTART,
                        tcp_read_start);
  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_SHUTDOWN, tcp_shutdown);
  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_SETKEEPALIVE,
                        tcp_set_keep_alive);
  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_GETSOCKNAME,
                        tcp_get_socket_name);

  jerry_release_value(prototype);

  return tcp;
}
