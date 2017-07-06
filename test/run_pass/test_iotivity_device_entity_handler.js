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

var assert = require('assert'),
  t = require('tools/iotivity-utils.js'),
  uuid = "test",
  processLoop = null,
  done = false,
  resourceHandleReceptacle = {},
  discoverHandleReceptacle = {},
  iotivity = require('iotivity'),
  ok = iotivity.OCStackResult.OC_STACK_OK,
  msg = " should return OC_STACK_OK",
  result;

function doGetRequest(destination) {
  var getHandleReceptacle = {};

  result = iotivity.OCDoResource(
    getHandleReceptacle,
    iotivity.OCMethod.OC_REST_GET,
    "/a/" + uuid + "-xyzzy",
    destination,
    {
      type: iotivity.OCPayloadType.PAYLOAD_TYPE_REPRESENTATION,
      values: {
        question: "Do you need something?"
      }
    },
    iotivity.OCConnectivityType.CT_DEFAULT,
    iotivity.OCQualityOfService.OC_HIGH_QOS,
    function(handle, response) {
      var returnValue =
        iotivity.OCStackApplicationResult.OC_STACK_KEEP_TRANSACTION;

      if (response && response.payload && response.payload.type ===
        iotivity.OCPayloadType.PAYLOAD_TYPE_REPRESENTATION &&
        response.payload.values && response.payload.values.answer) {

        assert.strictEqual(response.payload.values.answer,
          "You are asking wrong resource.", "Incorrect response received");

        returnValue =
          iotivity.OCStackApplicationResult.OC_STACK_DELETE_TRANSACTION;
        done = true;
      }
      else {
        assert.equal(false, true, "Unexpected GET response");
      }

      return returnValue;
    },
    null
  );
  assert.equal(result, ok, "OCDoResource(client)" + msg);
}

result = iotivity.OCInit(null, 0, iotivity.OCMode.OC_CLIENT_SERVER);
assert.equal(result, ok, "OCInit" + msg);

result = iotivity.OCSetDefaultDeviceEntityHandler(
  function(flag, request, uri) {
    result = t.checkBitFields(iotivity.OCEntityHandlerFlag, flag);
    assert.equal(result.OC_REQUEST_FLAG, true, "Incoming request doesn't" +
      " have correct flag");

    assert.strictEqual(uri, "/a/" + uuid + "-xyzzy", "Incoming request has" +
      " wrong Uri");

    if (request && request.payload && request.payload.type ===
      iotivity.OCPayloadType.PAYLOAD_TYPE_REPRESENTATION &&
      request.payload.values && request.payload.values.question) {
      assert.strictEqual(request.payload.values.question,
        "Do you need something?", "Incorrect request received");

      result = iotivity.OCDoResponse({
        requestHandle: request.requestHandle,
        resourceHandle: request.resource,
        ehResult: iotivity.OCEntityHandlerResult.OC_EH_OK,
        payload: {
          type: iotivity.OCPayloadType.PAYLOAD_TYPE_REPRESENTATION,
          values: {
            answer: "You are asking wrong resource."
          }
        },
        resourceUri: "/a/" + uuid,
        sendVendorSpecificHeaderOptions: []
      });
      assert.equal(result, ok, "OCDoResponse(server)" + msg);
    }
    else {
      assert.equal(false, true, "Unexpected request");
    }
    return iotivity.OCEntityHandlerResult.OC_EH_OK;
  }
);
assert.equal(result, ok, "OCSetDefaultDeviceEntityHandler(server)" + msg);

processLoop = setInterval(function() {
  var processResult = iotivity.OCProcess();
  assert.equal(processResult, ok, "OCProcess" + msg);
  if (done === true) {
    setTimeout(function() {
      process.exit(0);
    }, 500);
    done = false;
  }
}, 100);

result = iotivity.OCCreateResource(
  resourceHandleReceptacle,
  "core.fan",
  iotivity.OC_RSRVD_INTERFACE_DEFAULT,
  "/a/" + uuid,
  function() {
    return iotivity.OCEntityHandlerResult.OC_EH_ERROR;
  },
  iotivity.OCResourceProperty.OC_DISCOVERABLE
);
assert.equal(result, ok, "OCCreateResource(server)" + msg);

result = iotivity.OCDoResource(
  discoverHandleReceptacle,
  iotivity.OCMethod.OC_REST_DISCOVER,
  iotivity.OC_MULTICAST_DISCOVERY_URI,
  null,
  null,
  iotivity.OCConnectivityType.CT_DEFAULT,
  iotivity.OCQualityOfService.OC_HIGH_QOS,
  function (handle, response) {
    var returnValue =
      iotivity.OCStackApplicationResult.OC_STACK_KEEP_TRANSACTION;

    assert.equal(t.findResource(response, uuid), true,
      "Client did not found resource");

    if (t.findResource(response, uuid)) {
      doGetRequest(response.addr);
      returnValue =
        iotivity.OCStackApplicationResult.OC_STACK_DELETE_TRANSACTION;
    }

    return returnValue;
  },
  null
);
assert.equal(result, ok, "OCDoResource(client)" + msg);

process.on('exit', function(code) {
  clearInterval(processLoop);
  processLoop = null;
  result = iotivity.OCDeleteResource(resourceHandleReceptacle.handle);
  assert.equal(result, ok, "OCDeleteResource(server)" + msg);
  assert.equal(iotivity.OCStop(), ok, "OCStop" + msg);
});
