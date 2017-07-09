/* Copyright 2017-present Samsung Electronics Co., Ltd. and other contributors
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

#ifndef IOTIVITY_COMMON_H
#define IOTIVITY_COMMON_H

#include "iotjs_def.h"
#include "iotjs_objectwrap.h"
#include "iotjs_reqwrap.h"
#include <ctype.h>
#include <inttypes.h>
#include <ocpayload.h>
#include <ocstack.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include "common.c"

/* Struct for passing and getting CallbackInfo
 * on server side.
 */
typedef struct iotjs_callback_server_t {
  // Holds handle from IoTivity C-API.
  OCResourceHandle handle;

  // Holds callback from IoTivity C-API.
  OCEntityHandler callback;

  // Holds callback from JS-side.
  iotjs_jval_t js_callback;

  // Holds handle from JS-side.
  iotjs_jval_t js_handle;
} iotjs_callback_server_t;

/* Struct for passing and getting CallbackInfo
 * on client side.
 */
typedef struct iotjs_callback_client_t {
  // Holds handle from IoTivity C-API.
  OCDoHandle handle;

  // Holds callback from JS-side.
  iotjs_jval_t js_callback;

  // Holds handle from JS-side.
  iotjs_jval_t js_handle;
} iotjs_callback_client_t;

/* Struct for passed resources to OCCreateResource from JS side.
 * We have to use this structure because iot.js does not support
 * passing 6 and more arguments from JS side.
 */
typedef struct {
  const char* interfaceName;
  const char* typeName;
  const char* uri;
} create_resources;

/* Struct for passed resources to OCDoResource from JS side
 * We have to use this structure because iot.js does not support
 * passing 6 and more arguments from JS side.
 */
typedef struct {
  OCMethod method;
  const char* requestUri;
  OCDevAddr* devAddr;
  OCConnectivityType connType;
  OCQualityOfService qos;
} do_resources;

/* List of all iotjs_callback_client_t structures.
 */
typedef struct iotjs_client_node_t {
  iotjs_callback_client_t* value;
  struct iotjs_client_node_t* next;
} iotjs_client_node_t;

/* Head of the above list.
 */
iotjs_client_node_t* client_handler;

/* List of all iotjs_callback_server_t structures.
 */
typedef struct iotjs_server_node_t {
  iotjs_callback_server_t* value;
  struct iotjs_server_node_t* next;
} iotjs_server_node_t;

/* Head of the above list.
 */
iotjs_server_node_t* server_handler;

#define BOILER_PLATE_SET_HANDLE(name)                                       \
  OCResourceHandle member_##name;                                           \
  const iotjs_jval_t* js_##name = JHANDLER_GET_ARG(0, object);              \
  uintptr_t handle_##name = iotjs_jval_get_object_native_handle(js_##name); \
  member_##name = (OCResourceHandle)handle_##name;

#endif /* IOTIVITY_COMMON_H */
