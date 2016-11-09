/* Copyright 2016 Samsung Electronics Co., Ltd.
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


class UdpWrap : public HandleWrap {
 public:
  explicit UdpWrap(const iotjs_jval_t* judp)
      : HandleWrap(judp, reinterpret_cast<uv_handle_t*>(&_handle)) {
    const iotjs_environment_t* env = iotjs_environment_get();
    uv_udp_init(iotjs_environment_loop(env), &_handle);
  }

  static UdpWrap* FromJObject(const iotjs_jval_t* judp) {
    UdpWrap* wrap = reinterpret_cast<UdpWrap*>(
                iotjs_jval_get_object_native_handle(judp));
    IOTJS_ASSERT(wrap != NULL);
    return wrap;
  }

  uv_udp_t* udp_handle() {
    return &_handle;
  }

 protected:
  uv_udp_t _handle;
};

class SendWrap: public ReqWrap<uv_udp_send_t> {
 public:
  SendWrap(const iotjs_jval_t* jcallback, const size_t msg_size)
      : ReqWrap<uv_udp_send_t>(jcallback),
        _msg_size(msg_size) {
  }

  inline size_t msg_size() {
    return _msg_size;
  }

 protected:
  size_t _msg_size;
};


JHANDLER_FUNCTION(UDP) {
  JHANDLER_CHECK_THIS(object);
  JHANDLER_CHECK_ARGS(0);

  const iotjs_jval_t* judp = iotjs_jhandler_get_this(jhandler);

  UdpWrap* udp_wrap = new UdpWrap(judp);
  IOTJS_ASSERT(iotjs_jval_is_object(udp_wrap->jobject()));
  IOTJS_ASSERT(iotjs_jval_get_object_native_handle(judp) != 0);
}


JHANDLER_FUNCTION(Bind) {
  JHANDLER_CHECK_THIS(object);
  JHANDLER_CHECK_ARGS_2(string, number);

  UdpWrap* udp_wrap = UdpWrap::FromJObject(iotjs_jhandler_get_this(jhandler));
  IOTJS_ASSERT(udp_wrap != NULL);

  iotjs_string_t address = JHANDLER_GET_ARG(0, string);
  const int port = JHANDLER_GET_ARG(1, number);
  const iotjs_jval_t* this_obj = JHANDLER_GET_THIS(object);
  iotjs_jval_t reuse_addr = iotjs_jval_get_property(this_obj, "_reuseAddr");
  IOTJS_ASSERT(iotjs_jval_is_boolean(&reuse_addr) ||
               iotjs_jval_is_undefined(&reuse_addr));

  int flags = false;
  if (!iotjs_jval_is_undefined(&reuse_addr)) {
    flags = iotjs_jval_as_boolean(&reuse_addr)? UV_UDP_REUSEADDR : 0;
  }

  char addr[sizeof(sockaddr_in6)];
  int err = uv_ip4_addr(iotjs_string_data(&address), port,
                        reinterpret_cast<sockaddr_in*>(&addr));

  if (err == 0) {
    err = uv_udp_bind(udp_wrap->udp_handle(),
                      reinterpret_cast<const sockaddr*>(&addr),
                      flags);
  }

  iotjs_jhandler_return_number(jhandler, err);

  iotjs_jval_destroy(&reuse_addr);
  iotjs_string_destroy(&address);
}


static void OnAlloc(uv_handle_t* handle, size_t suggested_size,
                    uv_buf_t* buf) {
  if (suggested_size > IOTJS_MAX_READ_BUFFER_SIZE) {
    suggested_size = IOTJS_MAX_READ_BUFFER_SIZE;
  }

  buf->base = iotjs_buffer_allocate(suggested_size);
  buf->len = suggested_size;
}


static void OnRecv(uv_udp_t* handle, ssize_t nread, const uv_buf_t* buf,
                   const struct sockaddr* addr, unsigned int flags) {
  if (nread == 0 && addr == nullptr) {
    if (buf->base != nullptr)
      iotjs_buffer_release(buf->base);
    return;
  }

  UdpWrap* udp_wrap = reinterpret_cast<UdpWrap*>(handle->data);
  IOTJS_ASSERT(udp_wrap != NULL);

  // udp handle
  const iotjs_jval_t* judp = udp_wrap->jobject();
  IOTJS_ASSERT(iotjs_jval_is_object(judp));

  // onmessage callback
  iotjs_jval_t jonmessage = iotjs_jval_get_property(judp, "onmessage");
  IOTJS_ASSERT(iotjs_jval_is_function(&jonmessage));

  iotjs_jargs_t jargs = iotjs_jargs_create(4);
  iotjs_jargs_append_number(&jargs, nread);
  iotjs_jargs_append_jval(&jargs, judp);

  if (nread < 0) {
    if (buf->base != nullptr)
      iotjs_buffer_release(buf->base);
    iotjs_make_callback(&jonmessage, iotjs_jval_get_undefined(), &jargs);
    iotjs_jval_destroy(&jonmessage);
    iotjs_jargs_destroy(&jargs);
    return;
  }

  iotjs_jval_t jbuffer(CreateBuffer(static_cast<size_t>(nread)));

  BufferWrap* buffer_wrap = BufferWrap::FromJBuffer(&jbuffer);
  buffer_wrap->Copy(buf->base, nread);

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

  UdpWrap* udp_wrap = UdpWrap::FromJObject(iotjs_jhandler_get_this(jhandler));
  IOTJS_ASSERT(udp_wrap != NULL);

  int err = uv_udp_recv_start(udp_wrap->udp_handle(),
                              OnAlloc, OnRecv);

  // UV_EALREADY means that the socket is already bound but that's okay
  if (err == UV_EALREADY)
    err = 0;

  iotjs_jhandler_return_number(jhandler, err);
}


JHANDLER_FUNCTION(RecvStop) {
  JHANDLER_CHECK_THIS(object);
  JHANDLER_CHECK_ARGS(0);

  UdpWrap* udp_wrap = UdpWrap::FromJObject(iotjs_jhandler_get_this(jhandler));
  IOTJS_ASSERT(udp_wrap != NULL);

  int r = uv_udp_recv_stop(udp_wrap->udp_handle());

  iotjs_jhandler_return_number(jhandler, r);
}


static void OnSend(uv_udp_send_t* req, int status) {
  SendWrap* req_wrap = static_cast<SendWrap*>(req->data);
  UdpWrap* udp_wrap = reinterpret_cast<UdpWrap*>(req->handle->data);
  IOTJS_ASSERT(req_wrap != NULL);
  IOTJS_ASSERT(udp_wrap != NULL);

  // Take callback function object.
  const iotjs_jval_t* jcallback = req_wrap->jcallback();

  if (iotjs_jval_is_function(jcallback)) {
    // Take callback function object.

    iotjs_jargs_t jargs = iotjs_jargs_create(2);
    iotjs_jargs_append_number(&jargs, status);
    iotjs_jargs_append_number(&jargs, req_wrap->msg_size());

    iotjs_make_callback(jcallback, iotjs_jval_get_undefined(), &jargs);
    iotjs_jargs_destroy(&jargs);
  }

  delete req_wrap;
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

  UdpWrap* udp_wrap = UdpWrap::FromJObject(iotjs_jhandler_get_this(jhandler));
  IOTJS_ASSERT(udp_wrap != NULL);

  const iotjs_jval_t* jbuffer = JHANDLER_GET_ARG(0, object);
  const unsigned short port = JHANDLER_GET_ARG(1, number);
  iotjs_string_t address = JHANDLER_GET_ARG(2, string);
  const iotjs_jval_t* jcallback = JHANDLER_GET_ARG(3, object);

  BufferWrap* buffer_wrap = BufferWrap::FromJBuffer(jbuffer);
  char* buffer = buffer_wrap->buffer();
  int len = buffer_wrap->length();

  SendWrap* req_wrap = new SendWrap(jcallback, len);

  uv_buf_t buf;
  buf.base = buffer;
  buf.len = len;

  char addr[sizeof(sockaddr_in6)];
  int err = uv_ip4_addr(iotjs_string_data(&address), port,
                        reinterpret_cast<sockaddr_in*>(&addr));

  if (err == 0) {
    err = uv_udp_send(req_wrap->req(), udp_wrap->udp_handle(), &buf, 1,
        reinterpret_cast<const sockaddr*>(&addr), OnSend);
  }

  if (err)
    delete req_wrap;

  iotjs_jhandler_return_number(jhandler, err);

  iotjs_string_destroy(&address);
}


// Close socket
JHANDLER_FUNCTION(Close) {
  DoClose(jhandler);
}


GetSockNameFunction(UdpWrap, udp_handle, uv_udp_getsockname);

JHANDLER_FUNCTION(GetSockeName) {
  DoGetSockName(jhandler);
}

#define IOTJS_UV_SET_SOCKOPT(fn)                                              \
  JHANDLER_CHECK_THIS(object);                                                \
  JHANDLER_CHECK_ARGS_1(number);                                              \
  \
  UdpWrap* udp_wrap = UdpWrap::FromJObject(iotjs_jhandler_get_this(jhandler));\
  IOTJS_ASSERT(udp_wrap != NULL);                                             \
  \
  int flag = JHANDLER_GET_ARG(0, number);                                     \
  int err = fn(udp_wrap->udp_handle(), flag);                                 \
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

  UdpWrap* udp_wrap = UdpWrap::FromJObject(iotjs_jhandler_get_this(jhandler));
  IOTJS_ASSERT(udp_wrap != NULL);

  iotjs_string_t address = JHANDLER_GET_ARG(0, string);
  const iotjs_jval_t* arg1 = iotjs_jhandler_get_arg(jhandler, 1);
  bool isUndefinedOrNull = iotjs_jval_is_undefined(arg1) ||
                           iotjs_jval_is_null(arg1);
  iotjs_string_t iface;

  const char* iface_cstr;
  if (isUndefinedOrNull) {
    iface_cstr = nullptr;
  } else {
    iface = iotjs_jval_as_string(arg1);
    iface_cstr = iotjs_string_data(&iface);
  }

  int err = uv_udp_set_membership(udp_wrap->udp_handle(),
                                  iotjs_string_data(&address),
                                  iface_cstr,
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


} // namespace iotjs


extern "C" {

iotjs_jval_t InitUdp() {
  return iotjs::InitUdp();
}

} // extern "C"
