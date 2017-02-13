## Module: ble

### Platform Support

The following shows ble module APIs available for each platform.

|  | Linux<br/>(Ubuntu) | Raspbian<br/>(Raspberry Pi) | Nuttx<br/>(STM32F4-Discovery) |
| :---: | :---: | :---: | :---: |
| ble.startAdvertising | O | O | X |
| ble.stopAdvertising | O | O | X |
| ble.setServices | O | O | X |

### Events

#### `'stateChange'`
* `callback: Function(state)`

  * `state: String` - "unknown" | "resetting" | "unsupported" | "unauthorized" | "poweredOff" | "poweredOn"

Emitted when adapter state is changed.


### Methods

#### ble.startAdvertising(name, serviceUuids[, callback(error)])
* `name: String` - maximum 26 bytes
* `serviceUuids: Array`
  * 1 128-bit service UUID
  * 1 128-bit service UUID + 2 16-bit service UUIDs
  * 7 16-bit service UUID
* `callback: Function(error)`

Start advertising.

ble.state must be poweredOn before advertising is started. 
ble.on('stateChange', callback(state)); can be used register for state change events.


#### ble.stopAdvertising(callback)
* `callback: Function()`

Stop advertising.
