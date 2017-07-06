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

/* Sets OCDeviceInfo struct with values from JS-side.
 *
 * @param device_info OCDeviceInfo C structure.
 * @param js_object Object from JS.
 */
void c_OCSetDeviceInfo(OCDeviceInfo* device_info,
                       const iotjs_jval_t* js_object) {
  iotjs_jval_t js_device_name =
      iotjs_jval_get_property(js_object, "deviceName");
  iotjs_jval_t js_spec_version =
      iotjs_jval_get_property(js_object, "specVersion");
  iotjs_jval_t js_data_model_versions =
      iotjs_jval_get_property(js_object, "dataModelVersions");
  iotjs_jval_t js_types = iotjs_jval_get_property(js_object, "types");

  device_info->deviceName = get_string_from_jval(js_device_name);
  device_info->specVersion = get_string_from_jval(js_spec_version);

  c_OCStringLL(&(device_info->dataModelVersions), &js_data_model_versions);
  c_OCStringLL(&(device_info->types), &js_types);

  iotjs_jval_destroy(&js_device_name);
  iotjs_jval_destroy(&js_spec_version);
  iotjs_jval_destroy(&js_data_model_versions);
  iotjs_jval_destroy(&js_types);
}
