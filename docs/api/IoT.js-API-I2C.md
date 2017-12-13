## Platform Support

The following shows I2C module APIs available for each platform.

|  | Linux<br/>(Ubuntu) | Raspbian<br/>(Raspberry Pi) | NuttX<br/>(STM32F4-Discovery) | TizenRT<br/>(Artik053) |
| :---: | :---: | :---: | :---: | :---: |
| i2c.read | O | O | O | O |
| i2c.write | O | O | O | O |
| i2c.close | O | O | O | O |


# I2C

The I2C class supports the I2C protocol. I2C bus has two signals - SDA and SCL.


Note: All functions including constructor take a callback. If the callback is undefined, it is completed synchronously. Otherwise, it will be executed asynchronously.

### new I2C(configuration[, callback])
* `configuration` {Object} Configuration for open I2CBus.
  * `device` {string} Device path. (only on Linux)
  * `bus` {number} The specified bus number. (NuttX and TizenRT only)
  * `address` {number} Device address.
* `callback` {Function}
  * `err` {Error|null}
* Returns: {Object} An instance of I2CBus.

Get I2C object with configuration and open I2C bus.

**Example**

```js
var I2C = require('i2c');

// Create I2C object and open synchronously
var i2c_1 = new I2C({device: '/dev/i2c-1', address: 0x23});

// Create I2C object and open asynchronously
var i2c_2 = new I2C({device: '/dev/i2c-1', address: 0x23}, function(err) {
  if (err) {
    throw err;
  }
});
```

### i2c.read(length[, callback])
* `length` {number} Number of bytes to read.
* `callback` {Function}
  * `err` {Error|null}
  * `res` {Array} Array of bytes.

Read bytes from I2C device.

**Example**

```js
var I2C = require('i2c');

var i2c = new I2C({device: '/dev/i2c-1', address: 0x23});

// Read synchronously
var res = i2c.read(2);

// Read asynchronously
i2c.read(2, function(err, res) {
  if (!err) {
    console.log('read result: ' + res);
  }
});
```

### i2c.write(bytes[, callback])
* `bytes` {Array} Array of bytes to write.
* `callback` {Function}
  * `err` {Error|null}

Write bytes to I2C device.

**Example**

```js
var I2C = require('i2c');

var i2c = new I2C({device: '/dev/i2c-1', address: 0x23});

// Write synchronously
i2c.write([0x10]);

// Write asynchronously
i2c.write([0x10], function(err) {
  if(!err) {
    console.log('write done');
  }
});
```


### i2c.close([callback])
* `callback` {Function}
  * `err` {Error|null}
  
Close I2C device.

**Example**

```js
var I2C = require('i2c');

var i2c = new I2C({device: '/dev/i2c-1', address: 0x23});

// Close synchronously
i2c_bus.close();

// Close asynchronously
i2c_bus.close(function(err) {
  if (err) {
    throw err;
  }
});
```
