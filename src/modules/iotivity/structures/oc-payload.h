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

#ifndef IOTIVITY_PAYLOAD_H
#define IOTIVITY_PAYLOAD_H

#define SET_OCSTRING(payload, attr)    \
  OCStringLL* c_##attr;                \
  c_OCStringLL(&c_##attr, &js_##attr); \
  payload->attr = c_##attr;

/* Sets OCRepPayload C-struct with JS-side objects.
 *
 * @param p_payload OCRepPayload C structure.
 * @param js_payload Object from JS.
 *
 * @returns true if finish successful.
 */
bool c_OCRepPayload(OCRepPayload** p_payload, const iotjs_jval_t* js_payload);

/* Sets proper OCPayload C-struct depending on type.
 *
 * @param payload OCPayload C structure.
 * @param js_payload Object from JS.
 */
void c_OCPayload(OCPayload** payload, const iotjs_jval_t* js_payload);

/* Converts OCStringLL C-structure to JS array.
 *
 * @param jarray JS array.
 * @param str OCStringLL C structure.
 */
void js_OCStringLL(const iotjs_jval_t* jarray, OCStringLL* str);

/* Converts OCResourcePayload C-structure to JS object.
 *
 * @param req JS object.
 * @param payload OCResourcePayload C structure.
 */
void js_OCResourcePayload(const iotjs_jval_t* req, OCResourcePayload* payload);

/* Converts OCDiscoveryPayload C-structure to JS object.
 *
 * @param req JS object.
 * @param payload OCDiscoveryPayload C structure.
 */
void js_OCDiscoveryPayload(const iotjs_jval_t* req,
                           OCDiscoveryPayload* payload);

/* Creates payloadValueArray recursively. This array is used
 * in @function js_OCRepPayload.
 *
 * @param array OCRepPayloadValueArray which has to be converted
 * to JS array.
 * @param p_data_index Current data array index.
 * @param dimension_index OCRepPayloadValueArray current index.
 * @param jarray JS array.
 */
void create_payload_array_recursively(OCRepPayloadValueArray* array,
                                      size_t* p_data_index, int dimension_index,
                                      const iotjs_jval_t* jarray);

/* Calls create_payload_array_recursively.
 *
 * @param jarray JS array.
 * @param array OCRepPayloadValueArray which has to be converted
 * to JS array.
 */
void js_OCRepPayloadValueArray(const iotjs_jval_t* jarray,
                               OCRepPayloadValueArray* array);

/* Converts OCRepPayload C-structure to JS object.
 *
 * @param req JS object.
 * @param payload OCRepPayload C-structure.
 */
void js_OCRepPayload(const iotjs_jval_t* req, OCRepPayload* payload);

/* Calls proper payload-convert function depending on type.
 * C-lang does not have generic types, so it is important
 * to go around this issue.
 *
 * @param req JS object.
 * @param payload OCPayload C-structure.
 */
void js_OCPayload(const iotjs_jval_t* req, OCPayload* payload);

bool flatten_array(iotjs_jval_t* array, void** flat_array,
                   size_t dimensions[MAX_REP_ARRAY_DEPTH],
                   OCRepPayloadPropType array_type);

bool validate_rep_payload_array(iotjs_jval_t* jarray, bool* type_established,
                                OCRepPayloadPropType* array_type,
                                size_t dimensions[MAX_REP_ARRAY_DEPTH],
                                int index);

#endif
