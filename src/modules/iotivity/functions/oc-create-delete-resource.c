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

#include "iotjs_def.h"
#include "iotjs_objectwrap.h"
#include "iotjs_reqwrap.h"
#include "../common.h"
#include "../structures.h"

/* Adds new data to the list of iotjs_callback_server_t info.
 *
 * @param node It's iotjs_callback_server_t struct with C-side and JS-side data.
 */
void add_handler(iotjs_callback_server_t* node) {
  iotjs_server_node_t* temp = IOTJS_ALLOC(iotjs_server_node_t);
  temp->value = node;
  temp->next = server_handler;
  server_handler = temp;
}

/* Changes server handler. When OCBindResourceHandler method is called
 * it is better to just change JS-side callback, than deleting whole handler
 * and adding new.
 *
 * @param cb_server Server handler which has new JS-side callback.
 */
void change_handler(iotjs_callback_server_t* cb_server) {
  iotjs_server_node_t* temp;
  temp = server_handler;
  while (temp != NULL) {
    if (temp->value->handle == cb_server->handle) {
      iotjs_jval_destroy(&temp->value->js_callback);
      temp->value->js_callback = cb_server->js_callback;
      return;
    }
    temp = temp->next;
  }
}

/*
 * Gets JS-side attr by C attr.
 */
#define GET_ATTR_FROM_SERVER_LIST(js_object, attr) \
  iotjs_server_node_t* temp;                       \
  temp = server_handler;                           \
  while (temp != NULL) {                           \
    if (temp->value->attr == attr) {               \
      js_object = &temp->value->js_##attr;         \
      break;                                       \
    }                                              \
    temp = temp->next;                             \
  }

/* Deletes handler from list.
 *
 * @param server iotjs_callback_server_t which will be deleted.
 */
void delete_handler(iotjs_callback_server_t* server) {
  iotjs_server_node_t *temp, *prev = NULL;

  temp = server_handler;

  while (temp != NULL) {
    if (temp->value->handle == server->handle) {
      if (temp == server_handler) {
        server_handler = (temp->next);
      } else {
        prev->next = temp->next;
      }
      iotjs_jval_destroy(&(temp->value->js_callback));
      iotjs_jval_destroy(&(temp->value->js_handle));
      free(temp->value);
      free(temp);
      break;
    } else {
      prev = temp;
      temp = temp->next;
    }
  }
}

/* When call OCCreateResource, it is required to set typeName,
 * interfaceName and uri. Iotjs, currently, does not support
 * passing 6 arguments to function, so there is need of
 * passing JS object with these strings and convert them.
 *
 * @param res_names Struct with above strings.
 * @param js_object Object from JS.
 */
void iotjs_create_resources(create_resources* res_names,
                            const iotjs_jval_t* js_object) {
  iotjs_jval_t js_type_name = iotjs_jval_get_property(js_object, "typeName");
  iotjs_jval_t js_interface_name =
      iotjs_jval_get_property(js_object, "interfaceName");
  iotjs_jval_t js_uri = iotjs_jval_get_property(js_object, "uri");

  iotjs_string_t str_type_name = iotjs_jval_as_string(&js_type_name);
  iotjs_string_t str_interface_name = iotjs_jval_as_string(&js_interface_name);
  iotjs_string_t str_uri = iotjs_jval_as_string(&js_uri);

  res_names->typeName = get_string_from_iotjs(&str_type_name);
  res_names->interfaceName = get_string_from_iotjs(&str_interface_name);
  res_names->uri = get_string_from_iotjs(&str_uri);

  iotjs_jval_destroy(&js_type_name);
  iotjs_jval_destroy(&js_interface_name);
  iotjs_jval_destroy(&js_uri);
  iotjs_string_destroy(&str_type_name);
  iotjs_string_destroy(&str_interface_name);
  iotjs_string_destroy(&str_uri);
}

/* Convert OCEntityHandlerRequest to JS object. Used in server-side callback.
 *
 * @param js_request JS object.
 * @param request Request which has to be converted to JS object.
 */
void js_OCEntityHandlerRequest(const iotjs_jval_t* js_request,
                               OCEntityHandlerRequest* request) {
  iotjs_jval_t js_obs_info = iotjs_jval_create_object();
  iotjs_jval_t js_dev_addr = iotjs_jval_create_object();

  js_OCObservationInfo(&js_obs_info, &request->obsInfo);
  js_OCDevAddr(&js_dev_addr, &request->devAddr);

  iotjs_jval_set_property_jval(js_request, "obsInfo", &js_obs_info);
  iotjs_jval_set_property_jval(js_request, "devAddr", &js_dev_addr);

  if (request->resource) {
    OCResourceHandle handle = request->resource;
    const iotjs_jval_t* js_res_handle = NULL;
    GET_ATTR_FROM_SERVER_LIST(js_res_handle, handle);
    iotjs_jval_set_property_jval(js_request, "resource", js_res_handle);
  }

  if (request->requestHandle) {
    uintptr_t req_handle = (uintptr_t)(request->requestHandle);
    iotjs_jval_t js_req_handle = iotjs_jval_create_object();
    iotjs_jval_set_object_native_handle(&js_req_handle, req_handle, NULL);
    iotjs_jval_set_property_jval(js_request, "requestHandle", &js_req_handle);
    iotjs_jval_destroy(&js_req_handle);
  }

  iotjs_jval_set_property_number(js_request, "method", request->method);

  if (request->query) {
    iotjs_jval_set_property_string_raw(js_request, "query", request->query);
  }

  js_OCHeaderOption(js_request, request->rcvdVendorSpecificHeaderOptions,
                    request->numRcvdVendorSpecificHeaderOptions);

  if (request->payload) {
    iotjs_jval_t js_payload = iotjs_jval_create_object();
    js_OCPayload(&js_payload, request->payload);
    iotjs_jval_set_property_jval(js_request, "payload", &js_payload);
    iotjs_jval_destroy(&js_payload);
  }

  iotjs_jval_destroy(&js_obs_info);
  iotjs_jval_destroy(&js_dev_addr);
}
