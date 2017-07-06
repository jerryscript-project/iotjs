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
  getCallback = function (flag, request) {
    console.log("Entity handler called with flag = " + flag +
      " and the following request:");
    console.log(JSON.stringify(request, null, 4));

    // If we find the magic question in the request, we return the magic answer
    if (request && request.payload && request.payload.values &&
      request.payload.values.question ===
      "How many angels can dance on the head of pin?") {

      iotivity.OCDoResponse({
        requestHandle: request.requestHandle,
        resourceHandle: request.resource,
        ehResult: iotivity.OCEntityHandlerResult.OC_EH_OK,
        payload: {
          type: iotivity.OCPayloadType.PAYLOAD_TYPE_REPRESENTATION,
          values: {
            name: "answer",
            value: "As many as wanting."
          }
        },
        resourceUri: sampleUri,
        sendVendorSpecificHeaderOptions: []
      });

      return iotivity.OCEntityHandlerResult.OC_EH_OK;
    }

    // By default we error out
    return iotivity.OCEntityHandlerResult.OC_EH_ERROR;
  },
  // This is the same value as server.get.js
  sampleUri = "/a/iotivity-node-get-sample",
  iotivity = require("iotivity");

console.log("Starting OCF stack in server mode");

// Start iotivity and set up the processing loop
iotivity.OCInit(null, 0, iotivity.OCMode.OC_SERVER);

iotivity.OCSetDeviceInfo({
  specVersion: "res.1.1.0",
  dataModelVersions: ["abc.0.0.1"],
  deviceName: "server.get",
  types: []
});
iotivity.OCSetPlatformInfo({
  platformID: "server.get.sample",
  manufacturerName: "iotivity-node"
});

intervalId = setInterval(function () {
  var result = iotivity.OCProcess();
  if (result === 256) {
    console.log("SIGINT: Quitting...");
    iotivity.OCStop();
    process.exit(0);
  }
}, 1000);

console.log("Registering resource");

// Create a new resource
iotivity.OCCreateResource(

  // The bindings fill in this object
  handleReceptacle,

  "core.fan",
  "oic.if.baseline",
  sampleUri,
  getCallback,
  iotivity.OCResourceProperty.OC_DISCOVERABLE);

console.log("Server ready");
