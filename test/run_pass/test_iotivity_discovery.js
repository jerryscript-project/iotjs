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
  discoverHandleReceptacle = {},
  resourceHandleReceptacle = {},
  msg = " should return OC_STACK_OK",
  done = false,
  result;

var iotivity = require('iotivity');
var ok = iotivity.OCStackResult.OC_STACK_OK;

result = iotivity.OCInit(null, 0, iotivity.OCMode.OC_CLIENT_SERVER);
assert.equal(result, ok, "OCInit" + msg);

processLoop = setInterval(function () {
  var processResult = iotivity.OCProcess();
  assert.equal(processResult, ok, "OCProcess" + msg);
  if (done) {
    setTimeout(function() {
      process.exit(0);
    }, 500);
  }
}, 100);

result = iotivity.OCCreateResource(
  resourceHandleReceptacle,
  "core.fan",
  iotivity.OC_RSRVD_INTERFACE_DEFAULT,
  "/a/" + uuid,
  callback = function () {

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
    assert.equal(t.findResource(response, uuid), true,
      "Client did not find resource");
    done = true;
    return iotivity.OCStackApplicationResult.OC_STACK_DELETE_TRANSACTION;
  },
  null
);
assert.equal(result, ok, "OCDoResource(client)" + msg);

process.on('exit', function(code) {
  clearInterval(processLoop);
  processLoop = null;
  result = iotivity.OCDeleteResource(resourceHandleReceptacle.handle);
  assert.equal(result, ok, 'OCDeleteResource(server)' + msg);
  assert.equal(iotivity.OCStop(), ok, 'OCStop' + msg);
});
