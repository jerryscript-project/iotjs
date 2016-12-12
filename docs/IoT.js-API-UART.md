## Module: uart

### Platform Support

The following shows uart module APIs available for each platform.

|  | Linux<br/>(Ubuntu) | Raspbian<br/>(Raspberry Pi) | Nuttx<br/>(STM32F4-Discovery) |
| :---: | :---: | :---: | :---: |
| uart.open | O | O | X |
| uart.write | O | O | X |

### Constructor

#### new uart(path[, options][, callback])
* `path: String`
* `options: Object`
* `callback: Function(err)`

`options` is an object specifying following information:
* `baudRate: Number (Default value is 9600)`
* `dataBits: Number (Default value is 8)`
* `autoOpen: Boolean (Default value is true)`

Create UART object. Following methods can be called with UART object.

For example,
```javascript
var uart = require('uart');
var options = {
  baudRate: 115200,
  dataBits: 8,
  autoOpen: true,
}

var serial = new uart('/dev/ttyUSB0', options, function(err) {
  // Do something.
});

serial.write("Hello?", function(err) {
  // Do something.
});
```


### Events

#### `'data'`
* `callback: Function(data)`

 `data` is a string from sender

For example,
```javascript
...
serial.on('data', function(data) {
  console.log('read result: ' + data.toString());
});

```


### Methods

#### uart.open(callback)
* `callback: Function(err)`

Opens UART device.


#### uart.write(data, callback)
* `data: String`
* `callback: Function(err)`

Writes data to UART device.
