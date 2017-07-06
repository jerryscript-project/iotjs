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

/* Sets OCPlatformInfo not obligatory fields with values from JS-side.
 *
 * @param platform_info OCPlatformInfo C structure.
 * @param js_object Object from JS-side.
 */
void c_set_not_obligatory_platform_info(OCPlatformInfo *platform_info,
                                        const iotjs_jval_t *js_object) {
  iotjs_jval_t js_manufacturer_uri =
      iotjs_jval_get_property(js_object, "manufacturerUri");
  iotjs_jval_t js_model_number =
      iotjs_jval_get_property(js_object, "modelNumber");
  iotjs_jval_t js_date_of_manufacture =
      iotjs_jval_get_property(js_object, "dateOfManufacture");
  iotjs_jval_t js_platform_version =
      iotjs_jval_get_property(js_object, "platformVersion");
  iotjs_jval_t js_operating_system_version =
      iotjs_jval_get_property(js_object, "operatingSystemVersion");
  iotjs_jval_t js_hardware_version =
      iotjs_jval_get_property(js_object, "hardwareVersion");
  iotjs_jval_t js_firmware_version =
      iotjs_jval_get_property(js_object, "firmwareVersion");
  iotjs_jval_t js_support_url =
      iotjs_jval_get_property(js_object, "supportUrl");
  iotjs_jval_t js_system_time =
      iotjs_jval_get_property(js_object, "systemTime");

  platform_info->operatingSystemVersion =
      get_string_from_jval(js_operating_system_version);
  platform_info->dateOfManufacture =
      get_string_from_jval(js_date_of_manufacture);
  platform_info->manufacturerUrl = get_string_from_jval(js_manufacturer_uri);
  platform_info->platformVersion = get_string_from_jval(js_platform_version);
  platform_info->hardwareVersion = get_string_from_jval(js_hardware_version);
  platform_info->firmwareVersion = get_string_from_jval(js_firmware_version);
  platform_info->modelNumber = get_string_from_jval(js_model_number);
  platform_info->supportUrl = get_string_from_jval(js_support_url);
  platform_info->systemTime = get_string_from_jval(js_system_time);

  iotjs_jval_destroy(&js_manufacturer_uri);
  iotjs_jval_destroy(&js_model_number);
  iotjs_jval_destroy(&js_date_of_manufacture);
  iotjs_jval_destroy(&js_platform_version);
  iotjs_jval_destroy(&js_operating_system_version);
  iotjs_jval_destroy(&js_hardware_version);
  iotjs_jval_destroy(&js_firmware_version);
  iotjs_jval_destroy(&js_support_url);
  iotjs_jval_destroy(&js_system_time);
}

/* Sets OCPlatformInfo obligatory fields with values from JS-side.
 *
 * @param platform_info OCPlatformInfo C structure.
 * @param js_object Object from JS.
 */
void c_OCSetPlatformInfo(OCPlatformInfo *platform_info,
                         const iotjs_jval_t *js_object) {
  iotjs_jval_t js_platform_id =
      iotjs_jval_get_property(js_object, "platformID");
  iotjs_jval_t js_manufacturer_name =
      iotjs_jval_get_property(js_object, "manufacturerName");

  platform_info->platformID = get_string_from_jval(js_platform_id);
  platform_info->manufacturerName = get_string_from_jval(js_manufacturer_name);

  iotjs_jval_destroy(&js_platform_id);
  iotjs_jval_destroy(&js_manufacturer_name);

  c_set_not_obligatory_platform_info(platform_info, js_object);
}

/* Deallocates memory from OCPlatformInfo C struct.
 *
 * @param platform_info OCPlatformInfo C struct.
 */
void free_platform_info(OCPlatformInfo *platform_info) {
  /** Platform ID.*/
  free((char *)platform_info->platformID);

  /** Manufacturer name.*/
  free((char *)platform_info->manufacturerName);

  /** Manufacturer URL for platform property.*/
  free((char *)platform_info->manufacturerUrl);

  /** Model number.*/
  free((char *)platform_info->modelNumber);

  /** Manufacturer date.*/
  free((char *)platform_info->dateOfManufacture);

  /** Platform version.*/
  free((char *)platform_info->platformVersion);

  /** Operating system version.*/
  free((char *)platform_info->operatingSystemVersion);

  /** HW version.*/
  free((char *)platform_info->hardwareVersion);

  /** FW version.*/
  free((char *)platform_info->firmwareVersion);

  /** Platform support URL.*/
  free((char *)platform_info->supportUrl);

  /** System time.*/
  free((char *)platform_info->systemTime);
}
