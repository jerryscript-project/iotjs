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

/* Sets OCHeaderOption struct with values from JS-side.
 *
 * @param dest OCHeaderOption C structure.
 * @param js_options Object from JS.
 * @param option_count Number of options.
 */
void c_OCHeaderOption(OCHeaderOption* dest, const iotjs_jval_t* js_options,
                      uint8_t* option_count) {
  uint8_t index;
  uint16_t d_index, length, data_length;
  OCHeaderOption options[MAX_HEADER_OPTIONS];

  length = get_jarray_size(js_options);
  for (index = 0; index < length; index++) {
    iotjs_jval_t js_array = iotjs_jval_get_property_by_index(js_options, index);
    iotjs_jval_t js_protocol_id =
        iotjs_jval_get_property(&js_array, "protocolID");
    options[index].protocolID =
        (OCTransportProtocolID)iotjs_jval_as_number(&js_protocol_id);

    iotjs_jval_t js_option_id = iotjs_jval_get_property(&js_array, "optionID");
    options[index].optionID = iotjs_jval_as_number(&js_option_id);

    iotjs_jval_t js_array_option_data =
        iotjs_jval_get_property(&js_array, "optionData");
    data_length = get_jarray_size(&js_array_option_data);

    for (d_index = 0; d_index < MAX_HEADER_OPTION_DATA_LENGTH; d_index++) {
      if (d_index < data_length) {
        iotjs_jval_t js_option_data =
            iotjs_jval_get_property_by_index(&js_array_option_data, d_index);
        options[index].optionData[d_index] =
            iotjs_jval_as_number(&js_option_data);
        iotjs_jval_destroy(&js_option_data);
      } else {
        options[index].optionData[d_index] = 0;
      }
    }
    options[index].optionLength = data_length;
    iotjs_jval_destroy(&js_array_option_data);
    iotjs_jval_destroy(&js_protocol_id);
    iotjs_jval_destroy(&js_option_id);
    iotjs_jval_destroy(&js_array);
  }
  if (length > 0) {
    memcpy(dest, options, MAX_HEADER_OPTIONS * sizeof(OCHeaderOption));
    *option_count = length;
  } else
    dest = NULL;
}

/* Sets JS-side OCHeaderOption-type object with values from C struct.
 *
 * @param req JS-side object.
 * @param options OCHeaderOption C structure.
 * @param option_count Number of options.
 */
void js_OCHeaderOption(const iotjs_jval_t* req, OCHeaderOption* options,
                       uint8_t option_count) {
  uint8_t index;
  uint16_t data_index;
  for (index = 0; index < option_count; index++) {
    iotjs_jval_t value = iotjs_jval_create_object();
    iotjs_jval_set_property_number(&value, "protocolID",
                                   options[index].protocolID);
    iotjs_jval_set_property_number(&value, "optionID", options[index].optionID);

    // options[index].optionData
    uint16_t option_length =
        (options[index].optionLength > MAX_HEADER_OPTION_DATA_LENGTH
             ? MAX_HEADER_OPTION_DATA_LENGTH
             : options[index].optionLength);
    iotjs_jval_t js_option_data = iotjs_jval_create_array(option_length);
    for (data_index = 0; data_index < option_length; data_index++) {
      iotjs_jval_t value_option_data =
          iotjs_jval_create_number(options[index].optionData[data_index]);
      iotjs_jval_set_property_by_index(&js_option_data, data_index,
                                       &value_option_data);
      iotjs_jval_destroy(&value_option_data);
    }

    iotjs_jval_set_property_jval(&value, "optionData", &js_option_data);
    iotjs_jval_destroy(&js_option_data);
    iotjs_jval_set_property_by_index(req, index, &value);
    iotjs_jval_destroy(&value);
  }
}
