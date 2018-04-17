### Platform Support

The following table shows ADC module APIs available for each platform.

|  | Linux<br/>(Ubuntu) | Tizen<br/>(Raspberry Pi) | Raspbian<br/>(Raspberry Pi) | NuttX<br/>(STM32F4-Discovery) | TizenRT<br/>(Artik053) |
| :---: | :---: | :---: | :---: | :---: | :---: |
| adc.open | X | X | X | O | O |
| adc.openSync | X | X | X | O | O |
| adcpin.read | X | X | X | O | O |
| adcpin.readSync | X | X | X | O | O |
| adcpin.close | X | X | X | O | O |
| adcpin.closeSync | X | X | X | O | O |


# ADC

This module allows reading analogue data from hardware pins.

The hardware pins can be read from or written to, therefore they are called bidirectional IO pins. This module provides the reading part.

On NuttX, you have to know the number of pins that is defined on the target board module. For more information, please see the list below.
  * [STM32F4-discovery](../targets/nuttx/stm32f4dis/IoT.js-API-Stm32f4dis.md#adc-pin)


### adc.open(configuration, callback)
* `configuration` {Object}
  * `device` {string} Mandatory configuration on Linux.
  * `pin` {number} Mandatory configuration on NuttX and TizenRT.
* `callback` {Function}
  * `err`: {Error|null}
  * `adcpin` {Object} An instance of AdcPin.
* Returns: {Object} An instance of AdcPin.

Opens an ADC pin with the specified configuration asynchronously.

**Example**
```js
var adc = require('adc');
var adc0 = adc.open({
  device: '/sys/devices/12d10000.adc/iio:device0/in_voltage0_raw'
}, function(err) {
  if (err) {
    throw err;
  }
});
```

### adc.openSync(configuration)
* `configuration` {Object}
  * `device` {string} Mandatory configuration on Linux.
  * `pin` {number} Mandatory configuration on NuttX and TizenRT.
* `callback` {Function}
  * `err`: {Error|null}
* Returns: {Object} An instance of AdcPin.

Opens an ADC pin with the specified configuration synchronously.

**Example**
```js
var adc = require('adc');
var adc0 = adc.openSync({
  device: '/sys/devices/12d10000.adc/iio:device0/in_voltage0_raw'
});
```


## Class: AdcPin

### adcpin.read(callback)
* `callback` {Function}
  * `err`: {Error|null}
  * `{number}` Analog value.

Reads the analog value from the pin asynchronously.

`callback` will be called having read the analog value.

**Example**
```js
adc0.read(function(err, value) {
  if (err) {
    throw err;
  }
  console.log('value:', value);
});
```


### adcpin.readSync()
* Returns: `{number}` Analog value.

Reads the analog value from the pin synchronously.

**Example**
```js
var value = adc0.readSync();
console.log('value:', value);
```


### adcpin.close([callback])
* `callback` {Function}
  * `err`: {Error|null}

Closes ADC pin asynchronously. This function must be called after the work of ADC finished.

`callback` will be called after ADC device is released.

**Example**
```js
adc0.close(function(err) {
  if (err) {
    throw err;
  }
});
```


### adcpin.closeSync()

Closes ADC pin synchronously. This function must be called after the work of ADC finished.

**Example**
```js
adc0.closeSync();
console.log('adc pin is closed');
```
