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
  result, countReceptacle, initialResourceCount,
  childResourceHandleReceptacle = {},
  resourceHandleReceptacle = {},
  t = require('tools/iotivity-utils.js'),
  uuid = "test",
  iotivity = require('iotivity'),
  msg = " should return OC_STACK_OK",
  ok = iotivity.OCStackResult.OC_STACK_OK,
  initialEntityHandler = function () {
    return iotivity.OCEntityHandlerResult.OC_EH_ERROR;
  },
  alternateEntityHandler = function () {
    return iotivity.OCEntityHandlerResult.OC_EH_ERROR;
  };

var result;

result = iotivity.OCRegisterPersistentStorageHandler(t.persistent);
assert.equal(result, ok, "OCRegisterPersistentStorageHandler" + msg);

result = iotivity.OCInit(null, 0, iotivity.OCMode.OC_SERVER);
assert.equal(result, ok);

result = iotivity.OCSetDeviceInfo({
  deviceName: "resource-operations-" + uuid,
  specVersion: "ver.0.0.0",
  dataModelVersions: ["test.1.1.1"],
  types: []
});
assert.equal(result, ok, "OCSetDeviceInfo" + msg);

result = iotivity.OCSetPlatformInfo({
  platformID: "server." + uuid,
  manufacturerName: "resource-ops"
});
assert.equal(result, ok, "OCSetPlatformInfo" + msg);

countReceptacle = {};
result = iotivity.OCGetNumberOfResources(countReceptacle);
assert.equal(result, ok, "OCGetNumberOfResources" + msg);

initialResourceCount = countReceptacle.count;

result = iotivity.OCCreateResource(
  resourceHandleReceptacle,
  "core.fan",
  iotivity.OC_RSRVD_INTERFACE_DEFAULT,
  "/a/" + uuid,
  initialEntityHandler,
  iotivity.OCResourceProperty.OC_SLOW |
  iotivity.OCResourceProperty.OC_ACTIVE
);
assert.equal(result, ok, "OCCreateResource" + msg);

countReceptacle = {};
result = iotivity.OCGetNumberOfResources(countReceptacle);
assert.equal(result, ok, "OCGetNumberOfResources" + msg);
assert.strictEqual(countReceptacle.count, initialResourceCount + 1,
  "OCGetNumberOfResources should resports an increase by one");

var resultObject = {};
resultObject = iotivity.OCGetResourceHandler(resourceHandleReceptacle.handle);
assert.strictEqual(resultObject, initialEntityHandler,
  "OCGetResourceHandler should return initialEntityHandler");

result = iotivity.OCBindResourceHandler(resourceHandleReceptacle.handle,
  alternateEntityHandler);
assert.equal(result, ok, "OCBindResourceHandler" + msg);

resultObject = iotivity.OCGetResourceHandler(resourceHandleReceptacle.handle);
assert.strictEqual(resultObject, alternateEntityHandler,
  "OCGetResourceHandler" + msg);

result = iotivity.OCGetResourceUri(resourceHandleReceptacle.handle);
assert.strictEqual(result, "/a/" + uuid,
  'OCGetResourceUri should return /a/' + uuid);

result = iotivity.OCGetServerInstanceIDString();

countReceptacle = {};
result = iotivity.OCGetNumberOfResourceInterfaces(
  resourceHandleReceptacle.handle, countReceptacle)
assert.equal(result, ok, "OCGetNumberOfResourceInterfaces" + msg);
assert.equal(countReceptacle.count, 1,
  "OCGetNumberOfResourceInterfaces should give 1 interface");

result = iotivity.OCBindResourceInterfaceToResource(
  resourceHandleReceptacle.handle, "a.b.c");
assert.equal(result, ok, "OCBindResourceInterfaceToResource" + msg);

countReceptacle = {};
result = iotivity.OCGetNumberOfResourceInterfaces(
  resourceHandleReceptacle.handle, countReceptacle);
assert.equal(result, ok, "OCGetNumberOfResourceInterfaces" + msg);
assert.equal(countReceptacle.count, 2,
  "OCGetNumberOFResourceInterfaces should give 2 interfaces");

result = iotivity.OCGetResourceInterfaceName(
  resourceHandleReceptacle.handle, 0);
assert.strictEqual(result, iotivity.OC_RSRVD_INTERFACE_DEFAULT,
  "OCGetResourceInterfaceName should return first interface: " +
  iotivity.OC_RSRVD_INTERFACE_DEFAULT);

result = iotivity.OCGetResourceInterfaceName(
  resourceHandleReceptacle.handle, 1);
assert.strictEqual(result, "a.b.c",
  "OCGetResourceInterfaceName should return second interface: a.b.c");

result = iotivity.OCBindResourceTypeToResource(resourceHandleReceptacle.handle,
  "core.led");
assert.equal(result, ok, "OCBindResourceTypeToResource" + msg);

countReceptacle = {};
result = iotivity.OCGetNumberOfResourceTypes(resourceHandleReceptacle.handle,
  countReceptacle);
assert.equal(result, ok, "OCGetNumberOfResourceTypes" + msg);
assert.equal(countReceptacle.count, 2,
  "OCGetNumberOfResourceTypes should give 2 types");

result = iotivity.OCGetResourceTypeName(resourceHandleReceptacle.handle, 0);
assert.strictEqual(result, "core.fan",
  "OCGetResourceTypeName should return core.fan");

result = iotivity.OCGetResourceTypeName(resourceHandleReceptacle.handle, 1);
assert.strictEqual(result, "core.led",
  "OCGetResourceTypeName should return core.led");

result = t.checkBitFields(iotivity.OCResourceProperty,
  iotivity.OCGetResourceProperties(resourceHandleReceptacle.handle));
assert.strictEqual(result.OC_ACTIVE, true,
  "OCGetResourceProperties should returns propierties set on resource");
assert.strictEqual(result.OC_SLOW, true,
  "OCGetResourceProperties should returns propierties set on resource");

resultObject = {};
resultObject = iotivity.OCGetResourceHandle(initialResourceCount)
assert.strictEqual(resultObject, resourceHandleReceptacle.handle,
  "OCGetResourceHandle should return first resource handle" + msg);

result = iotivity.OCCreateResource(
  childResourceHandleReceptacle,
  "core.fan",
  iotivity.OC_RSRVD_INTERFACE_DEFAULT,
  "/a/" + uuid + "-child",
  initialEntityHandler,
  iotivity.OCResourceProperty.OC_SLOW |
  iotivity.OCResourceProperty.OC_ACTIVE
);
assert.equal(result, ok, "OCCreateResource" + msg);

result = iotivity.OCGetResourceHandleFromCollection(
  resourceHandleReceptacle.handle, 0);
assert.strictEqual(result, null,
  "OCGetResourceHandleFromCollection should return null");

result = iotivity.OCBindResource(
  resourceHandleReceptacle.handle, childResourceHandleReceptacle.handle);
assert.equal(result, ok, "OCBindResource" + msg);

resultObject = {};
resultObject = iotivity.OCGetResourceHandleFromCollection(
  resourceHandleReceptacle.handle, 0);
assert.strictEqual(resultObject, childResourceHandleReceptacle.handle,
  "OCGetResourceHandleFromCollection should return child resource handle");

result = iotivity.OCUnBindResource(
  resourceHandleReceptacle.handle, childResourceHandleReceptacle.handle);
assert.equal(result, ok, "OCUnBindResource" + msg);

result = iotivity.OCGetResourceHandleFromCollection(
  resourceHandleReceptacle.handle, 0);
assert.strictEqual(result, null,
  "OCGetResourceHandleFromCollection Should return null handler after " +
  "having detached the child from the parent");

var lastResourceHandleReceptacle = {};
result = iotivity.OCCreateResource(
  lastResourceHandleReceptacle,
  "core.engine",
  iotivity.OC_RSRVD_INTERFACE_DEFAULT,
  "/a/" + uuid + "-engine",
  initialEntityHandler,
  iotivity.OCResourceProperty.OC_SLOW |
  iotivity.OCResourceProperty.OC_ACTIVE
);
assert.equal(result, ok, "OCCreateResource" + msg);

result = iotivity.OCDeleteResource(childResourceHandleReceptacle.handle);
assert.equal(result, ok, "OCDeleteResource" + msg);
assert.strictEqual(childResourceHandleReceptacle.handle.stale, true,
  "Child resource should be marked as deleted");

result = iotivity.OCDeleteResource(resourceHandleReceptacle.handle);
assert.equal(result, ok, "OCDeleteResource" + msg);
assert.strictEqual(resourceHandleReceptacle.handle.stale, true,
  "Parent resource should be marked as deleted");

result = iotivity.OCDeleteResource(lastResourceHandleReceptacle.handle);
assert.equal(result, ok, "OCDeleteResource" + msg);

t.persistent.clean();
result = iotivity.OCStop();
assert.equal(result, ok, "OCStop" + msg);
