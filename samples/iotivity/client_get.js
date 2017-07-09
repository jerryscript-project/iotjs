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
  sampleUri = "/a/iotivity-node-get-sample",
  iotivity = require('iotivity'),
  getResponseHandler = function (handle, response) {
    console.log("Received response to GET request:");
    console.log(JSON.stringify(response, null, 4));
    return iotivity.OCStackApplicationResult.OC_STACK_DELETE_TRANSACTION;
  };

console.log("Starting OCF stack in client mode");

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

iotivity.OCDoResource(
  handleReceptacle,

  iotivity.OCMethod.OC_REST_DISCOVER,
  "/oic/res",
  null,
  null,
  iotivity.OCConnectivityType.CT_DEFAULT,
  iotivity.OCQualityOfService.OC_HIGH_QOS,

  function (handle, response) {
    console.log("Received response to DISCOVERY request:");
    console.log(JSON.stringify(response, null, 4));

    var index,
      destination = response.addr,
      getHandleReceptacle = {},
      resources = response && response.payload && response.payload.resources,
      resourceCount = resources ? resources.length : 0;

    for (index = 0; index < resourceCount; index++) {
      if (resources[index].uri === sampleUri) {
        iotivity.OCDoResource(
          getHandleReceptacle,
          iotivity.OCMethod.OC_REST_GET,
          sampleUri,
          destination,

          {
            type: iotivity.OCPayloadType.PAYLOAD_TYPE_REPRESENTATION,
            values: {
              name: "question",
              value: "How many angels can dance on the head of pin?"
            }
          },
          iotivity.OCConnectivityType.CT_DEFAULT,
          iotivity.OCQualityOfService.OC_HIGH_QOS,
          getResponseHandler,
          null
        );
      }
    }
    return iotivity.OCStackApplicationResult.OC_STACK_KEEP_TRANSACTION;
  },
  null
);
