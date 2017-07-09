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
  sampleUri = "/a/fan",
  observerResponseCount = 0,
  resourceMissing = true,
  discCallback = function (handle, response) {
    console.log("Received response to DISCOVER request:");
    console.log(JSON.stringify(response, null, 4));

    var index,
      destination = response.addr,
      observeHandleReceptacle = {},
      resources = response && response.payload && response.payload.resources,
      resourceCount = resources ? resources.length : 0;

    if (resourceMissing) {
      for (index = 0; index < resourceCount; index++) {
        if (resources[index].uri === sampleUri) {
          resourceMissing = false;
          iotivity.OCDoResource(
            observeHandleReceptacle,
            iotivity.OCMethod.OC_REST_OBSERVE,
            sampleUri,
            destination,
            null,
            iotivity.OCConnectivityType.CT_DEFAULT,
            iotivity.OCQualityOfService.OC_HIGH_QOS,
            observeResponseHandler,
            null);
        }
      }
    }
    return iotivity.OCStackApplicationResult.OC_STACK_KEEP_TRANSACTION;
  },
  observeResponseHandler = function (handle, response) {
    console.log("Received response to OBSERVE request:");
    console.log(JSON.stringify(response, null, 4));
    if (++observerResponseCount >= 2) {
      console.log("Enough observations. Calling OCCancel()");
      iotivity.OCCancel(
        handle,
        iotivity.OCQualityOfService.OC_HIGH_QOS,
        array = []);
      return iotivity.OCStackApplicationResult.OC_STACK_DELETE_TRANSACTION;
    }
    else {
      return iotivity.OCStackApplicationResult.OC_STACK_KEEP_TRANSACTION;
    }
  },

  options = "/oic/res";

iotivity.OCInit(null, 0, iotivity.OCMode.OC_CLIENT);

intervalId = setInterval(function () {
  var result = iotivity.OCProcess();
  if (result === 256) {
    console.log("SIGINT: Quitting...");
    iotivity.OCStop();
    process.exit(0);
  }
}, 1000);

iotivity.OCDoResource(

  handleReceptacle,
  iotivity.OCMethod.OC_REST_DISCOVER,
  "/oic/res",
  null,
  null,
  iotivity.OCConnectivityType.CT_DEFAULT,
  iotivity.OCQualityOfService.OC_HIGH_QOS,

  discCallback,

  null);

