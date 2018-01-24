## Platform Support

The following shows I2C module APIs available for each platform.

|  | Linux<br/>(Ubuntu) | Raspbian<br/>(Raspberry Pi) | NuttX<br/>(STM32F4-Discovery) | TizenRT<br/>(Artik053) |
| :---: | :---: | :---: | :---: | :---: |
| i2c.open | O | O | O | O |
| i2c.openSync | O | O | O | O |
| i2cbus.read | O | O | O | O |
| i2cbus.readSync | O | O | O | O |
| i2cbus.write | O | O | O | O |
| i2cbus.writeSync | O | O | O | O |
| i2cbus.close | O | O | O | O |
| i2cbus.closeSync | O | O | O | O |


# I2C

The I2C module supports the I2C protocol. I2C bus has two signals - SDA and SCL.

### i2c.open(configuration, callback)
* `configuration` {Object} Configuration for open I2CBus.
  * `device` {string} Device path. (only on Linux)
  * `bus` {number} The specified bus number. (NuttX and TizenRT only)
  * `address` {number} Device address.
* `callback` {Function}
  * `err` {Error|null}
  * `i2cBus` {Object} An instance of I2CBus.
* Returns: {Object} An instance of I2CBus.

Get I2CBus object with configuration asynchronously.

**Example**

```js
var i2c = require('i2c');

i2c.open({device: '/dev/i2c-1', address: 0x23}, function(err, wire) {
  if (err) {
    throw err;
  }
});
```

### i2c.openSync(configuration)
* `configuration` {Object} Configuration for open I2CBus.
  * `device` {string} Device path. (only on Linux)
  * `bus` {number} The specified bus number. (NuttX and TizenRT only)
  * `address` {number} Device address.
* Returns: {Object} An instance of I2CBus.

Get I2CBus object with configuration synchronously.

**Example**

```js
var i2c = require('i2c');

var wire = i2c.openSync({device: '/dev/i2c-1', address: 0x23});
```


## Class: I2CBus


### i2cbus.read(length[, callback])
* `length` {number} Number of bytes to read.
* `callback` {Function}
  * `err` {Error|null}
  * `res` {Array} Array of bytes.

Read bytes from I2C device asynchronously.

**Example**

```js
var i2c = require('i2c');

i2c.open({device: '/dev/i2c-1', address: 0x23}, function(err, wire) {
  wire.read(2, function(err, res) {
    if (!err) {
      console.log('read result: ' + res);
    }
  });
});
```

### i2cbus.readSync(length)
* `length` {number} Number of bytes to read.
* Returns: {Array} Array of bytes.

Read bytes from I2C device synchronously.

**Example**

```js
var i2c = require('i2c');

var wire = i2c.openSync({device: '/dev/i2c-1', address: 0x23});
var res = wire.readSync(2);
console.log(res);
```

### i2cbus.write(bytes[, callback])
* `bytes` {Array} Array of bytes to write.
* `callback` {Function}
  * `err` {Error|null}

Write bytes to I2C device asynchronously.

**Example**

```js
var i2c = require('i2c');

i2c.open({device: '/dev/i2c-1', address: 0x23}, function(err, wire){
  wire.write([0x10], function(err) {
    if(!err) {
      console.log('write done');
    }
  });
});
```

### i2cbus.writeSync(bytes)
* `bytes` {Array} Array of bytes to write.

Write bytes to I2C device synchronously.

**Example**

```js
var i2c = require('i2c');

var wire = i2c.openSync({device: '/dev/i2c-1', address: 0x23});
wire.writeSync([0x10]);
```

### i2cbus.close([callback])
* `callback` {Function}
  * `err` {Error|null}

Close I2C device asynchronously.

**Example**

```js
var i2c = require('i2c');

i2c.open({device: '/dev/i2c-1', address: 0x23}, function(err, wire) {
  wire.close();
});
```

### i2cbus.closeSync()

Close I2C device synchronously.

**Example**

```js
var i2c = require('i2c');

var wire = i2c.openSync({device: '/dev/i2c-1', address: 0x23});
wire.closeSync();
```