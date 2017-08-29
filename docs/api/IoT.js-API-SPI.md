### Platform Support

The following shows spi module APIs available for each platform.

|  | Linux<br/>(Ubuntu) | Raspbian<br/>(Raspberry Pi) | NuttX<br/>(STM32F4-Discovery) | TizenRT<br/>(Artik053) |
| :---: | :---: | :---: | :---: | :---: |
| spi.open | O | O | O | - |
| spibus.transfer | O | O | O | - |
| spibus.transferSync | O | O | O | - |
| spibus.close | O | O | O | - |
| spibus.closeSync | O | O | O | - |


## Class: SPI

SPI (Serial Peripheral Interface) is a communication protocol which defines a way to communicate between devices.

On NuttX, you have to know the number of pins that is defined on the target board module. For more information, please see the list below.
  * [STM32F4-discovery](../targets/nuttx/stm32f4dis/IoT.js-API-Stm32f4dis.md)

### new SPI()

Returns a new SPI object which can open SPI bus.

### SPI.MODE
The clock polarity and the clock phase can be specified as `0` or `1` to form four unique modes to provide flexibility in communication between devices. The `SPI.MODE` will specify which one to use (the combinations of the polarity and phase).

* `0` Clock Polarity(0), Clock Phase(0), Clock Edge(1)
* `1` Clock Polarity(0), Clock Phase(1), Clock Edge(0)
* `2` Clock Polarity(1), Clock Phase(0), Clock Edge(1)
* `3` Clock Polarity(1), Clock Phase(1), Clock Edge(0)


### SPI.CHIPSELECT
* `NONE`
* `HIGH`

The chip select is an access-enable switch. When the chip select pin is in the `HIGH` state, the device responds to changes on its input pins.

### SPI.BITORDER
* `MSB` The most significant bit.
* `LSB` The least significant bit.

Sets the order of the bits shifted out of and into the SPI bus, either MSB (most-significant bit first) or LSB (least-significant bit first).

### spi.open(configuration[, callback])
* `configuration` {Object}
  * `device` {string} The specified path for `spidev`. (only on Linux)
  * `bus` {number} The specified bus number. (NuttX and ARTIK05x only)
  * `mode` {SPI.MODE} The combinations of the polarity and phase. **Default:** `SPI.MODE[0]`.
  * `chipSelect` {SPI.CHIPSELECT} Chip select state. **Default:** `SPI.CHIPSELECT.NONE`.
  * `maxSpeed` {number} Maximum transfer speed. **Default:** `500000`.
  * `bitsPerWord` {number} Bits per word to send (should be 8 or 9). **Default:** `8`.
  * `bitOrder` {SPI.BITORDER} Order of the bits shifted out of and into the SPI bus. Default: `SPI.BITORDER.MSB`.
  * `loopback` {boolean} Using loopback. **Default:** `false`.
* `callback` {Function}.
  * `err` {Error|null}.
* Returns: {SPIBus}.

Opens an SPI device with the specified configuration.

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

## Class: SPIBus

The SPIBus is commonly used for communication.

### spibus.transfer(txBuffer, rxBuffer[, callback])
* `txBuffer` {Array|Buffer}.
* `rxBuffer` {Array|Buffer}.
* `callback` {Function}.
  * `err` {Error|null}.

Writes and reads data from the SPI device asynchronously.
The `txBuffer` and `rxBuffer` must have equal length.

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

### spibus.transferSync(txBuffer, rxBuffer)
* `txBuffer` {Array|Buffer}.
* `rxBuffer` {Array|Buffer}.

Writes and reads data from the SPI device synchronously.
The `txBuffer` and `rxBuffer` must have equal length.

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

### spibus.close([callback])
* `callback` {Function}.
  * `err` {Error|null}.

Closes the SPI device asynchronously.

The `callback` function will be called after the SPI device is closed.

**Example**
```js

spi0.close(function(err) {
  if (err) {
    throw err;
  }
  console.log('spi bus is closed');
});

```

### spibus.closeSync()

Closes the SPI device synchronously.

**Example**
```js

spi.closeSync();

console.log('spi bus is closed');

```
