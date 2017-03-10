## Platform Support

The following shows i2c module APIs available for each platform.

|  | Linux<br/>(Ubuntu) | Raspbian<br/>(Raspberry Pi) | Nuttx<br/>(STM32F4-Discovery) |
| :---: | :---: | :---: | :---: |
| i2c.open | O | O | O |
| i2cbus.read | O | O | O |
| i2cbus.readByte | O | O | O |
| i2cbus.readBytes | O | O | O |
| i2cbus.write | O | O | O |
| i2cbus.writeByte | O | O | O |
| i2cbus.writeBytes | O | O | O |
| i2cbus.close | O | O | O |


## Contents

  * [I2C](#i2c)
    * [Constructor](#constructor)
      * [`new I2C()`](#new-i2c)
    * [Prototype Methods](#i2c-prototype-methods)
      * [`i2c.open(configurable[, callback])`](#i2c-open)
  * [I2CBus](#i2cbus)
    * [Prototype Methods](#i2cbus-prototype-methods)
      * [i2cbus.read(length[, callback])](#i2cbus-read)
      * [i2cbus.readByte([callback])](#i2cbus-readbyte)
      * [i2cbus.readBytes(cmd, length[, callback])](#i2cbus-readbytes)
      * [i2cbus.write(array[, callback])](#i2cbus-write)
      * [i2cbus.writeByte(byte[, callback])](#i2cbus-writebyte)
      * [i2cbus.writeBytes(cmd, array[, callback])](#i2cbus-writebytes)
      * [i2cbus.close()](#i2cbus-close)


## Class: I2C <a name="i2c"></a>

The I2C class supports the I2C protocol. I2C bus has two signals - SDA and SCL.


## Constructor <a name="constructor"></a>


### new I2C()

Create I2C object.

**Example**

```js
var I2C = require('i2c');

var i2c = new I2C();
```


## Prototype Methods <a name="i2c-prototype-methods"></a>


### `i2c.open(configurable[, callback])` <a name="i2c-open"></a>
 * `configurable <Object>`, configuration for open I2CBus
  * `device <String(linux)|Number(nuttx)>`, device path
  * `address <Number>`
 * `callback <Function(err: Error)>`

Get I2CBus object with configurable.

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


## Class: I2CBus <a name="i2cbus"></a>


## Prototype Methods <a name="i2cbus-prototype-methods"></a>


### `i2cbus.read(length[, callback])` <a name="i2cbus-read"></a>
 * `length <Number>`, the number of bytes.
 * `callback <Function(err: Error, res: Array)>`, `res` contains an array of bytes.

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


### `i2cbus.readByte([callback])` <a name="i2cbus-readbyte"></a>
 * `callback <Function(err: Error, res: Array)>`, `res` contains an array of bytes.

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


### `i2cbus.readBytes(cmd, length[, callback])` <a name="i2cbus-readbytes"></a>
 * `command <Number>`
 * `length <Number>`, the number of bytes
 * `callback <Function(err: Error, res: Array)>`, `res` contains an array of bytes.

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


### `i2cbus.write(array[, callback])` <a name="i2cbus-write"></a>
 * `bytes <Array>`, an array of numbers
 * `callback <Function(err: Error)>`

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


### `i2cbus.writeByte(byte[, callback])` <a name="i2cbus-writebyte"></a>
 * `byte <Number>`
 * `callback <Function(err: Error)>`

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


### `i2cbus.writeBytes(cmd, array[, callback])` <a name="i2cbus-writebytes"></a>
 * `command <Number>`
 * `bytes <Array>`, an array of numbers
 * `callback <Function(err: Error)>`

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


### `i2cbus.close()` <a name="i2cbus-close"></a>

Close I2C device.

**Example**

```js
var I2C = require('i2c');

var i2c = new I2C();
var i2c_bus = i2c.open({device: '/dev/i2c-1', address: 0x23});

i2c_bus.close();
```
