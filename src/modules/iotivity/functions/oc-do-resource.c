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

/* Adds new data to the list of iotjs_callback_client_t info.
 *
 * @param node It's iotjs_callback_client_t struct with C-side and JS-side data.
 */
void add_handler_client(iotjs_callback_client_t* node) {
  iotjs_client_node_t* temp = IOTJS_ALLOC(iotjs_client_node_t);
  temp->value = node;
  temp->next = client_handler;
  client_handler = temp;
}

/* Gets JS-side handle by C-API handle.
 *
 * @param handle C-API handle.
 *
 * @returns JS-side iotjs_jval_t handle.
 */
iotjs_jval_t get_js_handle_client_by_handle(OCDoHandle handle) {
  iotjs_client_node_t* temp;

  temp = client_handler;

  while (temp != NULL) {
    if (temp->value->handle == handle) {
      return (temp->value->js_handle);
    }
    temp = temp->next;
  }
  return iotjs_jval_create_object();
}

/* Deletes handler from list.
 *
 * @param server iotjs_callback_client_t which will be deleted.
 */
void delete_handler_client(OCDoHandle handle) {
  iotjs_client_node_t *temp, *prev = NULL;

  temp = client_handler;

  while (temp != NULL) {
    if (temp->value->handle == handle) {
      if (temp == client_handler) {
        client_handler = (temp->next);
      } else {
        prev->next = temp->next;
      }
      iotjs_jval_destroy(&(temp->value->js_callback));
      iotjs_jval_destroy(&(temp->value->js_handle));
      free(temp->value);
      free(temp);
      return;
    } else {
      prev = temp;
      temp = temp->next;
    }
  }
}

/* When call OCDoResource, it is required to set requestUri,
 * method, connectivityType, qos and destination. Iotjs,
 * currently, does not support passing > 5 arguments to
 * function, so there is need of passing JS object with
 * these fields and convert them.
 *
 * @param res Struct with above fields.
 * @param js_object Object from JS.
 */
void iotjs_do_resources(do_resources* resources,
                        const iotjs_jval_t* js_object) {
  iotjs_jval_t js_request_uri =
      iotjs_jval_get_property(js_object, "requestUri");
  iotjs_jval_t js_method = iotjs_jval_get_property(js_object, "method");
  iotjs_jval_t js_conn_type = iotjs_jval_get_property(js_object, "connType");
  iotjs_jval_t js_qos = iotjs_jval_get_property(js_object, "qos");
  iotjs_jval_t js_destination =
      iotjs_jval_get_property(js_object, "destination");

  iotjs_string_t str_request_uri = iotjs_jval_as_string(&js_request_uri);

  if (iotjs_exist_on_js_side(&js_destination)) {
    resources->devAddr = IOTJS_ALLOC(OCDevAddr);
    c_OCDevAddr(resources->devAddr, &js_destination);
  }
  resources->requestUri = get_string_from_iotjs(&str_request_uri);
  resources->method = iotjs_jval_as_number(&js_method);
  resources->connType = iotjs_jval_as_number(&js_conn_type);
  resources->qos = iotjs_jval_as_number(&js_qos);

  iotjs_jval_destroy(&js_request_uri);
  iotjs_jval_destroy(&js_method);
  iotjs_jval_destroy(&js_conn_type);
  iotjs_jval_destroy(&js_qos);
  iotjs_jval_destroy(&js_destination);
  iotjs_string_destroy(&str_request_uri);
}

/* Convert OCClientResponse to JS object. Used in client-side callback.
 *
 * @param jarg JS object.
 * @param response Response which has to be converted to JS object.
 */
void js_OCClientResponse(iotjs_jargs_t* jarg, OCClientResponse* response) {
  iotjs_jval_t js_dev_addr = iotjs_jval_create_object();
  iotjs_jval_t js_addr = iotjs_jval_create_object();
  iotjs_jval_t js_payload = iotjs_jval_create_object();
  iotjs_jval_t js_identity = iotjs_jval_create_array(0);

  iotjs_jval_t js_response = iotjs_jval_create_object();

  // response.devAddr
  js_OCDevAddr(&js_dev_addr, &response->devAddr);
  iotjs_jval_set_property_jval(&js_response, "devAddr", &js_dev_addr);

  // response.addr
  if (response->addr) {
    js_OCDevAddr(&js_addr, response->addr);
    iotjs_jval_set_property_jval(&js_response, "addr", &js_addr);
  }

  // response.payload
  if (response->payload) {
    js_OCPayload(&js_payload, response->payload);
    iotjs_jval_set_property_jval(&js_response, "payload", &js_payload);
  }

  // response.identity
  js_OCIdentity(&js_identity, &response->identity);
  iotjs_jval_set_property_jval(&js_response, "identity", &js_identity);

  iotjs_jval_set_property_number(&js_response, "result", response->result);
  iotjs_jval_set_property_number(&js_response, "connType", response->connType);
  iotjs_jval_set_property_number(&js_response, "sequenceNumber",
                                 response->sequenceNumber);

  if (!(response->payload &&
        response->payload->type == PAYLOAD_TYPE_PRESENCE)) {
    iotjs_jval_t rcvdVSHO = iotjs_jval_create_array(0);
    iotjs_jval_set_property_string_raw(&js_response, "resourceUri",
                                       response->resourceUri);
    js_OCHeaderOption(&rcvdVSHO, response->rcvdVendorSpecificHeaderOptions,
                      response->numRcvdVendorSpecificHeaderOptions);
    iotjs_jval_set_property_jval(&js_response,
                                 "rcvdVendorSpecificHeaderOptions", &rcvdVSHO);
    iotjs_jval_destroy(&rcvdVSHO);
  }
  iotjs_jargs_append_jval(jarg, &js_response);

  iotjs_jval_destroy(&js_dev_addr);
  iotjs_jval_destroy(&js_addr);
  iotjs_jval_destroy(&js_payload);
  iotjs_jval_destroy(&js_identity);
  iotjs_jval_destroy(&js_response);
}
