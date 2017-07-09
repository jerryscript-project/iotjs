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

var result, notifyObserversTimeoutId = null,
  uuid = "test",
  notificationCount = 0,
  processLoop = null,
  observeCount = 0,
  firstResponse = true,
  listOfObservers = [],
  resourceHandleReceptacle = {},
  discoverHandleReceptacle = {},
  done = false,
  assert = require('assert'),
  t = require('tools/iotivity-utils.js'),
  iotivity = require('iotivity'),
  ok = iotivity.OCStackResult.OC_STACK_OK,
  msg = " should return OC_STACK_OK",
  result;

function doObserveRequest(destination) {
  var observeHandleReceptacle = {};
  result = iotivity.OCDoResource(
    observeHandleReceptacle,
    iotivity.OCMethod.OC_REST_OBSERVE,
    "/a/" + uuid,
    destination,
    null,
    iotivity.OCConnectivityType.CT_DEFAULT,
    iotivity.OCQualityOfService.OC_HIGH_QOS,
    function(handle, response) {
      if (firstResponse) {
        assert.equal(response.payload.values.message, "Want to rock",
          "Payload message should be \'Want to rock\'");
        firstResponse = false;
        observeCount--;
      }
      else if (observeCount  < 10) {
        assert.equal(response.payload.type,
          iotivity.OCPayloadType.PAYLOAD_TYPE_REPRESENTATION,
          "Observed value is not as expected");
        assert.equal(response.payload.values.observedValue,
          uuid + '-' + observeCount, "Observed value is not as expected");
      }
      observeCount++;
      if (observeCount >= 10) {
        result = iotivity.OCCancel(handle,
          iotivity.OCQualityOfService.OC_HIGH_QOS, []);
        assert.equal(result, ok, "OCCancel" + msg);
        done = true;
        return iotivity.OCStackApplicationResult.OC_STACK_DELETE_TRANSACTION;
      }
      return iotivity.OCStackApplicationResult.OC_STACK_KEEP_TRANSACTION;
    },
    null
  );
  assert.equal(result, ok, "OCDoResource(observe)" + msg);
}

result = iotivity.OCInit(null, 0, iotivity.OCMode.OC_CLIENT_SERVER);
assert.equal(result, ok, "OCInit" + msg);

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

function notifyObservers() {
  var notificationResult;
  if (notificationCount < 10) {
    if (resourceHandleReceptacle.handle && listOfObservers.length > 0) {
      notificationResult = iotivity.OCNotifyListOfObservers(
        resourceHandleReceptacle.handle,
        listOfObservers,
        {
          type: iotivity.OCPayloadType.PAYLOAD_TYPE_REPRESENTATION,
          values: { observedValue: uuid + "-" + notificationCount }
        },
        iotivity.OCQualityOfService.OC_HIGH_QOS
      );

      if (notificationResult === ok) {
        notificationCount++;
      }
      else {
        assert.equal(notificationResult, ok, "OCNotifyListOfObservers" + msg);
      }
    }
    notifyObserversTimeoutId = setTimeout(notifyObservers, 1000);
  }
}

result = iotivity.OCCreateResource(
  resourceHandleReceptacle,
  "core.fan",
  iotivity.OC_RSRVD_INTERFACE_DEFAULT,
  "/a/" + uuid,
  callback = function(flag, request) {
    var responseResult;
    if (notificationCount >= 10) {
      return iotivity.OCEntityHandlerResult.OC_EH_ERROR;
    }
    if (flag & iotivity.OCEntityHandlerFlag.OC_OBSERVE_FLAG) {
      assert.notEqual(request.obsInfo.obsId, 0,
        "Observe request observation ID is not zero");

      assert.equal(request.obsInfo.action,
        iotivity.OCObserveAction.OC_OBSERVE_REGISTER,
        "OCObserverAction should be OC_OBSERVE_REGISTER");

      listOfObservers.push(request.obsInfo.obsId);

      if (listOfObservers.length > 1) {
        assert.equal(true, false, "Too many obsIds encountered(" +
          listOfObservers.length + ")");
        process.exit(1);
      }

      result = iotivity.OCDoResponse({
        requestHandle: request.requestHandle,
        resourceHandle: request.resource,
        ehResult: iotivity.OCEntityHandlerResult.OC_EH_OK,
        payload: {
          type: iotivity.OCPayloadType.PAYLOAD_TYPE_REPRESENTATION,
          values: {
            message: "Want to rock"
          }
        },
        resourceUri: "/a/" + uuid,
        sendVendorSpecificHeaderOptions: []
      });
      assert.equal(result, ok, "OCDoResponse" + msg);
      if (listOfObservers.length === 1) {
        setTimeout(notifyObservers, 200);
      }
    }
    return iotivity.OCEntityHandlerResult.OC_EH_OK;
  },
  iotivity.OCResourceProperty.OC_DISCOVERABLE |
  iotivity.OCResourceProperty.OC_OBSERVABLE
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
  function(handle, response) {
    var returnValue =
      iotivity.OCStackApplicationResult.OC_STACK_KEEP_TRANSACTION;

    assert.equal(t.findResource(response, uuid), true,
      "Client should find resource");

    if (t.findResource(response, uuid)) {
      doObserveRequest(response.addr);
      returnValue =
        iotivity.OCStackApplicationResult.OC_STACK_DELETE_TRANSACTION;
    }
    return returnValue;
  },
  null
);
assert.equal(result, ok, "OCDoResource(discovery)" + msg);

process.on('exit', function(code) {
  clearInterval(processLoop);
  processLoop = null;
  assert.equal(code, 0, "Exit should be succesful");
  result = iotivity.OCDeleteResource(resourceHandleReceptacle.handle);
  assert.equal(result, ok, "OCDeleteResource(server)" + msg);
  assert.equal(iotivity.OCStop(), ok, "OCStop" + msg);
});
