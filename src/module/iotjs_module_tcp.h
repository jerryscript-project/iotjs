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


#ifndef IOTJS_MODULE_TCP_H
#define IOTJS_MODULE_TCP_H

#include "iotjs_binding.h"


namespace iotjs {


iotjs_jval_t InitTcp();

void DoClose(iotjs_jhandler_t* jhandler);
void AfterClose(uv_handle_t* handle);
void AddressToJS(const iotjs_jval_t* obj, const sockaddr* addr);

#define GetSockNameFunction(wrap_type, handle_type, function) \
  static void DoGetSockName(iotjs_jhandler_t* jhandler) { \
    JHANDLER_CHECK_ARGS_1(object); \
    \
    wrap_type* wrap = \
        wrap_type::FromJObject(JHANDLER_GET_THIS(object)); \
    IOTJS_ASSERT(wrap != NULL); \
    \
    sockaddr_storage storage; \
    int addrlen = sizeof(storage); \
    sockaddr* const addr = reinterpret_cast<sockaddr*>(&storage); \
    const int err = function(wrap->handle_type(), addr, &addrlen); \
    if (err == 0) \
      AddressToJS(JHANDLER_GET_ARG(0, object), addr); \
    iotjs_jhandler_return_number(jhandler, err); \
  }

} // namespace iotjs


#endif /* IOTJS_MODULE_TCP_H */
