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

var iotivity = require('iotivity'),
  handleReceptacle = {},
  discoverCallback = function (flag, request) {
    console.log("Entity handler called with flag = " +
      flag + " and the following request:");
    console.log(JSON.stringify(request, null, 4));
    return iotivity.OCEntityHandlerResult.OC_EH_OK;
  },
  intervalId;

console.log("Starting OCF stack in server mode");

iotivity.OCInit(null, 0, iotivity.OCMode.OC_SERVER);

intervalId = setInterval(function () {
  var result = iotivity.OCProcess();
  if (result === 256) {
    console.log("SIGINT: Quitting...");
    iotivity.OCStop();
    process.exit(0);
  }
}, 1000);

iotivity.OCSetDeviceInfo({
  dataModelVersions: ["test.0.0.1"],
  deviceName: "server.discoverable",
  types: []
});

iotivity.OCSetPlatformInfo({
  platformID: "server.discoverable.sample",
  manufacturerName: "iotivity-node",
});

console.log("Registering resources");

iotivity.OCCreateResource(
  handleReceptacle,

  "core.fan",
  "oic.if.baseline",
  "a/fan",

  discoverCallback,
  iotivity.OCResourceProperty.OC_DISCOVERABLE
);

iotivity.OCCreateResource(
  handleReceptacle,

  "core.light",
  "oic.if.baseline",
  "a/light",

  function (flag, request) {
    console.log("Entity handler called with flag = " +
      flag + " and the following request:");
    console.log(JSON.stringify(request, null, 4));
    return iotivity.OCEntityHandlerResult.OC_EH_OK;
  },
  iotivity.OCResourceProperty.OC_DISCOVERABLE
);

console.log("Server ready");
