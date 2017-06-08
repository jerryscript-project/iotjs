## Platform Support

The following shows I2C module APIs available for each platform.

|  | Linux<br/>(Ubuntu) | Raspbian<br/>(Raspberry Pi) | NuttX<br/>(STM32F4-Discovery) |
| :---: | :---: | :---: | :---: |
| i2c.open | O | O | O |
| i2cbus.read | O | O | O |
| i2cbus.readByte | O | O | O |
| i2cbus.readBytes | O | O | O |
| i2cbus.write | O | O | O |
| i2cbus.writeByte | O | O | O |
| i2cbus.writeBytes | O | O | O |
| i2cbus.close | O | O | O |


# I2C

The I2C class supports the I2C protocol. I2C bus has two signals - SDA and SCL.


### new I2C()

Returns with an I2C object.

**Example**

```js
var I2C = require('i2c');

var i2c = new I2C();
```


### i2c.open(configuration[, callback])
* `configuration` {Object} Configuration for open I2CBus.
  * `device` {string(linux)|number(NuttX)} Device path.
  * `address` {number} Device address.
* `callback` {Function}
  * `err` {Error|null}
* Returns: {Object} An instance of I2CBus.

Get I2CBus object with configuration.

**Example**

```js
var I2C = require('i2c');

var i2c = new I2C();
var i2c_bus = i2c.open({device: '/dev/i2c-1', address: 0x23}, function(err) {
  if (err) {
    throw err;
  }
});
```


## Class: I2CBus


### i2cbus.read(length[, callback])
* `length` {number} Number of bytes to read.
* `callback` {Function}
  * `err` {Error|null}
  * `res` {Array} Array of bytes.

Read bytes from I2C device.

**Example**

```js
var I2C = require('i2c');

var i2c = new I2C();
var i2c_bus = i2c.open({device: '/dev/i2c-1', address: 0x23});

i2c_bus.read(2, function(err, res) {
  if (!err) {
    console.log('read result: ' + res);
  }
});
```


### i2cbus.readByte([callback])
* `callback` {Function}
  * `err` {Error|null}
  * `res` {number} Result byte.

Read one byte from I2C device.

**Example**

```js
var I2C = require('i2c');

var i2c = new I2C();
var i2c_bus = i2c.open({device: '/dev/i2c-1', address: 0x23});

i2c_bus.readByte(function(err, res) {
  if(!err) {
    console.log('readByte result: ' + res);
  }
});
```


### i2cbus.readBytes(cmd, length[, callback])
* `cmd` {number} Command to the device.
* `length` {number} Number of bytes to read.
* `callback` {Function}
  * `err` {Error|null}
  * `res` {Array} Array of bytes.

Read bytes from I2C device with command.

**Example**

```js
var I2C = require('i2c');

var i2c = new I2C();
var i2c_bus = i2c.open({device: '/dev/i2c-1', address: 0x23});

i2c_bus.readBytes(0x20, 2, function(err, res) {
  if(!err) {
    console.log('readBytes result: ' + res);
  }
});
```


### i2cbus.write(bytes[, callback])
* `bytes` {Array} Array of bytes to write.
* `callback` {Function}
  * `err` {Error|null}

Write bytes to I2C device.

**Example**

```js
var I2C = require('i2c');

var i2c = new I2C();
var i2c_bus = i2c.open({device: '/dev/i2c-1', address: 0x23});

i2c_bus.write([0x10], function(err) {
  if(!err) {
    console.log('write done');
  }
});
```


### i2cbus.writeByte(byte[, callback])
* `byte` {number} Byte to write.
* `callback` {Function}
  * `err` {Error|null}

Write one byte to I2C device.

**Example**

```js
var I2C = require('i2c');

var i2c = new I2C();
var i2c_bus = i2c.open({device: '/dev/i2c-1', address: 0x23});

i2c_bus.writeByte(0x10, function(err) {
  if(!err) {
    console.log('writeByte done');
  } 
});
```


### i2cbus.writeBytes(cmd, bytes[, callback])
* `cmd` {number} Command to the device.
* `bytes` {Array} Array of bytes to write.
* `callback` {Function}
  * `err` {Error|null}

Write bytes to I2C device with command.

**Example**

```js
var I2C = require('i2c');

var i2c = new I2C();
var i2c_bus = i2c.open({device: '/dev/i2c-1', address: 0x23});

i2c_bus.writeBytes(0x10, [0x10], function(err) {
  if(!err) {
    console.log('writeBytes done');
  }
});
```


### i2cbus.close()

Close I2C device.

**Example**

```js
var I2C = require('i2c');

var i2c = new I2C();
var i2c_bus = i2c.open({device: '/dev/i2c-1', address: 0x23});

i2c_bus.close();
```
