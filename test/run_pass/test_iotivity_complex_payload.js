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
  resourceHandleReceptacle = {},
  discoverHandleReceptacle = {},
  done = false,
  iotivity = require('iotivity'),
  ok = iotivity.OCStackResult.OC_STACK_OK,
  msg = " should return OC_STACK_OK",
  result;

var compareObject = {
  type: iotivity.OCPayloadType.PAYLOAD_TYPE_REPRESENTATION,
  values: {
    primitiveValue: 44,
    objectValue: {
      type: iotivity.OCPayloadType.PAYLOAD_TYPE_REPRESENTATION,
      values: {
        childValue: 74
      }
    },
    arrayValue: [19, 23, 7]
  }
};

var compareObject2 = {
  type: iotivity.OCPayloadType.PAYLOAD_TYPE_REPRESENTATION,
  types: ['test.types'],
  interfaces: ['very.test.interfaces'],
  values: {
    putValue: "A string",
    anotherPutValue: 23.7,
    veryBooleanValue: true,
    childValues: {
      type: iotivity.OCPayloadType.PAYLOAD_TYPE_REPRESENTATION,
      values: {
        putChildValue: false,
        putChildArray: [[2, 3, 5], [9, 11, 17]],
        putBooleanArray: [true, false, false]
      }
    }
  },
  uri: 'too.complicated.uri',
  next: {
    type: iotivity.OCPayloadType.PAYLOAD_TYPE_REPRESENTATION,
    values: {
      nextValue: "here i am"
    }
  }
};

function doPutRequest(destination) {
  var putHandleReceptacle = {};

  result = iotivity.OCDoResource(
    putHandleReceptacle,
    iotivity.OCMethod.OC_REST_PUT,
    "/a/" + uuid,
    destination,
    {
      type: iotivity.OCPayloadType.PAYLOAD_TYPE_REPRESENTATION,
      types: ['test.types'],
      interfaces: ['very.test.interfaces'],
      values: {
        putValue: "A string",
        anotherPutValue: 23.7,
        veryBooleanValue: true,
        childValues: {
          type: iotivity.OCPayloadType.PAYLOAD_TYPE_REPRESENTATION,
          values: {
            putChildValue: false,
            putChildArray: [[2, 3, 5], [9, 11, 17]],
            putBooleanArray: [true, false, false]
          }
        }
      },
      uri: 'too.complicated.uri',
      next: {
        type: iotivity.OCPayloadType.PAYLOAD_TYPE_REPRESENTATION,
        values: {
          nextValue: "here i am"
        }
      }
    },
    iotivity.OCConnectivityType.CT_DEFAULT,
    iotivity.OCQualityOfService.OC_HIGH_QOS,
    function () {
      done = true;
      return iotivity.OCStackApplicationResult.
        OC_STACK_DELETE_TRANSACTION;
    },
    null,
    0
  );
  assert.equal(result, ok, "OCDoResource(put)" + msg);
}

function doGetRequest(destination) {
  var getHandleReceptacle = {};

  result = iotivity.OCDoResource(
    getHandleReceptacle,
    iotivity.OCMethod.OC_REST_GET,
    "/a/" + uuid,
    destination,
    null,
    iotivity.OCConnectivityType.CT_DEFAULT,
    iotivity.OCQualityOfService.OC_HIGH_QOS,
    function (handle, response) {
      if (t.payloadAssert(response.payload, compareObject)) {
        assert.equal(true, false, "Received payload is not correct");
      }

      doPutRequest(destination);
      return iotivity.OCStackApplicationResult.
        OC_STACK_DELETE_TRANSACTION;
    },
    null
  );
  assert.equal(result, ok, "OCDoResource(get)" + msg);
}

result = iotivity.OCInit(null, 0, iotivity.OCMode.OC_CLIENT_SERVER);
assert.equal(result, ok, "OCInit" + msg);

processLoop = setInterval(function () {
  var processResult = iotivity.OCProcess();
  assert.equal(processResult, ok, "OCProcess" + msg);
  if (done === true) {
    setTimeout(function() {
      process.exit(0);
    }, 500);
    done = false;
  }
}, 100);

var requestSequenceNumber = 0,
  requestSequence = [
    function (flag, request) {
      if (request.method !== iotivity.OCMethod.OC_REST_GET) {
        process.exit(1);
      }
      result = iotivity.OCDoResponse({
        requestHandle: request.requestHandle,
        resourceHandle: request.resource,
        resourceUri: "/a/" + uuid,
        ehResult: iotivity.OCEntityHandlerResult.OC_EH_OK,
        payload: {
          type: iotivity.OCPayloadType.PAYLOAD_TYPE_REPRESENTATION,
          values: {
            primitiveValue: 44,
            objectValue: {
              type: iotivity.OCPayloadType.PAYLOAD_TYPE_REPRESENTATION,
              values: {
                childValue: 74
              }
            },
            arrayValue: [19, 23, 7]
          }
        },
        sendVendorSpecificHeaderOptions: []
      });
      assert.equal(result, ok, "OCDoResponse(get)" + msg);

      return iotivity.OCEntityHandlerResult.OC_EH_OK;
    },

    function(flag, request) {
      if (request.method !== iotivity.OCMethod.OC_REST_PUT) {
        process.exit(1);
      }

      if (t.payloadAssert(request.payload, compareObject2)) {
        assert.equal(false, true, "Wrong payload is received");
      }
      result = iotivity.OCDoResponse({
        requestHandle: request.requestHandle,
        resourceHandle: request.resource,
        resourceUri: "/a/" + uuid,
        ehResult: iotivity.OCEntityHandlerResult.OC_EH_OK,
        payload: null,
        sendVendorSpecificHeaderOptions: []
      });
      assert.equal(result, ok, "OCDoResponse(put) + msg");

      return iotivity.OCEntityHandlerResult.OC_EH_OK;
    }
  ];

result = iotivity.OCCreateResource(
  resourceHandleReceptacle,
  "core.light",
  iotivity.OC_RSRVD_INTERFACE_DEFAULT,
  "/a/" + uuid,
  function(flag, request) {
    return requestSequence[requestSequenceNumber++](flag, request);
  },
  iotivity.OCResourceProperty.OC_DISCOVERABLE
);
assert.equal(result, ok, "OCCreateResource" + msg);

result = iotivity.OCDoResource(
  discoverHandleReceptacle,
  iotivity.OCMethod.OC_REST_DISCOVER,
  iotivity.OC_MULTICAST_DISCOVERY_URI,
  null,
  null,
  iotivity.OCConnectivityType.CT_DEFAULT,
  iotivity.OCQualityOfService.OC_HIGH_QOS,
  function(handle, response) {
    var returnValue =
      iotivity.OCStackApplicationResult.OC_STACK_KEEP_TRANSACTION;

    assert.equal(t.findResource(response, uuid), true,
      "Client did not found resource");

    if (t.findResource(response, uuid)) {
      returnValue =
        iotivity.OCStackApplicationResult.OC_STACK_DELETE_TRANSACTION;
      doGetRequest(response.addr);
    }

    return returnValue;
  },
  null
);
assert.equal(result, ok, "OCDoResource" + msg);

process.on('exit', function(code) {
  clearInterval(processLoop);
  processLoop = null;
  result = iotivity.OCDeleteResource(resourceHandleReceptacle.handle);
  assert.equal(result, ok, "OCDeleteResource(server)" + msg);
  assert.equal(iotivity.OCStop(), ok, "OCStop" + msg);
});
