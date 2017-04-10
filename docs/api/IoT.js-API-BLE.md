### Platform Support

The following shows ble module APIs available for each platform.

|  | Linux<br/>(Ubuntu) | Raspbian<br/>(Raspberry Pi) | Nuttx<br/>(STM32F4-Discovery) |
| :---: | :---: | :---: | :---: |
| ble.startAdvertising | O | O | X |
| ble.stopAdvertising | O | O | X |
| ble.setServices | O | O | X |


### Contents

- [BLE](#ble)
    - [Module Functions](#module-functions)
        - [`ble.startAdvertising(name, serviceUuids[, callback])`](#blestartadvertisingname-serviceuuids-callback)
        - [`ble.stopAdvertising(callback)`](#blestopadvertisingcallback)
        - [`ble.stopAdvertising(callback)`](#blestopadvertisingcallback-1)
        - [`ble.setServices(services[, callback])`](#blesetservicesservices-callback)
    - [Events](#events)
        - [`'stateChange'`](#statechange)
        - [`'advertisingStart'`](#advertisingstart)


# BLE


## Module Functions


### `ble.startAdvertising(name, serviceUuids[, callback])`
* `name <String>` - maximum 26 bytes
* `serviceUuids <Array>`
  * 1 128-bit service UUID
  * 1 128-bit service UUID + 2 16-bit service UUIDs
  * 7 16-bit service UUID
* `callback <Function(error)>`

Starts advertising.

ble.state must be poweredOn before advertising is started. 
ble.on('stateChange', callback(state)); can be used register for state change events.


### `ble.stopAdvertising(callback)`
* `callback <Function(error)>`

Stops advertising.


### `ble.stopAdvertising(callback)`
* `callback <Function(error)>`

Stops advertising.


### `ble.setServices(services[, callback])`
* `services <Array>`
* `callback <Function(error)>`

Sets the primary services available on the peripheral.


## Events


### `'stateChange'`
* `callback <Function(state)>`
  * `state <String>` - "unknown" | "resetting" | "unsupported" | "unauthorized" | "poweredOff" | "poweredOn"

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

### `'advertisingStart'`
* `callback <Function(error)>`

Emitted when advertisement start.

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