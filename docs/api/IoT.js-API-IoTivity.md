### Platform Support

The following shows `IoTivity` module APIs available for each platform.

|  | Linux<br/>(Ubuntu) | Raspbian<br/>(Raspberry Pi) | NuttX<br/>(STM32F4-Discovery) |
| :---: | :---: | :---: | :---: |
| iotivity.OCInit | O | O | x |
| iotivity.OCProcess | O | O | x |
| iotivity.OCSetDeviceInfo | O | O | x |
| iotivity.OCPlatformInfo | O | O | x |
| iotivity.OCCreateResource | O | O | x |
| iotivity.OCDeleteResource | O | O | x |
| iotivity.OCStop | O | O | x |
| iotivity.OCDoResource | O | O | x |
| iotivity.OCCancel | O | O | x |
| iotivity.OCGetServerInstanceIDString | O | O | x |
| iotivity.OCGetNumberOfResources | O | O | x |
| iotivity.OCDoResponse | O | O | x |
| iotivity.OCNotifyAllObservers | O | O | x |
| iotivity.OCNotifyListOfObservers | O | O | x |
| iotivity.OCStartPresence | O | O | x |
| iotivity.OCStopPresence | O | O | x |
| iotivity.OCRegisterPersistentStorageHandler | O | O | x |
| iotivity.OCGetNumberOfResourceInterfaces | O | O | x |
| iotivity.OCGetNumberOfResourceTypes | O | O | x |
| iotivity.OCGetResourceHandle | O | O | x |
| iotivity.OCGetResourceHandleFromCollection | O | O | x |
| iotivity.OCGetResourceInterfaceName | O | O | x |
| iotivity.OCGetResourceTypeName | O | O | x |
| iotivity.OCGetResourceProperties | O | O | x |
| iotivity.OCGetResourceUri | O | O | x |
| iotivity.OCGetResourceHandler | O | O | x |
| iotivity.OCBindResourceHandler | O | O | x |
| iotivity.OCBindResourceInterfaceToResource | O | O | x |
| iotivity.OCBindResourceTypeToResource | O | O | x |
| iotivity.OCBindResource | O | O | x |
| iotivity.OCUnBindResource | O | O | x |
| iotivity.OCSetDefaultDeviceEntityHandler | O | O | x |

# IoTivity

The IoTivity module provides access to IoTivity v1.2.1 C Api. You can use this module with
`require('iotivity')` and create both servers and clients.


### Enumerations

All enumerations are the same as in [IoTivity C Api](https://api-docs.iotivity.org/latest-c/index.html)

### OCPayload
A generic object representing a payload returned from a resource operation.
Used, by user, only `OCRepPayload` in few methods. Others are used only as responses from OC Stack.

* `type` {iotivity.OCPayloadType} The type of message that is sent/received.

## OCRepPayload
* `uri` {string} URI of the resource.
* `types` {Array} Resource types - an array of strings.
* `interfaces` {Array} Resource interfaces - an array of strings.
* `values` {Object} Object which specifies sent/received values.
* `next` {Object} Next OCRepPayload object.


### OCDevAddr
Data structure to encapsulate IPv4/IPv6/Contiki/IwIP device addresses.

* `adapter` {iotivity.OCTransportAdapter} Adapter type.
* `flags` {iotivity.OCTransportFlags} Transport modifiers.
* `port` {number} For IP.
* `addr` {string} Address for all adapters.
* `ifindex` {number} Usually zero for default interface.
* `routeData` {string} Destination GatewayID:ClientId.
* `remoteId` {string} Device ID of remote.

### iotivity.OCInit(ipAddress, port, mode)
* `ipAddress` {string} IP Address of host device (required).
* `port` {number} Port of host device (required).
* `mode` {iotivity.OCMode} An object which specifies mode of host.
* Returns: {number} [OC_STACK_OK](https://api-docs.iotivity.org/latest-c/octypes_8h.html#a8fc7163f720a1c3eb9beecb037a2c245acd9b39499308a6c2b1366d747341d4fa) on success, some other value upon failure.

This function initializes the OC Stack.
Must be called prior to starting the stack.

**Example**

```js
var iotivity = require('iotivity');

iotivity.OCInit("iotivity", 0, iotivity.OCMode.OCServer);
```

### iotivity.OCProcess()
* Returns: {number} `OC_STACK_OK` on success, some other value upon failure.

This function is called in main loop of OC client or server.
Allows low-level processing of stack services.

**Example**

```js
setTimeout(function() {
  iotivity.OCProcess();
}, 500);
```

### iotivity.OCSetDeviceInfo(deviceInfo)
* `deviceInfo` {Object} An object which specifies the device info.
* Returns: {number} `OC_STACK_OK` on success, some other value upon failure.

The `deviceInfo` object specifies the following information:
* `deviceName` {string} Device name (required).
* `dataModelVersions` {Array} Device data model versions - array of strings (required).
* `types` {Array} Device types - array of strings.
* `specVersion` {string} Device specification version.


**Example**

```js
iotivity.OCSetDeviceInfo({
  deviceName: "test-server",
  specVersion: "v1.0.0",
  dataModelVersions: ["test.1.1.1"],
  types:[]
});
```

### iotivity.OCSetPlatformInfo(platformInfo)
* `platformInfo` {Object} An object which specifies the platform info.
* Returns: {number} `OC_STACK_OK` on success, some other value upon failure.

The `platformInfo` object specifies the following information:
* `platformID` {string} Platform ID (required).
* `manufacturerName` {string} Manufacturer name (required).
* Rest of platformInfo attributes (optional) are also {string} - [API](https://api-docs.iotivity.org/latest-c/struct_o_c_platform_info.html)

**Example**

```js
iotivity.OCSetPlatformInfo({
  platformID: "server.test",
  manufacturerName: "manufacturer"
});
```

### iotivity.OCCreateResource(handle, typeName, ifaceName, uri, callback, mode)
* `handle` {Object} Handle to newly created resource (required). If function returns `OC_STACK_OK` handle to resource will be at `handle.handle`.
* `typeName` {string} Name of resource type (required).
* `ifaceName` {string} Name of resource interface (required).
* `uri` {string} Uri of the resource (required).
* `callback` {Function} Callback that is called by ocstack to handle requests, etc. (required). Callback returns {iotivity.OCEntityHandlerResult}.
* `mode` {iotivity.OCResourceProperty} An object which specifies properties supported by resource (required).
* Returns: {number} `OC_STACK_OK` on success, some other value upon failure.

**Example**

```js
iotivity.OCCreateResource(
  resourceHandle,
  "core.fan",
  iotivity.OC_RSRVD_INTERFACE_DEFAULT,
  "/a/led",
  function(flag, request) {
    console.log(JSON.stringify(request, null, 4));
    return iotivity.OCEntityHandlerResult.OC_EH_ERROR;
  },
  iotivity.OCResourceProperty.OC_DISCOVERABLE |
  iotivity.OCResourceProperty.OC_OBSERVABLE
);
```

### iotivity.OCDeleteResource(handle)
* `handle` {Object} Handle of resource to be deleted.
* Returns: {number} `OC_STACK_OK` on success, some other value upon failure.

This function deletes resource specified by handle.

**Example**

```js
iotivity.OCDeleteResource(resourceHandle.handle);
```

### iotivity.OCStop()
* Returns: {number} `OC_STACK_OK` on success, some other value upon failure.

This function stops the OC stack.

**Example**

```js
iotivity.OCStop();
```

### iotivity.OCDoResource(handle, method, requestUri, destination, payload, connectivityType, qos, callback, options)
* `handle` {Object} Handle to refer to the request sent out on behalf of calling this API. This handle can be used to cancel this operation via the `OCCancel` API.
* `method` {iotivity.OCMethod} To perform on the resource.
* `requestUri` {string} URI of the resource to interact with.
* `destination` {iotivity.OCDevAddr} Complete description of destination.
* `payload` {iotivity.OCPayload} Encoded request payload.
* `connectivityType` {iotivity.OCConnectivityType} Modifier flags when destination is not given.
* `qos` {iotivity.OCQualityOfService} Quality of service.
* `callback` {Function} Asynchronous callback function that is invoked by the stack when discovery or resource interaction is received.
* `options` {Array} An array containing the vendor specific header options to be sent with the request.
* Returns: {number} `OC_STACK_OK` on success, some other value upon failure.

This function discovers or Perform requests on a specified resource (specified by that Resource's respective URI).

**Example**

```js
iotivity.OCDoResource(
  discoverHandle,
  iotivity.OCMethod.OC_REST_DISCOVER,
  iotivity.OC_MULTICAST_DISCOVERY_URI,
  null,
  null,
  iotivity.OCConnectivityType.CT_DEFAULT,
  iotivity.OCQualityOfService.OC_HIGH_QOS,
  function(handle, response) {
    console.log(JSON.stringify(response, null, 4));
    return iotivity.OCStackApplicationResult.OC_DELETE_TRANSACTION;
  },
  null
);
```

### iotivity.OCCancel(handle, qos, options)
* `handle` {Object} Used to identify a specific OCDoResource invocation.
* `qos` {iotivity.OCQualityOfService} Used to specify Quality of Service.
* `options` {Array} An array containing the vendor specific header options to be sent with the request.
* Returns: {number} `OC_STACK_OK` on success, some other value upon failure.

This function cancels a request associated with a specific `OCDoResource` invocation.

**Example**

```js
iotivity.OCCancel(discoverHandle.handle, iotivity.OCQualityOfService.OC_HIGH_QOS, []);
```

### iotivity.OCGetServerInstanceIDString()
* Returns: {string} A string representation the server instance ID.

Get a string representation the server instance ID.

**Example**

```js
var id = iotivity.OCGetServerInstanceIDString();
```

### iotivity.OCGetNumberOfResources(numOfResources)
* `numofResources` {Object} Object which will hold at `count` attribute count variable.
* Returns: {number} `OC_STACK_OK` on success, some other value upon failure.

This function gets the number of resources that have been created in the stack.

**Example**

```js
var numOfResources = {};
iotivity.OCGetNumberOfResources(numOfResources);
```

### iotivity.OCDoResponse(response)
* `response` Object that contains response parameters.
* Returns: {number} `OC_STACK_OK` on success, some other value upon failure.

The `response` object specifies the following information:
* `requestHandle` {Object} Request handle.
* `resourceHandle` {Object} Resource handle.
* `manufacturerName` {Object} Allow the entity handler to pass a result with the response.
* `payload` {Object} Payload object.
* `resourceUri` {string} URI of new resource that entity handler might create.
* `sendVendorSpecificHeaderOptions` {Array} An array of the vendor specific header options the entity handler wishes to use in response..

This function sends a response to a request.

**Example**

```js
result = iotivity.OCCreateResource(
  resourceHandleReceptacle,
  "core.fan",
  iotivity.OC_RSRVD_INTERFACE_DEFAULT,
  "/a/fan",
  function (flag, request) {
    iotivity.OCDoResponse({
      requestHandle: request.requestHandle,
      resourceHandle: request.resource,
      ehResult: iotivity.OCEntityHandlerResult.OC_EH_OK,
      payload: {
        type: iotivity.OCPayloadType.PAYLOAD_TYPE_REPRESENTATION,
        values: {
          power: 0
        }
      },
      resourceUri: "/a/fan",
      sendVendorSpecificHeaderOptions: []
    });
    return iotivity.OCEntityHandlerResult.OC_EH_OK;
  },
  iotivity.OCResourceProperty.OC_DISCOVERABLE |
  iotivity.OCResourceProperty.OC_OBSERVABLE
);

```

### iotivity.OCNotifyAllObservers(handle, qos)
* `handle` {Object} handle of resource.
* `qos` {iotivity.OCQualityOfService} 	Desired quality of service for the observation notifications.
* Returns: {number} `OC_STACK_OK` on success, some other value upon failure.

This function notify all registered observers that the resource representation has changed.

**Example**

```js
iotivity.OCNotifyAllObservers(handleResource.handle, iotivity.OCQualityOfService.OC_LOW_QOS);
```

### iotivity.OCNotifyListOfObservers(handle, obsidList, payload, qos)
* `handle` {Object} Handle of resource.
* `obsidList` {Array} List of observation IDs that need to be notified.
* `payload` {Object} Object representing the notification.
* `qos` {iotivity.OCQualityOfService} 	Desired quality of service for the observation notifications.
* Returns: {number} `OC_STACK_OK` on success, some other value upon failure.

Notify specific observers with updated value of representation.

**Example**

```js
iotivity.OCNotifyListOfObservers(
  resourceHandle.handle,
  listOfObservers,
  {
    type: iotivity.OCPayloadType.PAYLOAD_TYPE_REPRESENTATION,
    values: { observedValue: "notify!" }
  },
  iotivity.OCQualityOfService.OC_HIGH_QOS
);
```

### iotivity.OCStartPresence(ttl)
* `ttl` {number} Time to live.
* Returns: {number} `OC_STACK_OK` on success, some other value upon failure.

The server starts the presence advertisement with TTL (time to live) in seconds

**Example**

```js
iotivity.OCStartPresence(30);
```

### iotivity.OCStopPresence()
* Returns: {number} `OC_STACK_OK` on success, some other value upon failure.

Stops presence.

**Example**

```js
iotivity.OCStopPresence();
```

### iotivity.OCRegisterPersistentStorageHandler(persistentStorageHandler)
* `persistentStorageHandler` {Object} Open, read, write, close & unlink handlers.
* Returns: {number} `OC_STACK_OK` on success, some other value upon failure.

**Example**

```js
var persistent = {
  open: function(filename, mode) {
    var fd;
    fd = fs.openSync(filename, mode);
    fd = (fd === undefined ? -1 : fd);
    return fd;
  },
  close: function(fp) {
    fs.closeSync(fp);
    return 0;
  },
  read: function(buffer, totalSize, fp) {
    var result = fs.readSync(fp, buffer, 0, totalSize, null);
    return result;
  },
  write: function(buffer, totalSize, fp) {
    return fs.writeSync(fp, buffer, 0, totalSize, null);
  },
  unlink: function(path) {
    fs.unlinkSync(path);
    return 0;
  },
};

iotivity.OCRegisterPersistentStorageHandler(persistent);
```

### iotivity.OCGetNumberOfResourceInterfaces(handle, numOfResources)
* `handle` {Object} Handle of resource.
* `numOfResources` {Object} Object which will hold at `count` attribute count variable.
* Returns: {number} `OC_STACK_OK` on success, some other value upon failure.

This function gets the number of resource interfaces of the resource.

**Example**

```js
numOfResources = {};
result = iotivity.OCGetNumberOfResourceInterfaces(resourceHandle.handle, numOfResources);
```

### iotivity.OCGetNumberOfResourceTypes(handle, numOfResources)
* `handle` {Object} Handle of resource.
* `numOfResources` {Object} Object which will hold at `count` attribute count variable.
* Returns: {number} `OC_STACK_OK` on success, some other value upon failure.

This function gets the number of resource types of the resource.

**Example**

```js
numOfResources = {};
result = iotivity.OCGetNumberOfResourceTypes(resourceHandle.handle, numOfResources);
```

### iotivity.OCGetResourceHandle(index)
* `index` {number} Index of resource.
* Returns: {Object} Found resource handle or NULL if not found.

**Example**

```js
var resultObject = {};
resultObject = iotivity.OCGetResourceHandle(0);
```

### iotivity.OCGetResourceHandleFromCollection(handle, index)
* `handle` {Object} Handle of collection resource.
* `index` {number} Index of contained resource.
* Returns: {Object} Handle to contained resource if resource found or NULL if resource not found.

**Example**

```js
iotivity.OCGetResourceHandleFromCollection(resourceHandle.handle, 0);
```

### iotivity.OCGetResourceInterfaceName(handle, index)
* `handle` {Object} Handle of resource.
* `index` {number} Index of resource.
* Returns: {string} Resource interface name if resource found or NULL if resource not found.

**Example**

```js
result = iotivity.OCGetResourceInterfaceName(resourceHandle.handle, 0);
```

### iotivity.OCGetResourceTypeName(handle, index)
* `handle` {Object} Handle of resource.
* `index` {number} Index of resource.
* Returns: {string} Resource type name if resource found or NULL if resource not found.

This function gets name of resource type of the resource.

**Example**

```js
result = iotivity.OCGetResourceTypeName(resourceHandle.handle, 0);
```

### iotivity.OCGetResourceProperties(handle)
* `handle` {Object} Handle of resource.
* Returns: {number} OCResourceProperty Bitmask or -1 if resource is not found.

This function gets the properties of the resource specified by handle.

**Example**

```js
iotivity.OCGetResourceProperties(resourceHandle.handle));
```

### iotivity.OCGetResourceUri(handle)
* `handle` {Object} Handle of resource.
* Returns: {string} URI string if resource found or NULL if not found.

This function gets the URI of the resource specified by handle.

**Example**

```js
result = iotivity.OCGetResourceUri(resourceHandle.handle);
```

### iotivity.OCGetResourceHandler(handle)
* `handle` {Object} Handle of resource.
* Returns: {Object} Entity handler if resource found or NULL resource not found.

This function gets the entity handler for a resource.

**Example**

```js
var resultObject = {};
resultObject = iotivity.OCGetResourceHandler(resourceHandle.handle);
```

### iotivity.OCBindResourceHandler(handle, callback)
* `handle` {Object} Handle to the resource that the contained resource is to be bound.
* `callback` {Function} Entity handler function that is called by ocstack to handle requests.
* Returns: {number} `OC_STACK_OK` on success, some other value upon failure.

This function binds an entity handler to the resource.

**Example**

```js
result = iotivity.OCBindResourceHandler(resourceHandle.handle, alternateEntityHandler);
```

### iotivity.OCBindResourceInterfaceToResource(handle, resourceIfaceName)
* `handle` {Object} Handle to the resource.
* `resourceIfaceName` {string} Name of resource interface.
* Returns: {number} `OC_STACK_OK` on success, some other value upon failure.

This function binds a resource interface to a resource.

**Example**

```js
iotivity.OCBindResourceInterfaceToResource(resourceHandle.handle, "core.rw");
```

### iotivity.OCBindResourceTypeToResource(handle, resourceTypeName)
* `handle` {Object} Handle to the resource.
* `resourceTypeName` {string} Name of resource type.
* Returns: {number} `OC_STACK_OK` on success, some other value upon failure.

This function binds a resource type to a resource.

**Example**

```js
iotivity.OCBindResourceTypeToResource(resourceHandle.handle, "core.led");
```

### iotivity.OCBindResource(collectionHandle, resourceHandle)
* `collectionHandle` {Object} Handle to the collection resource.
* `resourceHandle` {Object} Handle to resource to be added to the collection resource.
* Returns: {number} `OC_STACK_OK` on success, some other value upon failure.

This function adds a resource to a collection resource.

**Example**

```js
iotivity.OCBindResource(resourceHandle.handle, childResourceHandle.handle);
```

### iotivity.OCUnBindResource(collectionHandle, resourceHandle)
* `collectionHandle` {Object} Handle to the collection resource.
* `resourceHandle` {Object} Handle to resource to be removed from the collection resource.
* Returns: {number} `OC_STACK_OK` on success, some other value upon failure.

This function removes a resource from a collection resource.

**Example**

```js
iotivity.OCUnBindResource(resourceHandle.handle, childResourceHandle.handle);
```

### iotivity.OCSetDefaultDeviceEntityHandler(callback)
* `callback` {Object} Entity handler function that is called by ocstack to handle requests for any undefined resources or default actions.If NULL is passed it removes the device default entity handler.
* Returns: {number} `OC_STACK_OK` on success, some other value upon failure.

This function sets default device entity handler.

**Example**

```js
iotivity.OCSetDefaultDeviceEntityHandler(
  function(flag, request, uri) {
    if (request && request.payload && request.payload.type ===
      iotivity.OCPayloadType.PAYLOAD_TYPE_REPRESENTATION &&
      request.payload.values && request.payload.values.question) {

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
    }
    return iotivity.OCEntityHandlerResult.OC_EH_OK;
  }
);
```

### More examples are in samples/iotivity/