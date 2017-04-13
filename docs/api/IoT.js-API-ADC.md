### Platform Support

The following shows ADC module APIs available for each platform.

|  | Linux<br/>(Ubuntu) | Raspbian<br/>(Raspberry Pi) | Nuttx<br/>(STM32F4-Discovery) |
| :---: | :---: | :---: | :---: |
| adc.open | O | O | O |
| adcpin.read | O | O | O |
| adcpin.readSync | O | O | O |
| adcpin.close | O | O | O |
| adcpin.closeSync | O | O | O |


## Contents
* [Pin](#pin)
* [ADC](#adc)
  * [Constructor](#adc-constructor)
    * [`new ADC()`](#adc-new)
  * [Prototype methods](#adc-prototype-methods)
    * [`adc.open(configuration[, callback])`](#adc-open)
* [ADCPin](#adcpin)
  * [Prototype methods](#adcpin-prototype-methods)
    * [`adcpin.read([callback])`](#adcpin-read)
    * [`adcpin.readSync()`](#adcpin-read-sync)
    * [`adcpin.close([callback])`](#adcpin-close)
    * [`adcpin.closeSync()`](#adcpin-close-sync)

## `pin` <a name="pin"></a>
On Nuttx, you have to know pin number that is defined in target board module. For more module information, please see below list.
  * [STM32F4-discovery](../targets/nuttx/stm32f4dis/IoT.js-API-Stm32f4dis.md#adc-pin)

## Class: ADC <a name="adc"></a>


## Constructor <a name="adc-constructor"></a>


### `new ADC()` <a name="adc-new"></a>

Returns a new ADC object which can open ADC pin.


## Prototype methods <a name="adc-prototype-methods"></a>


### `adc.open(configuration[, callback])` <a name="adc-open"></a>
* `configuration <Object>`
  * `device <String>`, mandatory configuration on Linux
  * `pin <Number>`, mandatory configuration on Nuttx
* `callback <Function(err: Error | null)>`
* Returns: `<ADCPin>`

Opens ADC pin with the specified configuration.

**Example**
```js
var Adc = require('adc');
var adc = new Adc();
var adc0 = adc.open({
  device: '/sys/devices/12d10000.adc/iio:device0/in_voltage0_raw'
}, function(err) {
  if (err) {
    throw err;
  }
});
```


## Class: ADCPin <a name="adcpin"></a>


## Prototype methods <a name="adcpin-prototype-methods"></a>


### `adcpin.read([callback])` <a name="adcpin-read"></a>
* `callback <Function(err: Error | null, value: Number)>`

Reads the analog value from pin asynchronously.

`callback` will be called after read the analog value.

**Example**
```js
adc0.read(function(err, value) {
  if (err) {
    throw err;
  }
  console.log('value:', value);
});
```


### `adcpin.readSync()` <a name="adcpin-read-sync"></a>
* Return: `<Number>`, analog value

Reads the analog value from pin synchronously.

**Example**
```js
var value = adc0.readSync();
console.log('value:', value);
```


### `adcpin.close([callback])` <a name="adcpin-close"></a>
* `callback <Function(err: Error | null)>`

Closes ADC pin asynchronously. This function must be called after the work of ADC finished.

`callback` will be called after ADC device is released.

**Example**
```js
adc0.clsoe(function(err) {
  if (err) {
    throw err;
  }
});
```


### `adcpin.closeSync()` <a name="adcpin-close-sync"></a>

Closes ADC pin synchronously. This function must be called after the work of ADC finished.

**Example**
```js
adc0.clsoeSync();
console.log('adc pin is closed');
```
