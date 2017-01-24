## Module: i2c

### Platform Support

The following shows i2c module APIs available for each platform.

|  | Linux<br/>(Ubuntu) | Raspbian<br/>(Raspberry Pi) | Nuttx<br/>(STM32F4-Discovery) |
| :---: | :---: | :---: | :---: |
| i2c.scan | O | O | X |
| i2c.write | O | O | X |
| i2c.writeByte | O | O | X |
| i2c.writeBytes | O | O | X |
| i2c.read | O | O | X |
| i2c.readByte | O | O | X |
| i2c.readBytes | O | O | X |
| i2c.stream | O | O | X |

### Constructor

#### new i2c([address[, options]])
* `address: Number (Default value is 0x23)`
* `options: Object`

`options` is an object specifying following information:
* `device: String (Default value is /dev/i2c-1)`

Create I2C object. Following methods can be called with I2C object.

For example,
```javascript
var i2c = require('i2c');
var wire = new i2c(0x23, {device: '/dev/i2c-1'});

wire.scan(function(err, data) {
 // Do something.
});
```


### Events

#### `'data'`
* `callback: Function(data)`

 `data` is an object specifying following information:
 * `address: Number`
 * `data: Array (an array of bytes)`
 * `cmd: Number`
 * `length: Number`
 * `timestamp: Number`

Emitted when `stream` method is called.

For example,
```javascript
...
wire.on('data', function(data) {
  console.log('data.timestamp: ' + data.timestamp);
  console.log('data.address: ' + data.address);
  console.log('data.cmd: ' + data.cmd);
  console.log('data.length: ' + data.length);
  console.log('data.data: ' + data.data);
});

wire.stream(0x20, 2, 1000);
```


### Methods

#### i2c.read(length, callback)
* `length: Number`
* `callback: Function(err, res)`

Read bytes from I2C device.
`length` is the number of bytes. `res` contains an array of bytes.


#### i2c.readByte(callback)
* `callback: Function(err, res)`

Read one byte from I2C device. `res` contains result as a number.


#### i2c.readBytes(command, length, callback)
* `command: Number`
* `length: Number`
* `callback: Function(err, res)`

Read bytes from I2C device with command.
`length` is the number of bytes. `res` contains an array of bytes.


#### i2c.scan(callback)
* `callback: Function(err, data)`

Scan I2C device. `data` contains an array of addresses.


#### i2c.stream(command, length, delay)
* `command: Number`
* `length: Number`
* `delay: Number`

Read bytes from I2C device with command, continuously. It emits `data` event.
`length` is the number of bytes. It will pause for the amount of time(`delay` in ms).


#### i2c.write(bytes, callback)
* `bytes: Array`
* `callback: Function(err)`

Write bytes to I2C device. `bytes` is an array of numbers.


#### i2c.writeByte(byte, callback)
* `byte: Number`
* `callback: Function(err)`

Write one byte to I2C device.


#### i2c.writeBytes(command, bytes, callback)
* `command: Number`
* `bytes: Array`
* `callback: Function(err)`

Write bytes to I2C device with command. `bytes` is an array of numbers.
