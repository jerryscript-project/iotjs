### Platform Support

The following shows spi module APIs available for each platform.

|  | Linux<br/>(Ubuntu) | Raspbian<br/>(Raspberry Pi) | Nuttx<br/>(STM32F4-Discovery) |
| :---: | :---: | :---: | :---: |
| spi.open | O | O | X |
| spibus.transfer | O | O | X |
| spibus.transferSync | O | O | X |
| spibus.close | O | O | X |
| spibus.closeSync | O | O | X |


## Contents
* [SPI](#spi)
  * [Constructor](#spi-constructor)
    * [`new SPI()`](#spi-new)
  * [Properties](#spi-properties)
    * [`MODE`](#spi-mode)
    * [`CHIPSELECT`](#spi-chipselect)
    * [`BITORDER`](#spi-bitorder)
  * [Prototype methods](#spi-prototype-methods)
    * [`spi.open(configuration[, callback])`](#spi-open)
* [SPIBus](#spibus)
  * [Prototype methods](#spibus-prototype-methods)
    * [`spibus.transfer(tx, rx[, callback])`](#spibus-transfer)
    * [`spibus.transferSync(tx, rx)`](#spibus-transfer-sync)
    * [`spibus.close([callback])`](#spibus-close)
    * [`spibus.closeSync()`](#spibus-close-sync)


## Class: SPI <a name="spi"></a>


## Constructor <a name="spi-constructor"></a>


### `new SPI()` <a name="spi-new"></a>

Returns a new SPI object which can open SPI bus.


## Properties <a name="spi-properties"></a>


### `MODE`<a name="spi-mode"></a>
* `0` - Clock Polarity(0), Clock Phase(0), Clock Edge(1)
* `1` - Clock Polarity(0), Clock Phase(1), Clock Edge(0)
* `2` - Clock Polarity(1), Clock Phase(0), Clock Edge(1)
* `3` - Clock Polarity(1), Clock Phase(1), Clock Edge(0)


### `CHIPSELECT`<a name="spi-chipselect"></a>
* `NONE`
* `HIGH` - chip select active high


### `BITORDER`<a name="spi-bitorder"></a>
* `MSB` - most significant bit
* `LSB` - least significant bit


## Prototype methods <a name="spi-prototype-methods"></a>


### `spi.open(configuration[, callback])` <a name="spi-open"></a>
* `configuration <Object>`
  * `device <String>`, the specified path for `spidev`
  * `mode <SPI.MODE>`, the combinations of polarity and phases, Default: `SPI.MODE[0]`
  * `chipSelect <SPI.CHIPSELECT>`, Default: `SPI.CHIPSELECT.NONE`
  * `maxSpeed <Number>`, max transfer speed, Default: `500000`
  * `bitsPerWord <Number>`, bits per word to send, should be 8 or 9, Default: `8`
  * `bitOrder <SPI.bitORDER>`, Default: `SPI.BITORDER.MSB`
  * `loopback <Boolean>`, Default: `false`
* `callback <Function(err: Error | null)>`
* Returns: `<SPIBus>`

Opens SPI device with the specified configuration.

**Example**
```js
var Spi = require('spi');
var spi = new Spi();
var spi0 = spi.open({
  device: '/dev/spidev0.0'
  }, function(err) {
    if (err) {
      throw err;
    }
});
```

## Class: SPIBus <a name="spibus"></a>


## Prototype methods <a name="spibus-prototype-methods"></a>


### `spibus.transfer(txBuffer, rxBuffer[, callback])` <a name="spibus-transfer"></a>
* `txBuffer <Array | Buffer>`
* `rxBuffer <Array | Buffer>`
* `callback <Function(err: Error | null)>`

Writes and reads data from SPI device asynchronously.
`txBuffer` and `rxBuffer` must have equal length.

**Example**
```js
var tx = new Buffer('Hello IoTjs');
var rx = new Buffer(tx.length);
spi0.transfer(tx, rx, function(err) {
  if (err) {
    throw err;
  }

  var value = '';
  for (var i = 0; i < tx.length; i++) {
    value += String.fromCharCode(rx[i]);
  }
  console.log(value);
});
```

### `spibus.transferSync(txBuffer, rxBuffer)` <a name="spibus-transfer-sync"></a>
* `txBuffer <Array | Buffer>`
* `rxBuffer <Array | Buffer>`

Writes and reads data from SPI device synchronously.
`txBuffer` and `rxBuffer` must have equal length.

**Example**
```js
var tx = new Buffer('Hello IoTjs');
var rx = new Buffer(tx.length);
spi0.transferSync(tx, rx);
var value = '';
for (var i = 0; i < tx.length; i++) {
  value += String.fromCharCode(rx[i]);
}
console.log(value);
```


### `spibus.close([callback])` <a name="spibus-close"></a>
* `callback <Function(err: Error | null)>`

Closes SPI device asynchronously.

`callback` will be called after SPI device is closed.

**Example**
```js
spi0.close(function(err) {
  if (err) {
    throw err;
  }
  console.log('spi bus is closed');
});
```


### `spibus.closeSync()` <a name="spibus-close-sync"></a>

Closes SPI device synchronously.

**Example**
```js
spi.closeSync();
console.log('spi bus is closed');
```
