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


static void iotjs_udpwrap_destroy(iotjs_udpwrap_t* udpwrap);


iotjs_udpwrap_t* iotjs_udpwrap_create(const iotjs_jval_t* judp) {
  iotjs_udpwrap_t* udpwrap = IOTJS_ALLOC(iotjs_udpwrap_t);
  IOTJS_VALIDATED_STRUCT_CONSTRUCTOR(iotjs_udpwrap_t, udpwrap);

  iotjs_handlewrap_initialize(&_this->handlewrap, judp,
                              (uv_handle_t*)(&_this->handle),
                              (JFreeHandlerType)iotjs_udpwrap_destroy);

  const iotjs_environment_t* env = iotjs_environment_get();
  uv_udp_init(iotjs_environment_loop(env), &_this->handle);

  return udpwrap;
}


static void iotjs_udpwrap_destroy(iotjs_udpwrap_t* udpwrap) {
  IOTJS_VALIDATED_STRUCT_DESTRUCTOR(iotjs_udpwrap_t, udpwrap);
  iotjs_handlewrap_destroy(&_this->handlewrap);
  IOTJS_RELEASE(udpwrap);
}


iotjs_udpwrap_t* iotjs_udpwrap_from_handle(uv_udp_t* udp_handle) {
  uv_handle_t* handle = (uv_handle_t*)(udp_handle);
  iotjs_handlewrap_t* handlewrap = iotjs_handlewrap_from_handle(handle);
  iotjs_udpwrap_t* udpwrap = (iotjs_udpwrap_t*)handlewrap;
  IOTJS_ASSERT(iotjs_udpwrap_udp_handle(udpwrap) == udp_handle);
  return udpwrap;
}


iotjs_udpwrap_t* iotjs_udpwrap_from_jobject(const iotjs_jval_t* judp) {
  iotjs_handlewrap_t* handlewrap = iotjs_handlewrap_from_jobject(judp);
  return (iotjs_udpwrap_t*)handlewrap;
}


uv_udp_t* iotjs_udpwrap_udp_handle(iotjs_udpwrap_t* udpwrap) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_udpwrap_t, udpwrap);
  uv_handle_t* handle = iotjs_handlewrap_get_uv_handle(&_this->handlewrap);
  return (uv_udp_t*)handle;
}


iotjs_jval_t* iotjs_udpwrap_jobject(iotjs_udpwrap_t* udpwrap) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_udpwrap_t, udpwrap);
  return iotjs_handlewrap_jobject(&_this->handlewrap);
}


#define THIS iotjs_send_reqwrap_t* send_reqwrap

iotjs_send_reqwrap_t* iotjs_send_reqwrap_create(const iotjs_jval_t* jcallback,
                                                const size_t msg_size) {
  iotjs_send_reqwrap_t* send_reqwrap = IOTJS_ALLOC(iotjs_send_reqwrap_t);
  IOTJS_VALIDATED_STRUCT_CONSTRUCTOR(iotjs_send_reqwrap_t, send_reqwrap);

  iotjs_reqwrap_initialize(&_this->reqwrap, jcallback, (uv_req_t*)&_this->req);
  _this->msg_size = msg_size;

  return send_reqwrap;
}


static void iotjs_send_reqwrap_destroy(THIS) {
  IOTJS_VALIDATED_STRUCT_DESTRUCTOR(iotjs_send_reqwrap_t, send_reqwrap);
  iotjs_reqwrap_destroy(&_this->reqwrap);
  IOTJS_RELEASE(send_reqwrap);
}


void iotjs_send_reqwrap_dispatched(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_send_reqwrap_t, send_reqwrap);
  iotjs_send_reqwrap_destroy(send_reqwrap);
}


uv_udp_send_t* iotjs_send_reqwrap_req(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_send_reqwrap_t, send_reqwrap);
  return &_this->req;
}


const iotjs_jval_t* iotjs_send_reqwrap_jcallback(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_send_reqwrap_t, send_reqwrap);
  return iotjs_reqwrap_jcallback(&_this->reqwrap);
}


size_t iotjs_send_reqwrap_msg_size(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_send_reqwrap_t, send_reqwrap);
  return _this->msg_size;
}

#undef THIS


JHANDLER_FUNCTION(UDP) {
  JHANDLER_CHECK_THIS(object);
  JHANDLER_CHECK_ARGS(0);

  const iotjs_jval_t* judp = JHANDLER_GET_THIS(object);
  iotjs_udpwrap_t* udp_wrap = iotjs_udpwrap_create(judp);
}


JHANDLER_FUNCTION(Bind) {
  JHANDLER_CHECK_THIS(object);
  JHANDLER_CHECK_ARGS_2(string, number);

  const iotjs_jval_t* judp = JHANDLER_GET_THIS(object);
  iotjs_udpwrap_t* udp_wrap = iotjs_udpwrap_from_jobject(judp);

  iotjs_string_t address = JHANDLER_GET_ARG(0, string);
  const int port = JHANDLER_GET_ARG(1, number);
  const iotjs_jval_t* this_obj = JHANDLER_GET_THIS(object);
  iotjs_jval_t reuse_addr = iotjs_jval_get_property(this_obj, "_reuseAddr");
  IOTJS_ASSERT(iotjs_jval_is_boolean(&reuse_addr) ||
               iotjs_jval_is_undefined(&reuse_addr));

  int flags = false;
  if (!iotjs_jval_is_undefined(&reuse_addr)) {
    flags = iotjs_jval_as_boolean(&reuse_addr) ? UV_UDP_REUSEADDR : 0;
  }

  char addr[sizeof(sockaddr_in6)];
  int err =
      uv_ip4_addr(iotjs_string_data(&address), port, (sockaddr_in*)(&addr));

  if (err == 0) {
    err = uv_udp_bind(iotjs_udpwrap_udp_handle(udp_wrap),
                      (const sockaddr*)(&addr), flags);
  }

  iotjs_jhandler_return_number(jhandler, err);

  iotjs_jval_destroy(&reuse_addr);
  iotjs_string_destroy(&address);
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
    if (buf->base != NULL)
      iotjs_buffer_release(buf->base);
    return;
  }

  iotjs_udpwrap_t* udp_wrap = iotjs_udpwrap_from_handle(handle);

  // udp handle
  const iotjs_jval_t* judp = iotjs_udpwrap_jobject(udp_wrap);
  IOTJS_ASSERT(iotjs_jval_is_object(judp));

  // onmessage callback
  iotjs_jval_t jonmessage = iotjs_jval_get_property(judp, "onmessage");
  IOTJS_ASSERT(iotjs_jval_is_function(&jonmessage));

  iotjs_jargs_t jargs = iotjs_jargs_create(4);
  iotjs_jargs_append_number(&jargs, nread);
  iotjs_jargs_append_jval(&jargs, judp);

  if (nread < 0) {
    if (buf->base != NULL)
      iotjs_buffer_release(buf->base);
    iotjs_make_callback(&jonmessage, iotjs_jval_get_undefined(), &jargs);
    iotjs_jval_destroy(&jonmessage);
    iotjs_jargs_destroy(&jargs);
    return;
  }

  iotjs_jval_t jbuffer = iotjs_bufferwrap_create_buffer(nread);
  iotjs_bufferwrap_t* buffer_wrap = iotjs_bufferwrap_from_jbuffer(&jbuffer);

  iotjs_bufferwrap_copy(buffer_wrap, buf->base, nread);

  iotjs_jargs_append_jval(&jargs, &jbuffer);

  iotjs_jval_t rinfo = iotjs_jval_create_object();
  AddressToJS(&rinfo, addr);
  iotjs_jargs_append_jval(&jargs, &rinfo);

  iotjs_make_callback(&jonmessage, iotjs_jval_get_undefined(), &jargs);

  iotjs_jval_destroy(&rinfo);
  iotjs_jval_destroy(&jbuffer);
  iotjs_jval_destroy(&jonmessage);
  iotjs_buffer_release(buf->base);
  iotjs_jargs_destroy(&jargs);
}


JHANDLER_FUNCTION(RecvStart) {
  JHANDLER_CHECK_THIS(object);
  JHANDLER_CHECK_ARGS(0);

  const iotjs_jval_t* judp = JHANDLER_GET_THIS(object);
  iotjs_udpwrap_t* udp_wrap = iotjs_udpwrap_from_jobject(judp);

  int err =
      uv_udp_recv_start(iotjs_udpwrap_udp_handle(udp_wrap), OnAlloc, OnRecv);

  // UV_EALREADY means that the socket is already bound but that's okay
  if (err == UV_EALREADY)
    err = 0;

  iotjs_jhandler_return_number(jhandler, err);
}


JHANDLER_FUNCTION(RecvStop) {
  JHANDLER_CHECK_THIS(object);
  JHANDLER_CHECK_ARGS(0);

  const iotjs_jval_t* judp = JHANDLER_GET_THIS(object);
  iotjs_udpwrap_t* udp_wrap = iotjs_udpwrap_from_jobject(judp);

  int r = uv_udp_recv_stop(iotjs_udpwrap_udp_handle(udp_wrap));

  iotjs_jhandler_return_number(jhandler, r);
}


static void OnSend(uv_udp_send_t* req, int status) {
  iotjs_send_reqwrap_t* req_wrap = (iotjs_send_reqwrap_t*)(req->data);
  IOTJS_ASSERT(req_wrap != NULL);

  // Take callback function object.
  const iotjs_jval_t* jcallback = iotjs_send_reqwrap_jcallback(req_wrap);

  if (iotjs_jval_is_function(jcallback)) {
    // Take callback function object.

    iotjs_jargs_t jargs = iotjs_jargs_create(2);
    iotjs_jargs_append_number(&jargs, status);
    iotjs_jargs_append_number(&jargs, iotjs_send_reqwrap_msg_size(req_wrap));

    iotjs_make_callback(jcallback, iotjs_jval_get_undefined(), &jargs);
    iotjs_jargs_destroy(&jargs);
  }

  iotjs_send_reqwrap_dispatched(req_wrap);
}


// Send messages using the socket.
// [0] buffer
// [1] port
// [2] ip
// [3] callback function
JHANDLER_FUNCTION(Send) {
  JHANDLER_CHECK_THIS(object);
  JHANDLER_CHECK_ARGS_3(object, number, string);
  IOTJS_ASSERT(iotjs_jval_is_function(iotjs_jhandler_get_arg(jhandler, 3)) ||
               iotjs_jval_is_undefined(iotjs_jhandler_get_arg(jhandler, 3)));

  const iotjs_jval_t* judp = JHANDLER_GET_THIS(object);
  iotjs_udpwrap_t* udp_wrap = iotjs_udpwrap_from_jobject(judp);

  const iotjs_jval_t* jbuffer = JHANDLER_GET_ARG(0, object);
  const unsigned short port = JHANDLER_GET_ARG(1, number);
  iotjs_string_t address = JHANDLER_GET_ARG(2, string);
  const iotjs_jval_t* jcallback = JHANDLER_GET_ARG(3, object);

  iotjs_bufferwrap_t* buffer_wrap = iotjs_bufferwrap_from_jbuffer(jbuffer);
  char* buffer = iotjs_bufferwrap_buffer(buffer_wrap);
  int len = iotjs_bufferwrap_length(buffer_wrap);

  iotjs_send_reqwrap_t* req_wrap = iotjs_send_reqwrap_create(jcallback, len);

  uv_buf_t buf;
  buf.base = buffer;
  buf.len = len;

  char addr[sizeof(sockaddr_in6)];
  int err =
      uv_ip4_addr(iotjs_string_data(&address), port, (sockaddr_in*)(&addr));

  if (err == 0) {
    err = uv_udp_send(iotjs_send_reqwrap_req(req_wrap),
                      iotjs_udpwrap_udp_handle(udp_wrap), &buf, 1,
                      (const sockaddr*)(&addr), OnSend);
  }

  if (err) {
    iotjs_send_reqwrap_dispatched(req_wrap);
  }

  iotjs_jhandler_return_number(jhandler, err);

  iotjs_string_destroy(&address);
}


// Close socket
JHANDLER_FUNCTION(Close) {
  JHANDLER_CHECK_THIS(object);
  JHANDLER_CHECK_ARGS(0);

  const iotjs_jval_t* judp = JHANDLER_GET_THIS(object);
  iotjs_handlewrap_t* wrap = iotjs_handlewrap_from_jobject(judp);

  iotjs_handlewrap_close(wrap, NULL);
}


GetSockNameFunction(udpwrap, udp_handle, uv_udp_getsockname);


JHANDLER_FUNCTION(GetSockeName) {
  DoGetSockName(jhandler);
}


#define IOTJS_UV_SET_SOCKOPT(fn)                                \
  JHANDLER_CHECK_THIS(object);                                  \
  JHANDLER_CHECK_ARGS_1(number);                                \
                                                                \
  const iotjs_jval_t* judp = JHANDLER_GET_THIS(object);         \
  iotjs_udpwrap_t* udp_wrap = iotjs_udpwrap_from_jobject(judp); \
                                                                \
  int flag = JHANDLER_GET_ARG(0, number);                       \
  int err = fn(iotjs_udpwrap_udp_handle(udp_wrap), flag);       \
                                                                \
  iotjs_jhandler_return_number(jhandler, err);


JHANDLER_FUNCTION(SetBroadcast) {
#if !defined(__NUTTX__)
  IOTJS_UV_SET_SOCKOPT(uv_udp_set_broadcast);
#else
  IOTJS_ASSERT(!"Not implemented");

  iotjs_jhandler_return_null(jhandler);
#endif
}


JHANDLER_FUNCTION(SetTTL) {
#if !defined(__NUTTX__)
  IOTJS_UV_SET_SOCKOPT(uv_udp_set_ttl);
#else
  IOTJS_ASSERT(!"Not implemented");

  iotjs_jhandler_return_null(jhandler);
#endif
}


JHANDLER_FUNCTION(SetMulticastTTL) {
#if !defined(__NUTTX__)
  IOTJS_UV_SET_SOCKOPT(uv_udp_set_multicast_ttl);
#else
  IOTJS_ASSERT(!"Not implemented");

  iotjs_jhandler_return_null(jhandler);
#endif
}


JHANDLER_FUNCTION(SetMulticastLoopback) {
#if !defined(__NUTTX__)
  IOTJS_UV_SET_SOCKOPT(uv_udp_set_multicast_loop);
#else
  IOTJS_ASSERT(!"Not implemented");

  iotjs_jhandler_return_null(jhandler);
#endif
}

#undef IOTJS_UV_SET_SOCKOPT


void SetMembership(iotjs_jhandler_t* jhandler, uv_membership membership) {
#if !defined(__NUTTX__)
  JHANDLER_CHECK_THIS(object);
  JHANDLER_CHECK_ARGS_1(string);

  const iotjs_jval_t* judp = JHANDLER_GET_THIS(object);
  iotjs_udpwrap_t* udp_wrap = iotjs_udpwrap_from_jobject(judp);

  iotjs_string_t address = JHANDLER_GET_ARG(0, string);
  const iotjs_jval_t* arg1 = iotjs_jhandler_get_arg(jhandler, 1);
  bool isUndefinedOrNull =
      iotjs_jval_is_undefined(arg1) || iotjs_jval_is_null(arg1);
  iotjs_string_t iface;

  const char* iface_cstr;
  if (isUndefinedOrNull) {
    iface_cstr = NULL;
  } else {
    iface = iotjs_jval_as_string(arg1);
    iface_cstr = iotjs_string_data(&iface);
  }

  int err = uv_udp_set_membership(iotjs_udpwrap_udp_handle(udp_wrap),
                                  iotjs_string_data(&address), iface_cstr,
                                  membership);

  iotjs_jhandler_return_number(jhandler, err);

  iotjs_string_destroy(&address);
  if (!isUndefinedOrNull)
    iotjs_string_destroy(&iface);
#else
  IOTJS_ASSERT(!"Not implemented");

  iotjs_jhandler_return_null(jhandler);
#endif
}


JHANDLER_FUNCTION(AddMembership) {
  SetMembership(jhandler, UV_JOIN_GROUP);
}


JHANDLER_FUNCTION(DropMembership) {
  SetMembership(jhandler, UV_LEAVE_GROUP);
}


JHANDLER_FUNCTION(Ref) {
  IOTJS_ASSERT(!"Not implemented");

  iotjs_jhandler_return_null(jhandler);
}


JHANDLER_FUNCTION(Unref) {
  IOTJS_ASSERT(!"Not implemented");

  iotjs_jhandler_return_null(jhandler);
}


iotjs_jval_t InitUdp() {
  iotjs_jval_t udp = iotjs_jval_create_function(UDP);

  iotjs_jval_t prototype = iotjs_jval_create_object();
  iotjs_jval_set_property_jval(&udp, "prototype", &prototype);

  iotjs_jval_set_method(&prototype, "bind", Bind);
  iotjs_jval_set_method(&prototype, "recvStart", RecvStart);
  iotjs_jval_set_method(&prototype, "recvStop", RecvStop);
  iotjs_jval_set_method(&prototype, "send", Send);
  iotjs_jval_set_method(&prototype, "close", Close);
  iotjs_jval_set_method(&prototype, "getsockname", GetSockeName);
  iotjs_jval_set_method(&prototype, "setBroadcast", SetBroadcast);
  iotjs_jval_set_method(&prototype, "setTTL", SetTTL);
  iotjs_jval_set_method(&prototype, "setMulticastTTL", SetMulticastTTL);
  iotjs_jval_set_method(&prototype, "setMulticastLoopback",
                        SetMulticastLoopback);
  iotjs_jval_set_method(&prototype, "addMembership", AddMembership);
  iotjs_jval_set_method(&prototype, "dropMembership", DropMembership);
  iotjs_jval_set_method(&prototype, "ref", Ref);
  iotjs_jval_set_method(&prototype, "unref", Unref);

  iotjs_jval_destroy(&prototype);

  return udp;
}
