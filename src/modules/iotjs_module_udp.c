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

#include "iotjs_module_udp.h"

#include "iotjs_handlewrap.h"
#include "iotjs_module_buffer.h"
#include "iotjs_module_tcp.h"
#include "iotjs_reqwrap.h"


IOTJS_DEFINE_NATIVE_HANDLE_INFO_THIS_MODULE(udpwrap);


iotjs_udpwrap_t* iotjs_udpwrap_create(jerry_value_t judp) {
  iotjs_udpwrap_t* udpwrap = IOTJS_ALLOC(iotjs_udpwrap_t);

  iotjs_handlewrap_initialize(&udpwrap->handlewrap, judp,
                              (uv_handle_t*)(&udpwrap->handle),
                              &this_module_native_info);

  const iotjs_environment_t* env = iotjs_environment_get();
  uv_udp_init(iotjs_environment_loop(env), &udpwrap->handle);

  return udpwrap;
}


static void iotjs_udpwrap_destroy(iotjs_udpwrap_t* udpwrap) {
  iotjs_handlewrap_destroy(&udpwrap->handlewrap);
  IOTJS_RELEASE(udpwrap);
}


iotjs_udpwrap_t* iotjs_udpwrap_from_handle(uv_udp_t* udp_handle) {
  uv_handle_t* handle = (uv_handle_t*)(udp_handle);
  iotjs_handlewrap_t* handlewrap = iotjs_handlewrap_from_handle(handle);
  iotjs_udpwrap_t* udpwrap = (iotjs_udpwrap_t*)handlewrap;
  IOTJS_ASSERT(iotjs_udpwrap_udp_handle(udpwrap) == udp_handle);
  return udpwrap;
}


iotjs_udpwrap_t* iotjs_udpwrap_from_jobject(jerry_value_t judp) {
  iotjs_handlewrap_t* handlewrap = iotjs_handlewrap_from_jobject(judp);
  return (iotjs_udpwrap_t*)handlewrap;
}


uv_udp_t* iotjs_udpwrap_udp_handle(iotjs_udpwrap_t* udpwrap) {
  uv_handle_t* handle = iotjs_handlewrap_get_uv_handle(&udpwrap->handlewrap);
  return (uv_udp_t*)handle;
}


iotjs_send_reqwrap_t* iotjs_send_reqwrap_create(jerry_value_t jcallback,
                                                const size_t msg_size) {
  iotjs_send_reqwrap_t* send_reqwrap = IOTJS_ALLOC(iotjs_send_reqwrap_t);

  iotjs_reqwrap_initialize(&send_reqwrap->reqwrap, jcallback,
                           (uv_req_t*)&send_reqwrap->req);
  send_reqwrap->msg_size = msg_size;

  return send_reqwrap;
}


static void iotjs_send_reqwrap_destroy(iotjs_send_reqwrap_t* send_reqwrap) {
  iotjs_reqwrap_destroy(&send_reqwrap->reqwrap);
  IOTJS_RELEASE(send_reqwrap);
}


JS_FUNCTION(UDP) {
  DJS_CHECK_THIS();

  jerry_value_t judp = JS_GET_THIS();
  iotjs_udpwrap_create(judp);

  return jerry_create_undefined();
}


JS_FUNCTION(Bind) {
  JS_DECLARE_THIS_PTR(udpwrap, udp_wrap);
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
    err = uv_udp_bind(iotjs_udpwrap_udp_handle(udp_wrap),
                      (const sockaddr*)(&addr), flags);
  }

  jerry_release_value(reuse_addr);
  iotjs_string_destroy(&address);

  return jerry_create_number(err);
}


static void OnAlloc(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
  if (suggested_size > IOTJS_MAX_READ_BUFFER_SIZE) {
    suggested_size = IOTJS_MAX_READ_BUFFER_SIZE;
  }

  buf->base = iotjs_buffer_allocate(suggested_size);
  buf->len = suggested_size;
}


static void OnRecv(uv_udp_t* handle, ssize_t nread, const uv_buf_t* buf,
                   const struct sockaddr* addr, unsigned int flags) {
  if (nread == 0 && addr == NULL) {
    iotjs_buffer_release(buf->base);
    return;
  }

  iotjs_udpwrap_t* udp_wrap = iotjs_udpwrap_from_handle(handle);

  // udp handle
  jerry_value_t judp = iotjs_handlewrap_jobject(&udp_wrap->handlewrap);
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
  AddressToJS(jargs[3], addr);

  iotjs_invoke_callback(jonmessage, jerry_create_undefined(), jargs, 4);

  jerry_release_value(jonmessage);
  iotjs_buffer_release(buf->base);

  for (int i = 0; i < 4; i++) {
    jerry_release_value(jargs[i]);
  }
}


JS_FUNCTION(RecvStart) {
  JS_DECLARE_THIS_PTR(udpwrap, udp_wrap);

  int err =
      uv_udp_recv_start(iotjs_udpwrap_udp_handle(udp_wrap), OnAlloc, OnRecv);

  // UV_EALREADY means that the socket is already bound but that's okay
  if (err == UV_EALREADY)
    err = 0;

  return jerry_create_number(err);
}


JS_FUNCTION(RecvStop) {
  JS_DECLARE_THIS_PTR(udpwrap, udp_wrap);

  int r = uv_udp_recv_stop(iotjs_udpwrap_udp_handle(udp_wrap));

  return jerry_create_number(r);
}


static void OnSend(uv_udp_send_t* req, int status) {
  iotjs_send_reqwrap_t* req_wrap = (iotjs_send_reqwrap_t*)(req->data);
  IOTJS_ASSERT(req_wrap != NULL);

  // Take callback function object.
  jerry_value_t jcallback = iotjs_reqwrap_jcallback(&req_wrap->reqwrap);

  if (jerry_value_is_function(jcallback)) {
    // Take callback function object.

    jerry_value_t jargs[2] = { jerry_create_number(status),
                               jerry_create_number(req_wrap->msg_size) };

    iotjs_invoke_callback(jcallback, jerry_create_undefined(), jargs, 2);
    jerry_release_value(jargs[0]);
    jerry_release_value(jargs[1]);
  }

  iotjs_send_reqwrap_destroy(req_wrap);
}


// Send messages using the socket.
// [0] buffer
// [1] port
// [2] ip
// [3] callback function
JS_FUNCTION(Send) {
  JS_DECLARE_THIS_PTR(udpwrap, udp_wrap);
  DJS_CHECK_ARGS(3, object, number, string);
  IOTJS_ASSERT(jerry_value_is_function(jargv[3]) ||
               jerry_value_is_undefined(jargv[3]));

  const jerry_value_t jbuffer = JS_GET_ARG(0, object);
  const unsigned short port = JS_GET_ARG(1, number);
  iotjs_string_t address = JS_GET_ARG(2, string);
  jerry_value_t jcallback = JS_GET_ARG(3, object);

  iotjs_bufferwrap_t* buffer_wrap = iotjs_bufferwrap_from_jbuffer(jbuffer);
  size_t len = iotjs_bufferwrap_length(buffer_wrap);

  iotjs_send_reqwrap_t* req_wrap = iotjs_send_reqwrap_create(jcallback, len);

  uv_buf_t buf;
  buf.base = buffer_wrap->buffer;
  buf.len = len;

  char addr[sizeof(sockaddr_in6)];
  int err =
      uv_ip4_addr(iotjs_string_data(&address), port, (sockaddr_in*)(&addr));

  if (err == 0) {
    err = uv_udp_send(&req_wrap->req, iotjs_udpwrap_udp_handle(udp_wrap), &buf,
                      1, (const sockaddr*)(&addr), OnSend);
  }

  if (err) {
    iotjs_send_reqwrap_destroy(req_wrap);
  }

  iotjs_string_destroy(&address);

  return jerry_create_number(err);
}


// Close socket
JS_FUNCTION(Close) {
  JS_DECLARE_THIS_PTR(handlewrap, wrap);

  iotjs_handlewrap_close(wrap, NULL);

  return jerry_create_undefined();
}


JS_FUNCTION(GetSockeName) {
  DJS_CHECK_ARGS(1, object);
  iotjs_udpwrap_t* wrap = iotjs_udpwrap_from_jobject(JS_GET_THIS());
  IOTJS_ASSERT(wrap != NULL);

  sockaddr_storage storage;
  int addrlen = sizeof(storage);
  sockaddr* const addr = (sockaddr*)(&storage);
  int err = uv_udp_getsockname(iotjs_udpwrap_udp_handle(wrap), addr, &addrlen);
  if (err == 0)
    AddressToJS(JS_GET_ARG(0, object), addr);
  return jerry_create_number(err);
}


#define IOTJS_UV_SET_SOCKOPT(fn)                          \
  JS_DECLARE_THIS_PTR(udpwrap, udp_wrap);                 \
  DJS_CHECK_ARGS(1, number);                              \
                                                          \
  int flag = JS_GET_ARG(0, number);                       \
  int err = fn(iotjs_udpwrap_udp_handle(udp_wrap), flag); \
                                                          \
  return jerry_create_number(err);


JS_FUNCTION(SetBroadcast) {
#if !defined(__NUTTX__)
  IOTJS_UV_SET_SOCKOPT(uv_udp_set_broadcast);
#else
  IOTJS_ASSERT(!"Not implemented");

  return jerry_create_null();
#endif
}


JS_FUNCTION(SetTTL) {
#if !defined(__NUTTX__)
  IOTJS_UV_SET_SOCKOPT(uv_udp_set_ttl);
#else
  IOTJS_ASSERT(!"Not implemented");

  return jerry_create_null();
#endif
}


JS_FUNCTION(SetMulticastTTL) {
#if !defined(__NUTTX__)
  IOTJS_UV_SET_SOCKOPT(uv_udp_set_multicast_ttl);
#else
  IOTJS_ASSERT(!"Not implemented");

  return jerry_create_null();
#endif
}


JS_FUNCTION(SetMulticastLoopback) {
#if !defined(__NUTTX__)
  IOTJS_UV_SET_SOCKOPT(uv_udp_set_multicast_loop);
#else
  IOTJS_ASSERT(!"Not implemented");

  return jerry_create_null();
#endif
}

#undef IOTJS_UV_SET_SOCKOPT


static jerry_value_t SetMembership(const jerry_value_t jthis,
                                   const jerry_value_t* jargv,
                                   const jerry_length_t jargc,
                                   uv_membership membership) {
#if !defined(__NUTTX__)
  JS_DECLARE_THIS_PTR(udpwrap, udp_wrap);
  DJS_CHECK_ARGS(1, string);

  iotjs_string_t address = JS_GET_ARG(0, string);
  bool isUndefinedOrNull =
      jerry_value_is_undefined(jargv[1]) || jerry_value_is_null(jargv[1]);
  iotjs_string_t iface;

  const char* iface_cstr;
  if (isUndefinedOrNull) {
    iface_cstr = NULL;
  } else {
    iface = iotjs_jval_as_string(jargv[1]);
    iface_cstr = iotjs_string_data(&iface);
  }

  int err = uv_udp_set_membership(iotjs_udpwrap_udp_handle(udp_wrap),
                                  iotjs_string_data(&address), iface_cstr,
                                  membership);

  if (!isUndefinedOrNull)
    iotjs_string_destroy(&iface);

  iotjs_string_destroy(&address);
  return jerry_create_number(err);
#else
  IOTJS_ASSERT(!"Not implemented");

  return jerry_create_null();
#endif
}


JS_FUNCTION(AddMembership) {
  return SetMembership(jthis, jargv, jargc, UV_JOIN_GROUP);
}


JS_FUNCTION(DropMembership) {
  return SetMembership(jthis, jargv, jargc, UV_LEAVE_GROUP);
}


JS_FUNCTION(Ref) {
  IOTJS_ASSERT(!"Not implemented");

  return jerry_create_null();
}


JS_FUNCTION(Unref) {
  IOTJS_ASSERT(!"Not implemented");

  return jerry_create_null();
}


jerry_value_t InitUdp() {
  jerry_value_t udp = jerry_create_external_function(UDP);

  jerry_value_t prototype = jerry_create_object();
  iotjs_jval_set_property_jval(udp, IOTJS_MAGIC_STRING_PROTOTYPE, prototype);

  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_BIND, Bind);
  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_RECVSTART, RecvStart);
  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_RECVSTOP, RecvStop);
  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_SEND, Send);
  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_CLOSE, Close);
  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_GETSOCKNAME,
                        GetSockeName);
  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_SETBROADCAST,
                        SetBroadcast);
  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_SETTTL, SetTTL);
  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_SETMULTICASTTTL,
                        SetMulticastTTL);
  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_SETMULTICASTLOOPBACK,
                        SetMulticastLoopback);
  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_ADDMEMBERSHIP,
                        AddMembership);
  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_DROPMEMBERSHIP,
                        DropMembership);
  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_REF, Ref);
  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_UNREF, Unref);

  jerry_release_value(prototype);

  return udp;
}
