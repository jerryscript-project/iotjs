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

var intervalId,
  handleReceptacle = {},
  iotivity = require('iotivity'),
  options = "/oic/res";

console.log("Starting OCF stack in client mode");

// Start iotivity and set up the processing loop
iotivity.OCInit(null, 0, iotivity.OCMode.OC_CLIENT);

intervalId = setInterval(function () {
  var result = iotivity.OCProcess();
  if (result === 256) {
    console.log("SIGINT: Quitting...");
    iotivity.OCStop();
    process.exit(0);
  }
}, 1000);

console.log("Issuing discovery request");

// Discover resources and list them
iotivity.OCDoResource(

  // The bindings fill in this object
  handleReceptacle,

  iotivity.OCMethod.OC_REST_DISCOVER,

  // Standard path for discovering devices/resources
  iotivity.OC_MULTICAST_DISCOVERY_URI,

  // There is no destination
  null,

  // There is no payload
  null,
  iotivity.OCConnectivityType.CT_DEFAULT,
  iotivity.OCQualityOfService.OC_HIGH_QOS,

  function (handle, response) {
    console.log("Discovery response: " +
      JSON.stringify(response, null, 4));
    return iotivity.OCStackApplicationResult.OC_STACK_KEEP_TRANSACTION;
  },

  // There are no header options
  null);
