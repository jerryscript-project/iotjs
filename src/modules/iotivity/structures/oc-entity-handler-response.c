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

#include "../common.h"
#include "oc-header-option-array.c"
#include "oc-payload.h"

/* Converts JS-side response object from OCDoResponse
 * to C struct.
 *
 * @param response C struct.
 * @param js_response JS-side response object.
 */
bool c_OCEntityHandlerResponse(OCEntityHandlerResponse* response,
                               const iotjs_jval_t* js_response) {
  OCEntityHandlerResponse local;
  local.numSendVendorSpecificHeaderOptions = 0;

  // requestHandle
  iotjs_jval_t js_req_handle =
      iotjs_jval_get_property(js_response, "requestHandle");
  uintptr_t req_handle = iotjs_jval_get_object_native_handle(&js_req_handle);
  local.requestHandle = (OCRequestHandle)req_handle;

  // resourceHandle
  iotjs_jval_t js_res_handle =
      iotjs_jval_get_property(js_response, "resourceHandle");
  if (iotjs_exist_on_js_side(&js_res_handle)) {
    uintptr_t res_handle = iotjs_jval_get_object_native_handle(&js_res_handle);
    local.resourceHandle = (OCResourceHandle*)res_handle;
  }

  // ehResult
  iotjs_jval_t js_eh_result = iotjs_jval_get_property(js_response, "ehResult");
  local.ehResult = (OCEntityHandlerResult)iotjs_jval_as_number(&js_eh_result);

  // payload
  local.payload = 0;
  iotjs_jval_t js_payload = iotjs_jval_get_property(js_response, "payload");
  if (!iotjs_jval_is_null(&js_payload)) {
    c_OCPayload(&local.payload, &js_payload);
  }

  // VendorSpecificHeaderOptions
  iotjs_jval_t js_sendVSHO =
      iotjs_jval_get_property(js_response, "sendVendorSpecificHeaderOptions");
  c_OCHeaderOption(local.sendVendorSpecificHeaderOptions, &js_sendVSHO,
                   &local.numSendVendorSpecificHeaderOptions);

  // resourceUri
  iotjs_jval_t js_resource_uri =
      iotjs_jval_get_property(js_response, "resourceUri");
  iotjs_string_t str_resource_uri = iotjs_jval_as_string(&js_resource_uri);
  const char* resource_uri = iotjs_string_data(&str_resource_uri);

  size_t length;
  length = strlen(resource_uri);
  if (length >= MAX_URI_LENGTH) {
    DLOG("Resource URI cannot fit inside MAX_URI_LENGTH");
    return false;
  }

  strncpy(local.resourceUri, resource_uri, MAX_URI_LENGTH);

  iotjs_jval_destroy(&js_req_handle);
  iotjs_jval_destroy(&js_res_handle);
  iotjs_jval_destroy(&js_eh_result);
  iotjs_jval_destroy(&js_payload);
  iotjs_jval_destroy(&js_sendVSHO);
  iotjs_jval_destroy(&js_resource_uri);
  iotjs_string_destroy(&str_resource_uri);

  *response = local;
  return true;
}
