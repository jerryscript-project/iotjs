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
  handleReceptacle2 = {},
  observerIds = [],
  observeCallback = function (flag, request) {
    if (request.resourceUri === "stop") {
      iotivity.OCStop();
      process.exit(0);
    }
    console.log("Entity handler called with flag = " + flag +
      " and the following request:");
    console.log(JSON.stringify(request, null, 4));

    if (flag & iotivity.OCEntityHandlerFlag.OC_OBSERVE_FLAG) {
      if (request.obsInfo.obsId !== 0) {
        if (request.obsInfo.action ===
          iotivity.OCObserveAction.OC_OBSERVE_REGISTER) {

          // Add new observer to list.
          observerIds.push(request.obsInfo.obsId);
        } else if (request.obsInfo.action ===
          iotivity.OCObserveAction.OC_OBSERVE_DEREGISTER) {

          // Remove requested observer from list.
          observerIdIndex = observerIds.indexOf(request.obsInfo.obsId);
          if (observerIdIndex >= 0) {
            observerIds.splice(observerIdIndex, 1);
          }
        }
      }
    }

    if (request.requestHandle) {
      iotivity.OCDoResponse({
        requestHandle: request.requestHandle,
        resourceHandle: request.resource,
        ehResult: iotivity.OCEntityHandlerResult.OC_EH_OK,
        payload: {
        },
        resourceUri: sampleUri,
        sendVendorSpecificHeaderOptions: []
      });
    }

    return iotivity.OCEntityHandlerResult.OC_EH_OK;
  },
  // This is the same value as server.get.js
  sampleUri = "/a/fan",
  iotivity = require("iotivity");

// Start iotivity and set up the processing loop
iotivity.OCInit(null, 0, iotivity.OCMode.OC_SERVER);

iotivity.OCSetDeviceInfo({
  specVersion: "res.1.1.0",
  dataModelVersions: ["abc.0.0.1"],
  deviceName: "server.observable",
  types: ["oic.d.tv"]
});

iotivity.OCSetPlatformInfo({
  platformID: "server.observe.sample",
  manufacturerName: "Samsung"
});

intervalId = setInterval(function () {
  var result = iotivity.OCProcess();
  if (result === 256) {
    console.log("SIGINT: Quitting...");
    iotivity.OCDeleteResource(handleReceptacle.handle);
    iotivity.OCDeleteResource(handleReceptacle2.handle);
    iotivity.OCStop();
    process.exit(0);
  }
}, 1000);

console.log("Local device ID: " + iotivity.OCGetServerInstanceIDString());

// Create a new resource
iotivity.OCCreateResource(

  // The bindings fill in this object
  handleReceptacle,

  "core.fan",
  "oic.if.baseline",
  sampleUri,

  observeCallback,
  iotivity.OCResourceProperty.OC_DISCOVERABLE |
  iotivity.OCResourceProperty.OC_OBSERVABLE);

iotivity.OCCreateResource(

  // The bindings fill in this object
  handleReceptacle2,

  "core.light",
  "oic.if.baseline",
  "/a/light",

  observeCallback,
  iotivity.OCResourceProperty.OC_DISCOVERABLE |
  iotivity.OCResourceProperty.OC_OBSERVABLE);

console.log("Server ready");
