/* Copyright 2016-present Samsung Electronics Co., Ltd. and other contributors
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

#include "iotjs_module_buffer.h"
#include "iotjs_module_tcp.h"
#include "iotjs_uv_handle.h"
#include "iotjs_uv_request.h"


static const jerry_object_native_info_t this_module_native_info = { NULL };


void iotjs_udp_object_init(jerry_value_t judp) {
  uv_handle_t* handle = iotjs_uv_handle_create(sizeof(uv_udp_t), judp,
                                               &this_module_native_info, 0);

  const iotjs_environment_t* env = iotjs_environment_get();
  uv_udp_init(iotjs_environment_loop(env), (uv_udp_t*)handle);
}


JS_FUNCTION(udp_constructor) {
  DJS_CHECK_THIS();

  jerry_value_t judp = JS_GET_THIS();
  iotjs_udp_object_init(judp);

  return jerry_create_undefined();
}


JS_FUNCTION(udp_bind) {
  JS_DECLARE_PTR(jthis, uv_udp_t, udp_handle);
  DJS_CHECK_ARGS(2, string, number);

  iotjs_string_t address = JS_GET_ARG(0, string);
  const int port = JS_GET_ARG(1, number);
  jerry_value_t this_obj = JS_GET_THIS();
  jerry_value_t reuse_addr =
      iotjs_jval_get_property(this_obj, IOTJS_MAGIC_STRING__REUSEADDR);
  IOTJS_ASSERT(jerry_value_is_boolean(reuse_addr) ||
               jerry_value_is_undefined(reuse_addr));

  unsigned int flags = 0;
  if (!jerry_value_is_undefined(reuse_addr)) {
    flags = iotjs_jval_as_boolean(reuse_addr) ? UV_UDP_REUSEADDR : 0;
  }

  char addr[sizeof(sockaddr_in6)];
  int err =
      uv_ip4_addr(iotjs_string_data(&address), port, (sockaddr_in*)(&addr));

  if (err == 0) {
    err = uv_udp_bind(udp_handle, (const sockaddr*)(&addr), flags);
  }

  jerry_release_value(reuse_addr);
  iotjs_string_destroy(&address);

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


static void on_recv(uv_udp_t* handle, ssize_t nread, const uv_buf_t* buf,
                    const struct sockaddr* addr, unsigned int flags) {
  if (nread == 0 && addr == NULL) {
    iotjs_buffer_release(buf->base);
    return;
  }

  // udp handle
  jerry_value_t judp = IOTJS_UV_HANDLE_DATA(handle)->jobject;
  IOTJS_ASSERT(jerry_value_is_object(judp));

  // onmessage callback
  jerry_value_t jonmessage =
      iotjs_jval_get_property(judp, IOTJS_MAGIC_STRING_ONMESSAGE);
  IOTJS_ASSERT(jerry_value_is_function(jonmessage));

  jerry_value_t jargs[4] = { jerry_create_number(nread),
                             jerry_acquire_value(judp), jerry_create_null(),
                             jerry_create_object() };

  if (nread < 0) {
    iotjs_buffer_release(buf->base);
    iotjs_invoke_callback(jonmessage, jerry_create_undefined(), jargs, 2);
    jerry_release_value(jonmessage);

    for (int i = 0; i < 4; i++) {
      jerry_release_value(jargs[i]);
    }
    return;
  }

  jargs[2] = iotjs_bufferwrap_create_buffer((size_t)nread);
  iotjs_bufferwrap_t* buffer_wrap = iotjs_bufferwrap_from_jbuffer(jargs[2]);
  iotjs_bufferwrap_copy(buffer_wrap, buf->base, (size_t)nread);
  address_to_js(jargs[3], addr);

  iotjs_invoke_callback(jonmessage, jerry_create_undefined(), jargs, 4);

  jerry_release_value(jonmessage);
  iotjs_buffer_release(buf->base);

  for (int i = 0; i < 4; i++) {
    jerry_release_value(jargs[i]);
  }
}


JS_FUNCTION(udp_recv_start) {
  JS_DECLARE_PTR(jthis, uv_udp_t, udp_handle);

  int err = uv_udp_recv_start(udp_handle, on_alloc, on_recv);

  // UV_EALREADY means that the socket is already bound but that's okay
  if (err == UV_EALREADY)
    err = 0;

  return jerry_create_number(err);
}


JS_FUNCTION(udp_recv_stop) {
  JS_DECLARE_PTR(jthis, uv_udp_t, udp_handle);

  int r = uv_udp_recv_stop(udp_handle);

  return jerry_create_number(r);
}


static void on_send(uv_udp_send_t* req, int status) {
  IOTJS_ASSERT(req != NULL);

  // Take callback function object.
  jerry_value_t jcallback = *IOTJS_UV_REQUEST_JSCALLBACK(req);

  if (jerry_value_is_function(jcallback)) {
    size_t msg_size = *((size_t*)IOTJS_UV_REQUEST_EXTRA_DATA(req));
    jerry_value_t jargs[2] = { jerry_create_number(status),
                               jerry_create_number(msg_size) };

    iotjs_invoke_callback(jcallback, jerry_create_undefined(), jargs, 2);
    jerry_release_value(jargs[0]);
    jerry_release_value(jargs[1]);
  }

  iotjs_uv_request_destroy((uv_req_t*)req);
}


// Send messages using the socket.
// [0] buffer
// [1] port
// [2] ip
// [3] callback function
JS_FUNCTION(udp_send) {
  JS_DECLARE_PTR(jthis, uv_udp_t, udp_handle);
  DJS_CHECK_ARGS(3, object, number, string);

  if (!jerry_value_is_undefined(jargv[3]) &&
      !jerry_value_is_function(jargv[3])) {
    return JS_CREATE_ERROR(TYPE, "Invalid callback given");
  }

  const jerry_value_t jbuffer = JS_GET_ARG(0, object);
  const unsigned short port = JS_GET_ARG(1, number);
  iotjs_string_t address = JS_GET_ARG(2, string);
  jerry_value_t jcallback = JS_GET_ARG(3, object);

  iotjs_bufferwrap_t* buffer_wrap = iotjs_jbuffer_get_bufferwrap_ptr(jbuffer);
  if (buffer_wrap == NULL) {
    return JS_CREATE_ERROR(TYPE, "Invalid buffer given");
  }

  size_t len = iotjs_bufferwrap_length(buffer_wrap);

  uv_req_t* req_send =
      iotjs_uv_request_create(sizeof(uv_udp_send_t), jcallback, sizeof(len));
  *((size_t*)IOTJS_UV_REQUEST_EXTRA_DATA(req_send)) = len;

  uv_buf_t buf;
  buf.base = buffer_wrap->buffer;
  buf.len = len;

  char addr[sizeof(sockaddr_in6)];
  int err =
      uv_ip4_addr(iotjs_string_data(&address), port, (sockaddr_in*)(&addr));

  if (err == 0) {
    err = uv_udp_send((uv_udp_send_t*)req_send, udp_handle, &buf, 1,
                      (const sockaddr*)(&addr), on_send);
  }

  if (err) {
    iotjs_uv_request_destroy(req_send);
  }

  iotjs_string_destroy(&address);

  return jerry_create_number(err);
}


// Close socket
JS_FUNCTION(udp_close) {
  JS_DECLARE_PTR(jthis, uv_handle_t, uv_handle);

  iotjs_uv_handle_close(uv_handle, NULL);

  return jerry_create_undefined();
}


JS_FUNCTION(udp_get_socket_name) {
  JS_DECLARE_PTR(jthis, uv_udp_t, udp_handle);
  DJS_CHECK_ARGS(1, object);

  sockaddr_storage storage;
  int addrlen = sizeof(storage);
  sockaddr* const addr = (sockaddr*)(&storage);
  int err = uv_udp_getsockname(udp_handle, addr, &addrlen);
  if (err == 0)
    address_to_js(JS_GET_ARG(0, object), addr);
  return jerry_create_number(err);
}


// The order of these config types must match the order
// in the dgram js module.
enum config_type { BROADCAST, TTL, MULTICASTTTL, MULTICASTLOOPBACK };

JS_FUNCTION(udp_configure) {
  JS_DECLARE_PTR(jthis, uv_udp_t, udp_handle);
  DJS_CHECK_ARGS(2, number, number);

  jerry_value_t ret_value = jerry_create_null();

#if !defined(__NUTTX__)
  enum config_type type = JS_GET_ARG(0, number);
  int flag = JS_GET_ARG(1, number);
  int (*fn)(uv_udp_t*, int) = NULL;

  switch (type) {
    case BROADCAST: {
      fn = &uv_udp_set_broadcast;
      break;
    }
    case TTL: {
      fn = &uv_udp_set_ttl;
      break;
    }
    case MULTICASTTTL: {
      fn = &uv_udp_set_multicast_ttl;
      break;
    }
    case MULTICASTLOOPBACK: {
      fn = &uv_udp_set_multicast_loop;
      break;
    }
    default: {
      IOTJS_ASSERT(!"Unknown config type");
      return jerry_create_null();
    }
  }
  ret_value = jerry_create_number(fn(udp_handle, flag));
#else
  IOTJS_ASSERT(!"Not implemented");
#endif
  return ret_value;
}


static jerry_value_t set_membership(const jerry_value_t jthis,
                                    const jerry_value_t* jargv,
                                    const jerry_length_t jargc,
                                    uv_membership membership) {
#if !defined(__NUTTX__)
  JS_DECLARE_PTR(jthis, uv_udp_t, udp_handle);
  DJS_CHECK_ARGS(1, string);

  iotjs_string_t address = JS_GET_ARG(0, string);
  bool is_undefined_or_null =
      jerry_value_is_undefined(jargv[1]) || jerry_value_is_null(jargv[1]);
  iotjs_string_t iface;

  const char* iface_cstr;
  if (is_undefined_or_null) {
    iface_cstr = NULL;
  } else {
    iface = iotjs_jval_as_string(jargv[1]);
    iface_cstr = iotjs_string_data(&iface);
  }

  int err = uv_udp_set_membership(udp_handle, iotjs_string_data(&address),
                                  iface_cstr, membership);

  if (!is_undefined_or_null)
    iotjs_string_destroy(&iface);

  iotjs_string_destroy(&address);
  return jerry_create_number(err);
#else
  IOTJS_ASSERT(!"Not implemented");

  return jerry_create_null();
#endif
}


JS_FUNCTION(udp_add_membership) {
  return set_membership(jthis, jargv, jargc, UV_JOIN_GROUP);
}


JS_FUNCTION(udp_drop_membership) {
  return set_membership(jthis, jargv, jargc, UV_LEAVE_GROUP);
}


JS_FUNCTION(udp_ref) {
  IOTJS_ASSERT(!"Not implemented");

  return jerry_create_null();
}


JS_FUNCTION(udp_unref) {
  IOTJS_ASSERT(!"Not implemented");

  return jerry_create_null();
}


jerry_value_t iotjs_init_udp(void) {
  jerry_value_t udp = jerry_create_external_function(udp_constructor);

  jerry_value_t prototype = jerry_create_object();
  iotjs_jval_set_property_jval(udp, IOTJS_MAGIC_STRING_PROTOTYPE, prototype);

  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_BIND, udp_bind);
  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_RECVSTART,
                        udp_recv_start);
  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_RECVSTOP, udp_recv_stop);
  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_SEND, udp_send);
  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_CLOSE, udp_close);
  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_GETSOCKNAME,
                        udp_get_socket_name);
  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_CONFIGURE, udp_configure);
  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_ADDMEMBERSHIP,
                        udp_add_membership);
  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_DROPMEMBERSHIP,
                        udp_drop_membership);
  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_REF, udp_ref);
  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_UNREF, udp_unref);

  jerry_release_value(prototype);

  return udp;
}
