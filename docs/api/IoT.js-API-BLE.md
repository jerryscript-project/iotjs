### Platform Support

The following shows BLE module APIs available for each platform.

|  | Linux<br/>(Ubuntu) | Raspbian<br/>(Raspberry Pi) | NuttX<br/>(STM32F4-Discovery) | TizenRT<br/>(Artik053) |
| :---: | :---: | :---: | :---: | :---: |
| ble.startAdvertising | O | O | X | - |
| ble.stopAdvertising | O | O | X | - |
| ble.setServices | O | O | X | - |


# BLE - Bluetooth Low Energy


### Event: 'advertisingStart'
* `callback` {Function}
  * error {Error}

Emitted when advertisement starts.

**Example**

```js
var ble = require('ble');
ble.on('advertisingStart', function(error) {
  console.log('on -> advertisingStart: ' + (error ? 'error ' + error : 'success'));
  if (!error) {
    ble.setServices([
        // service data
    ]);
  }
});
```

### Event: 'stateChange'
* `callback` {Function}
  * `state` {String} Can be 'unknown', 'resetting', 'unsupported', 'unauthorized', 'poweredOff' or 'poweredOn'.

Emitted when adapter state is changed.

**Example**

```js
var ble = require('ble');
ble.on('stateChange', function(state){
  console.log('onStateChange: ' + state);

  if (state == 'poweredOn') {
    ble.startAdvertising('iotjs', ['data'], function(err) {
    });
  } else {
    ble.stopAdvertising(function(err) {
    });
  }
});
```

### ble.startAdvertising(name, serviceUuids[, callback])
* `name` {string} Maximum 26 bytes.
* `serviceUuids` {Array[String]}
  * 1 128-bit service UUID
  * 1 128-bit service UUID + 2 16-bit service UUIDs
  * 7 16-bit service UUID
* `callback` {Function} Error handler.
  * `error` {Error}

Starts advertising.

`ble.state` must be in poweredOn state before advertising is started.
`ble.on('stateChange', callback(state));` can be used to register for state change events.

**Example**

```js
var name = 'name';
var serviceUuids = ['fffffffffffffffffffffffffffffff0']

ble.startAdvertising(name, serviceUuids[, callback(error)]);
```


### ble.stopAdvertising(callback)
* `callback` {Function} Error handler.
  * `error` {Error}

Stops advertising.


### ble.setServices(services[, callback])
* `services` {Array[PrimaryService]}
* `callback` {Function} Error handler.
  * `error` {Error}

Sets the primary services available on the peripheral.


## Class: Descriptor

Descriptors are defined attributes that describe a characteristic value.

### new Descriptor(options)
* `options` {Object}
  * `uuid:` {string} A Universally Unique ID (UUID) is a 16 or 128-bit hex value used to identify the type of every attribute.
  * `value` {string|Buffer}

**Example**

```js
var descriptor = new Descriptor({
    uuid: '2901',
    value: 'value'
});
```


## Class: Characteristic

Characteristics are defined attribute types that contain a single logical value.

### new Characteristic(options)
* `options` {Object}
  * `uuid:` {string} A Universally Unique ID (UUID) is a 16 or 128-bit hex value used to identify the type of every attribute.
  * `properties` {Array[string]} Can be a combination of 'read', 'write', 'writeWithoutResponse', 'notify' and 'indicate'.
  * `secure` {Array[string]} Enables security for properties, can be a combination of 'read', 'write', 'writeWithoutResponse', 'notify' and 'indicate'.
  * `value` {Buffer}
  * `descriptors` {Array[Descriptor]}
  * `onReadRequest` {Function} Read request handler. (optional)
    * `offset` {number} (0x0000 - 0xffff)
    * `callback` {Function}
      * `result` {Characteristic.RESULT_*}
      * `data` {Buffer}
  * `onWriteRequest` {Function} Write request handler. (optional)
    * `data` {Buffer}
    * `offset` {number} (0x0000 - 0xffff)
    * `withoutResponse` {boolean}
    * `callback` {Function}
      * `result` {Characteristic.RESULT_*}
  * `onSubscribe` {Function} Notify/indicate subscribe handler. (optional)
    * `maxValueSize` {number} Maximum data size.
    * `updateValueCallback` {Function}
  * `onUnsubscribe` {Function} Notify/indicate unsubscribe handler. (optional)
  * `onNotify` {Function} Notify sent handler. (optional)
  * `onIndicate` {Function} Indicate confirmation received handler. (optional)
Returns: {Characteristic}

**Example**

```js
var characteristic = new Characteristic({
  uuid: 'fffffffffffffffffffffffffffffff1', // or 'fff1' for 16-bit
  properties: ['read', 'write'],
  secure: [],
  value: null,
  descriptors: [descriptor],
  onReadRequest: null,
  onWriteRequest: null,
  onSubscribe: null,
  onUnsubscribe: null,
  onNotify: null,
  onIndicate: null
});
```

### Characteristic.RESULT_SUCCESS

### Characteristic.RESULT_INVALID_OFFSET

### Characteristic.RESULT_INVALID_ATTRIBUTE_LENGTH

### Characteristic.RESULT_UNLIKELY_ERROR


## Class: PrimaryService

PrimaryService is a collection of characteristics and relationships to other services that encapsulate the behavior of part of a device.

### new PrimaryService(options)
* `options` {Object}
  * `uuid` {string} A Universally Unique ID (UUID) is a 16 or 128-bit hex value used to identify the type of every attribute.
  * `characteristics` {Array[Characteristic]}

**Example**

```js
var primaryService = new PrimaryService({
  uuid: 'fffffffffffffffffffffffffffffff0', // or 'fff0' for 16-bit
  characteristics: [
    // see Characteristic for data type
  ]
});
```
