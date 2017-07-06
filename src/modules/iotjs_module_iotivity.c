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

#include "iotjs_module_iotivity.h"
#include <signal.h>

int start_server_node() {
  if (server_handler) {
    return 1;
  }
  server_handler = IOTJS_ALLOC(iotjs_server_node_t);
  server_handler->value = NULL;
  server_handler->next = NULL;
  return 0;
}

int start_client_node() {
  if (client_handler) {
    return 1;
  }
  client_handler = IOTJS_ALLOC(iotjs_client_node_t);
  client_handler->value = NULL;
  client_handler->next = NULL;
  return 0;
}

void clear_nodes() {
  iotjs_client_node_t *temp, *prev;
  temp = client_handler;
  while (temp != NULL) {
    prev = temp;
    iotjs_jval_destroy(&(temp->value->js_callback));
    iotjs_jval_destroy(&(temp->value->js_handle));
    temp = temp->next;
    free(prev->value);
    free(prev);
  }

  iotjs_server_node_t *temps, *prevs;
  temps = server_handler;
  while (temps != NULL) {
    prevs = temps;
    if (temps->value) {
      if (!temps->value->handle) {
        iotjs_jval_destroy(&(temps->value->js_callback));
      }
    }
    temps = temps->next;
    free(prevs->value);
    free(prevs);
  }
}

// Callback for OCSetDefaultDeviceEntityHandler.
// It calls JS-side callback with the same args.
OCEntityHandlerResult defaultDeviceEntityHandler(
    OCEntityHandlerFlag flag, OCEntityHandlerRequest* request, char* uri,
    void* callback) {
  iotjs_jargs_t jarg = iotjs_jargs_create(3);

  iotjs_jval_t js_flag = iotjs_jval_create_number(flag);
  iotjs_jval_t js_request = iotjs_jval_create_object();
  iotjs_jval_t js_uri = iotjs_jval_create_string_raw(uri);

  js_OCEntityHandlerRequest(&js_request, request);

  iotjs_jargs_append_jval(&jarg, &js_flag);
  iotjs_jargs_append_jval(&jarg, &js_request);
  iotjs_jargs_append_jval(&jarg, &js_uri);

  iotjs_jval_t iotjs_result =
      iotjs_make_callback_with_result(&(((iotjs_callback_server_t*)callback)
                                            ->js_callback),
                                      iotjs_jval_get_undefined(), &jarg);

  OCEntityHandlerResult result = iotjs_jval_as_number(&iotjs_result);

  iotjs_jargs_destroy(&jarg);
  iotjs_jval_destroy(&js_flag);
  iotjs_jval_destroy(&js_request);
  iotjs_jval_destroy(&js_uri);
  iotjs_jval_destroy(&iotjs_result);
  return result;
}

// Callback for OCCreateResource function.
// It calls JS-side callback with the same args.
OCEntityHandlerResult OCEntityHandlerCb(OCEntityHandlerFlag flag,
                                        OCEntityHandlerRequest* request,
                                        void* callback) {
  iotjs_jargs_t jarg = iotjs_jargs_create(2);
  iotjs_jval_t js_flag = iotjs_jval_create_number(flag);
  iotjs_jval_t js_request = iotjs_jval_create_object();

  js_OCEntityHandlerRequest(&js_request, request);

  iotjs_jargs_append_jval(&jarg, &js_flag);
  iotjs_jargs_append_jval(&jarg, &js_request);

  iotjs_jval_t iotjs_result =
      iotjs_make_callback_with_result(&(((iotjs_callback_server_t*)callback)
                                            ->js_callback),
                                      iotjs_jval_get_undefined(), &jarg);

  OCEntityHandlerResult result = iotjs_jval_as_number(&iotjs_result);

  iotjs_jargs_destroy(&jarg);
  iotjs_jval_destroy(&js_flag);
  iotjs_jval_destroy(&js_request);
  iotjs_jval_destroy(&iotjs_result);
  return result;
}

// Create an object containing the information from an OCClientResponse
// structure.
OCStackApplicationResult OCClientResponseHandlerCb(
    void* ctx, OCDoHandle handle, OCClientResponse* clientResponse) {
  iotjs_jargs_t jarg = iotjs_jargs_create(2);

  iotjs_jval_t js_handle = get_js_handle_client_by_handle(handle);
  iotjs_jargs_append_jval(&jarg, &js_handle);

  js_OCClientResponse(&jarg, clientResponse);

  iotjs_jval_t iotjs_result =
      iotjs_make_callback_with_result(&(((iotjs_callback_client_t*)ctx)
                                            ->js_callback),
                                      iotjs_jval_get_undefined(), &jarg);

  OCStackApplicationResult result = iotjs_jval_as_number(&iotjs_result);

  iotjs_jargs_destroy(&jarg);
  iotjs_jval_destroy(&iotjs_result);
  return result;
}

JHANDLER_FUNCTION(_OCInit) {
  JHANDLER_CHECK_ARGS(3, string, number, number);

  iotjs_string_t str_addr = JHANDLER_GET_ARG(0, string);
  char* ip_addr = get_string_from_iotjs(&str_addr);
  int port = JHANDLER_GET_ARG(1, number);
  int mode = JHANDLER_GET_ARG(2, number);

  OCStackResult result = OCInit(ip_addr, port, mode);

  if (result == OC_STACK_OK) {
    if (mode == OC_SERVER) {
      if (start_server_node() == 1) {
        iotjs_jval_t jerror =
            iotjs_jval_create_error("Server already initialized");
        iotjs_jhandler_throw(jhandler, &jerror);
        iotjs_jval_destroy(&jerror);
        return;
      }
    }
    if (mode == OC_CLIENT) {
      if (start_client_node() == 1) {
        iotjs_jval_t jerror =
            iotjs_jval_create_error("Client already initialized");
        iotjs_jhandler_throw(jhandler, &jerror);
        iotjs_jval_destroy(&jerror);
        return;
      }
    }
  }
  iotjs_string_destroy(&str_addr);
  iotjs_jhandler_return_number(jhandler, result);
}

int g_quit_flag = 0;

void handle_sig_int(int signum) {
  if (signum == SIGINT) {
    g_quit_flag = 1;
  }
}

JHANDLER_FUNCTION(_OCProcess) {
  OCStackResult result = OCProcess();
  signal(SIGINT, handle_sig_int);
  if (g_quit_flag == 1) {
    result = 256;
  }
  iotjs_jhandler_return_number(jhandler, result);
}

JHANDLER_FUNCTION(_OCSetDeviceInfo) {
  JHANDLER_CHECK_ARGS(1, object);
  OCDeviceInfo device_info;

  c_OCSetDeviceInfo(&device_info, JHANDLER_GET_ARG(0, object));

  OCStackResult result = OCSetDeviceInfo(device_info);

  free((char*)device_info.deviceName);
  free((char*)device_info.specVersion);
  free_linked_list(device_info.types);
  free_linked_list(device_info.dataModelVersions);
  iotjs_jhandler_return_number(jhandler, result);
}

JHANDLER_FUNCTION(_OCSetPlatformInfo) {
  JHANDLER_CHECK_ARGS(1, object);
  OCPlatformInfo platform_info;

  c_OCSetPlatformInfo(&platform_info, JHANDLER_GET_ARG(0, object));

  OCStackResult result = OCSetPlatformInfo(platform_info);
  free_platform_info(&platform_info);
  iotjs_jhandler_return_number(jhandler, result);
}

JHANDLER_FUNCTION(_OCStop) {
  OCStackResult result = OCStop();
  clear_nodes();
  server_handler = NULL;
  client_handler = NULL;
  iotjs_jhandler_return_number(jhandler, result);
}

JHANDLER_FUNCTION(_OCCreateResource) {
  JHANDLER_CHECK_ARGS(4, object, object, object, number);
  create_resources resources;

  const iotjs_jval_t* js_handle = JHANDLER_GET_ARG(0, object);

  iotjs_create_resources(&resources, JHANDLER_GET_ARG(1, object));

  iotjs_callback_server_t* cb_server = IOTJS_ALLOC(iotjs_callback_server_t);
  const iotjs_jval_t* cb_object = JHANDLER_GET_ARG(2, object);
  cb_server->js_callback = iotjs_jval_get_property(cb_object, "callback");

  uint8_t mode = JHANDLER_GET_ARG(3, number);

  cb_server->handle = 0;
  cb_server->callback = OCEntityHandlerCb;

  OCStackResult result =
      OCCreateResource(&(cb_server->handle), resources.typeName,
                       resources.interfaceName, resources.uri,
                       cb_server->callback, (void*)cb_server, mode);

  if (result == OC_STACK_OK) {
    uintptr_t handle = (uintptr_t)(cb_server->handle);
    cb_server->js_handle = iotjs_jval_create_object();
    iotjs_jval_set_object_native_handle(&(cb_server->js_handle), handle, NULL);
    iotjs_jval_set_property_jval(js_handle, "handle", &(cb_server->js_handle));

    add_handler(cb_server);
  }

  free((char*)resources.typeName);
  free((char*)resources.interfaceName);
  free((char*)resources.uri);
  iotjs_jhandler_return_number(jhandler, result);
}

JHANDLER_FUNCTION(_OCDeleteResource) {
  JHANDLER_CHECK_ARGS(1, object);

  iotjs_callback_server_t* server_handle = IOTJS_ALLOC(iotjs_callback_server_t);
  const iotjs_jval_t* js_handle = JHANDLER_GET_ARG(0, object);
  uintptr_t handle = iotjs_jval_get_object_native_handle(js_handle);

  server_handle->handle = (void*)handle;
  server_handle->js_handle = *js_handle;

  OCStackResult result = OCDeleteResource(server_handle->handle);
  iotjs_jval_set_property_boolean(js_handle, "stale", true);
  delete_handler(server_handle);

  IOTJS_RELEASE(server_handle);
  iotjs_jhandler_return_number(jhandler, result);
}

JHANDLER_FUNCTION(_OCDoResource) {
  JHANDLER_CHECK_ARGS(5, object, object, object, object, array);

  const iotjs_jval_t* js_handle = JHANDLER_GET_ARG(0, object);

  do_resources* c_resources = IOTJS_ALLOC(do_resources);
  iotjs_do_resources(c_resources, JHANDLER_GET_ARG(1, object));

  OCPayload* payload = 0;
  c_OCPayload(&payload, JHANDLER_GET_ARG(2, object));
  iotjs_callback_client_t* cb_client = IOTJS_ALLOC(iotjs_callback_client_t);

  const iotjs_jval_t* callback_object = JHANDLER_GET_ARG(3, object);
  cb_client->js_callback = iotjs_jval_get_property(callback_object, "callback");
  cb_client->handle = 0;
  uint8_t optionCount = 0;
  OCHeaderOption* options = IOTJS_ALLOC(OCHeaderOption);
  c_OCHeaderOption(options, JHANDLER_GET_ARG(4, array), &optionCount);

  OCCallbackData* cbData = IOTJS_ALLOC(OCCallbackData);
  cbData->cb = OCClientResponseHandlerCb;
  cbData->context = (void*)(cb_client);

  OCStackResult result =
      OCDoResource(&(cb_client->handle), c_resources->method,
                   c_resources->requestUri, c_resources->devAddr, payload,
                   c_resources->connType, c_resources->qos, cbData, options,
                   optionCount);

  if (result == OC_STACK_OK) {
    uintptr_t handle = (uintptr_t)(cb_client->handle);
    cb_client->js_handle = iotjs_jval_create_object();
    iotjs_jval_set_object_native_handle(&(cb_client->js_handle), handle, NULL);
    iotjs_jval_set_property_jval(js_handle, "handle", &(cb_client->js_handle));
    add_handler_client(cb_client);
  }

  free(c_resources->devAddr);
  free((char*)c_resources->requestUri);
  IOTJS_RELEASE(c_resources);
  IOTJS_RELEASE(cbData);
  IOTJS_RELEASE(options);
  iotjs_jhandler_return_number(jhandler, result);
}

JHANDLER_FUNCTION(_OCCancel) {
  JHANDLER_CHECK_ARGS(3, object, number, array);

  iotjs_callback_client_t* client_handle = IOTJS_ALLOC(iotjs_callback_client_t);
  const iotjs_jval_t* js_handle = JHANDLER_GET_ARG(0, object);
  uintptr_t handle = iotjs_jval_get_object_native_handle(js_handle);

  client_handle->handle = (OCDoHandle)handle;

  OCQualityOfService qos = (OCQualityOfService)JHANDLER_GET_ARG(1, number);
  uint8_t optionCount = 0;
  OCHeaderOption* options = IOTJS_ALLOC(OCHeaderOption);
  c_OCHeaderOption(options, JHANDLER_GET_ARG(2, array), &optionCount);

  OCStackResult result =
      OCCancel(client_handle->handle, qos, options, optionCount);
  delete_handler_client(client_handle->handle);
  IOTJS_RELEASE(options);
  IOTJS_RELEASE(client_handle);
  iotjs_jhandler_return_number(jhandler, result);
}

JHANDLER_FUNCTION(_OCGetServerInstanceIDString) {
  const char* idString = OCGetServerInstanceIDString();
  iotjs_jhandler_return_string_raw(jhandler, idString);
}

JHANDLER_FUNCTION(_OCGetNumberOfResources) {
  JHANDLER_CHECK_ARGS(1, object);
  const iotjs_jval_t* js_resource_count = JHANDLER_GET_ARG(0, object);

  uint8_t resource_count = 0;
  OCStackResult result = OCGetNumberOfResources(&resource_count);
  if (result == OC_STACK_OK) {
    iotjs_jval_set_property_number(js_resource_count, "count", resource_count);
  }
  iotjs_jhandler_return_number(jhandler, result);
}

JHANDLER_FUNCTION(_OCDoResponse) {
  JHANDLER_CHECK_ARGS(1, object);
  const iotjs_jval_t* js_response = JHANDLER_GET_ARG(0, object);

  OCEntityHandlerResponse* response = IOTJS_ALLOC(OCEntityHandlerResponse);
  if (!c_OCEntityHandlerResponse(response, js_response)) {
    iotjs_jhandler_return_number(jhandler, 255);
    return;
  }

  OCStackResult result = OCDoResponse(response);
  if (response->payload) {
    OCPayloadDestroy(response->payload);
  }
  IOTJS_RELEASE(response);
  iotjs_jhandler_return_number(jhandler, result);
}

JHANDLER_FUNCTION(_OCNotifyAllObservers) {
  JHANDLER_CHECK_ARGS(2, object, number);

  BOILER_PLATE_SET_HANDLE(handle)

  OCQualityOfService qos = (OCQualityOfService)JHANDLER_GET_ARG(1, number);

  OCStackResult result = OCNotifyAllObservers(member_handle, qos);

  iotjs_jhandler_return_number(jhandler, result);
}

JHANDLER_FUNCTION(_OCNotifyListOfObservers) {
  JHANDLER_CHECK_ARGS(4, object, array, object, number);

  BOILER_PLATE_SET_HANDLE(handle)

  const iotjs_jval_t* js_observations = JHANDLER_GET_ARG(1, array);

  OCObservationId* c_observations = NULL;
  uint8_t array_length = (uint8_t)get_jarray_size(js_observations);
  int index;

  if (array_length > 0) {
    c_observations = malloc(array_length * sizeof(OCObservationId));
  }

  iotjs_jval_t js_value;
  for (index = 0; index < array_length; index++) {
    js_value =
        iotjs_jval_get_property_by_index(js_observations, (uint32_t)index);
    c_observations[index] = (uint8_t)iotjs_jval_as_number(&js_value);
    iotjs_jval_destroy(&js_value);
  }

  OCRepPayload* payload = 0;
  c_OCRepPayload(&payload, JHANDLER_GET_ARG(2, object));

  OCQualityOfService qos = (OCQualityOfService)JHANDLER_GET_ARG(3, number);

  OCStackResult result = OCNotifyListOfObservers(member_handle, c_observations,
                                                 array_length, payload, qos);

  free(c_observations);
  OCRepPayloadDestroy(payload);
  iotjs_jhandler_return_number(jhandler, result);
}

JHANDLER_FUNCTION(_OCStartPresence) {
  JHANDLER_CHECK_ARGS(1, number);
  uint32_t ttl = (uint32_t)JHANDLER_GET_ARG(0, number);

  OCStackResult result = OCStartPresence(ttl);
  iotjs_jhandler_return_number(jhandler, result);
}

JHANDLER_FUNCTION(_OCStopPresence) {
  OCStackResult result = OCStopPresence();
  iotjs_jhandler_return_number(jhandler, result);
}

JHANDLER_FUNCTION(_OCRegisterPersistentStorageHandler) {
  JHANDLER_CHECK_ARGS(1, object);
  const iotjs_jval_t* js_callbacks = JHANDLER_GET_ARG(0, object);

  VALIDATE_MEMBER(js_callbacks, open)
  VALIDATE_MEMBER(js_callbacks, close)
  VALIDATE_MEMBER(js_callbacks, read)
  VALIDATE_MEMBER(js_callbacks, write)
  VALIDATE_MEMBER(js_callbacks, unlink)

  OCStackResult result = OCRegisterPersistentStorageHandler(&ps_callbacks);

  if (result == OC_STACK_OK) {
    callback_for_open = iotjs_jval_get_property(js_callbacks, "open");
    callback_for_close = iotjs_jval_get_property(js_callbacks, "close");
    callback_for_write = iotjs_jval_get_property(js_callbacks, "write");
    callback_for_unlink = iotjs_jval_get_property(js_callbacks, "unlink");
    callback_for_read = iotjs_jval_get_property(js_callbacks, "read");
  }

  iotjs_jhandler_return_number(jhandler, result);
}

JHANDLER_FUNCTION(_OCGetNumberOfResourceInterfaces) {
  JHANDLER_CHECK_ARGS(2, object, object);

  BOILER_PLATE_SET_HANDLE(handle)

  uint8_t count = 0;
  const iotjs_jval_t* js_count = JHANDLER_GET_ARG(1, object);

  OCStackResult result = OCGetNumberOfResourceInterfaces(member_handle, &count);

  if (result == OC_STACK_OK) {
    iotjs_jval_set_property_number(js_count, "count", count);
  }

  iotjs_jhandler_return_number(jhandler, result);
}

JHANDLER_FUNCTION(_OCGetNumberOfResourceTypes) {
  JHANDLER_CHECK_ARGS(2, object, object);

  BOILER_PLATE_SET_HANDLE(handle)
  uint8_t count = 0;
  const iotjs_jval_t* js_count = JHANDLER_GET_ARG(1, object);

  OCStackResult result = OCGetNumberOfResourceTypes(member_handle, &count);

  if (result == OC_STACK_OK) {
    iotjs_jval_set_property_number(js_count, "count", count);
  }

  iotjs_jhandler_return_number(jhandler, result);
}

JHANDLER_FUNCTION(_OCGetResourceHandle) {
  JHANDLER_CHECK_ARGS(1, number);

  uint8_t index = JHANDLER_GET_ARG(0, number);

  OCResourceHandle handle = OCGetResourceHandle(index);

  if (handle == NULL) {
    iotjs_jhandler_return_null(jhandler);
    return;
  } else {
    iotjs_jval_t* native_handle = NULL;
    GET_ATTR_FROM_SERVER_LIST(native_handle, handle);
    iotjs_jhandler_return_jval(jhandler, native_handle);
  }
}

JHANDLER_FUNCTION(_OCGetResourceHandleFromCollection) {
  JHANDLER_CHECK_ARGS(2, object, number);

  BOILER_PLATE_SET_HANDLE(handle1)

  uint8_t index = JHANDLER_GET_ARG(1, number);

  OCResourceHandle handle =
      OCGetResourceHandleFromCollection(member_handle1, index);

  if (handle == NULL) {
    iotjs_jhandler_return_null(jhandler);
  } else {
    iotjs_jval_t* native_handle = NULL;
    GET_ATTR_FROM_SERVER_LIST(native_handle, handle);
    iotjs_jhandler_return_jval(jhandler, native_handle);
  }
}

JHANDLER_FUNCTION(_OCGetResourceInterfaceName) {
  JHANDLER_CHECK_ARGS(2, object, number);

  BOILER_PLATE_SET_HANDLE(handle)

  uint8_t index = JHANDLER_GET_ARG(1, number);

  const char* result = OCGetResourceInterfaceName(member_handle, index);
  if (result == NULL) {
    iotjs_jhandler_return_null(jhandler);
  } else {
    iotjs_jhandler_return_string_raw(jhandler, result);
  }
}

JHANDLER_FUNCTION(_OCGetResourceTypeName) {
  JHANDLER_CHECK_ARGS(2, object, number);

  BOILER_PLATE_SET_HANDLE(handle)

  uint8_t index = JHANDLER_GET_ARG(1, number);

  const char* result = OCGetResourceTypeName(member_handle, index);

  if (result == NULL) {
    iotjs_jhandler_return_null(jhandler);
  } else {
    iotjs_jhandler_return_string_raw(jhandler, result);
  }
}

JHANDLER_FUNCTION(_OCGetResourceProperties) {
  JHANDLER_CHECK_ARGS(1, object);

  BOILER_PLATE_SET_HANDLE(handle)

  OCResourceProperty result = OCGetResourceProperties(member_handle);

  iotjs_jhandler_return_number(jhandler, result);
}

JHANDLER_FUNCTION(_OCGetResourceUri) {
  JHANDLER_CHECK_ARGS(1, object);

  BOILER_PLATE_SET_HANDLE(handle)

  const char* result = OCGetResourceUri(member_handle);
  if (result == NULL) {
    iotjs_jhandler_return_null(jhandler);
  } else {
    iotjs_jhandler_return_string_raw(jhandler, result);
  }
}

JHANDLER_FUNCTION(_OCGetResourceHandler) {
  JHANDLER_CHECK_ARGS(1, object);

  BOILER_PLATE_SET_HANDLE(handle)

  OCEntityHandler callback = OCGetResourceHandler(member_handle);

  iotjs_jval_t* return_handle = NULL;
  GET_ATTR_FROM_SERVER_LIST(return_handle, callback);

  iotjs_jhandler_return_jval(jhandler, return_handle);
}

JHANDLER_FUNCTION(_OCBindResourceHandler) {
  JHANDLER_CHECK_ARGS(2, object, object);

  BOILER_PLATE_SET_HANDLE(handle)

  iotjs_callback_server_t* cb_server = IOTJS_ALLOC(iotjs_callback_server_t);
  const iotjs_jval_t* cb_object = JHANDLER_GET_ARG(1, object);
  cb_server->js_callback = iotjs_jval_get_property(cb_object, "callback");

  cb_server->handle = member_handle;
  cb_server->callback = OCEntityHandlerCb;

  OCStackResult result =
      OCBindResourceHandler(cb_server->handle, cb_server->callback,
                            (void*)cb_server);

  if (result == OC_STACK_OK) {
    change_handler(cb_server);
    free(cb_server);
  }

  iotjs_jhandler_return_number(jhandler, result);
}

JHANDLER_FUNCTION(_OCBindResourceInterfaceToResource) {
  JHANDLER_CHECK_ARGS(2, object, string);

  BOILER_PLATE_SET_HANDLE(handle)

  iotjs_string_t str_name = JHANDLER_GET_ARG(1, string);
  char* c_name = get_string_from_iotjs(&str_name);

  OCStackResult result =
      OCBindResourceInterfaceToResource(member_handle, c_name);
  free(c_name);
  iotjs_string_destroy(&str_name);
  iotjs_jhandler_return_number(jhandler, result);
}

JHANDLER_FUNCTION(_OCBindResourceTypeToResource) {
  JHANDLER_CHECK_ARGS(2, object, string);

  BOILER_PLATE_SET_HANDLE(resource)

  iotjs_string_t str_name = JHANDLER_GET_ARG(1, string);
  char* c_name = get_string_from_iotjs(&str_name);

  OCStackResult result = OCBindResourceTypeToResource(member_resource, c_name);

  free(c_name);
  iotjs_string_destroy(&str_name);
  iotjs_jhandler_return_number(jhandler, result);
}

JHANDLER_FUNCTION(_OCBindResource) {
  JHANDLER_CHECK_ARGS(2, object, object);

  BOILER_PLATE_BIND(collection, resource)

  OCStackResult result = OCBindResource(member_collection, member_resource);

  iotjs_jhandler_return_number(jhandler, result);
}

JHANDLER_FUNCTION(_OCUnBindResource) {
  JHANDLER_CHECK_ARGS(2, object, object);

  BOILER_PLATE_BIND(collection, resource)

  OCStackResult result = OCUnBindResource(member_collection, member_resource);

  iotjs_jhandler_return_number(jhandler, result);
}

JHANDLER_FUNCTION(_OCSetDefaultDeviceEntityHandler) {
  JHANDLER_CHECK_ARGS(1, object);

  OCDeviceEntityHandler handler = 0;

  const iotjs_jval_t* callback_object = JHANDLER_GET_ARG(0, object);
  iotjs_jval_t callback = iotjs_jval_get_property(callback_object, "callback");

  iotjs_callback_server_t* entity_handler =
      IOTJS_ALLOC(iotjs_callback_server_t);
  entity_handler->js_callback = callback;
  handler = defaultDeviceEntityHandler;

  OCStackResult result =
      OCSetDefaultDeviceEntityHandler(handler, (void*)entity_handler);
  add_handler(entity_handler);
  iotjs_jhandler_return_number(jhandler, result);
}

iotjs_jval_t InitIotivity() {
  iotjs_jval_t iotivity = iotjs_jval_create_object();

  iotjs_jval_set_method(&iotivity, IOTJS_MAGIC_STRING_OC_INIT, _OCInit);
  iotjs_jval_set_method(&iotivity, IOTJS_MAGIC_STRING_OC_PROCESS, _OCProcess);
  iotjs_jval_set_method(&iotivity, IOTJS_MAGIC_STRING_OC_SET_DEVICE_INFO,
                        _OCSetDeviceInfo);
  iotjs_jval_set_method(&iotivity, IOTJS_MAGIC_STRING_OC_SET_PLATFORM_INFO,
                        _OCSetPlatformInfo);
  iotjs_jval_set_method(&iotivity, IOTJS_MAGIC_STRING_OC_CREATE_RESOURCE,
                        _OCCreateResource);
  iotjs_jval_set_method(&iotivity, IOTJS_MAGIC_STRING_OC_DELETE_RESOURCE,
                        _OCDeleteResource);
  iotjs_jval_set_method(&iotivity, IOTJS_MAGIC_STRING_OC_STOP, _OCStop);
  iotjs_jval_set_method(&iotivity, IOTJS_MAGIC_STRING_OC_DO_RESOURCE,
                        _OCDoResource);
  iotjs_jval_set_method(&iotivity, IOTJS_MAGIC_STRING_OC_CANCEL, _OCCancel);
  iotjs_jval_set_method(&iotivity, IOTJS_MAGIC_STRING_OC_GET_SERVER_ID,
                        _OCGetServerInstanceIDString);
  iotjs_jval_set_method(&iotivity, IOTJS_MAGIC_STRING_OC_GET_NUM_OF_RES,
                        _OCGetNumberOfResources);
  iotjs_jval_set_method(&iotivity, IOTJS_MAGIC_STRING_OC_DO_RESPONSE,
                        _OCDoResponse);
  iotjs_jval_set_method(&iotivity, IOTJS_MAGIC_STRING_OC_NOTIFY_ALL_OBS,
                        _OCNotifyAllObservers);
  iotjs_jval_set_method(&iotivity, IOTJS_MAGIC_STRING_OC_NOTIFY_LIST_OF_OBS,
                        _OCNotifyListOfObservers);
  iotjs_jval_set_method(&iotivity, IOTJS_MAGIC_STRING_OC_START_PRESENCE,
                        _OCStartPresence);
  iotjs_jval_set_method(&iotivity, IOTJS_MAGIC_STRING_OC_STOP_PRESENCE,
                        _OCStopPresence);
  iotjs_jval_set_method(&iotivity, IOTJS_MAGIC_STRING_OC_REGISTER_PS,
                        _OCRegisterPersistentStorageHandler);
  iotjs_jval_set_method(&iotivity, IOTJS_MAGIC_STRING_OC_GET_NUM_OF_RES_IFACES,
                        _OCGetNumberOfResourceInterfaces);
  iotjs_jval_set_method(&iotivity, IOTJS_MAGIC_STRING_OC_GET_NUM_OF_RES_TYPES,
                        _OCGetNumberOfResourceTypes);
  iotjs_jval_set_method(&iotivity, IOTJS_MAGIC_STRING_OC_GET_RES_HANDLE,
                        _OCGetResourceHandle);
  iotjs_jval_set_method(&iotivity,
                        IOTJS_MAGIC_STRING_OC_GET_RES_HANDLE_FROM_COLLECTION,
                        _OCGetResourceHandleFromCollection);
  iotjs_jval_set_method(&iotivity, IOTJS_MAGIC_STRING_OC_GET_RES_IFACE_NAME,
                        _OCGetResourceInterfaceName);
  iotjs_jval_set_method(&iotivity, IOTJS_MAGIC_STRING_OC_GET_RES_TYPE_NAME,
                        _OCGetResourceTypeName);
  iotjs_jval_set_method(&iotivity, IOTJS_MAGIC_STRING_OC_GET_RES_PROPERTIES,
                        _OCGetResourceProperties);
  iotjs_jval_set_method(&iotivity, IOTJS_MAGIC_STRING_OC_GET_RES_URI,
                        _OCGetResourceUri);
  iotjs_jval_set_method(&iotivity, IOTJS_MAGIC_STRING_OC_GET_RES_HANDLER,
                        _OCGetResourceHandler);
  iotjs_jval_set_method(&iotivity, IOTJS_MAGIC_STRING_OC_BIND_RES_HANDLER,
                        _OCBindResourceHandler);
  iotjs_jval_set_method(&iotivity, IOTJS_MAGIC_STRING_OC_BIND_RES_IFACE_TO_RES,
                        _OCBindResourceInterfaceToResource);
  iotjs_jval_set_method(&iotivity, IOTJS_MAGIC_STRING_OC_BIND_RES_TYPE_TO_RES,
                        _OCBindResourceTypeToResource);
  iotjs_jval_set_method(&iotivity, IOTJS_MAGIC_STRING_OC_BIND_RES,
                        _OCBindResource);
  iotjs_jval_set_method(&iotivity, IOTJS_MAGIC_STRING_OC_UNBIND_RES,
                        _OCUnBindResource);
  iotjs_jval_set_method(&iotivity, IOTJS_MAGIC_STRING_OC_SET_DEFAULT_DEVICE_EH,
                        _OCSetDefaultDeviceEntityHandler);

  return iotivity;
}
