IoT.js provides ADC module.

## Module: adc

### Platform Support

The following shows ADC module APIs available for each platform.

|  | Linux<br/>(Ubuntu) | Raspbian<br/>(Raspberry Pi) | Nuttx<br/>(STM32F4-Discovery) |
| :---: | :---: | :---: | :---: |
| adc.read | X | X | O |
| adc.readSync | X | X | O |
| adc.unexport | X | X | O |

### `pin`
* On Nuttx, you have to use [Pin module](IoT.js-API-Pin.md).


### Constructor

#### new adc(pin[, callback])
* `pin: Pin module`
* `callback: Function(err)`
 * `err: Error`

Creates ADC object.

For example,
```javascript
var adc = require('adc');
var stm32f4Pin = require('pin').STM32F4DIS;

var adc1 = new adc(stm32f4Pin.ADC1_3, function() {
  adc.read(function(err, value) {
    console.log(value);
  });
});
```

### Methods

#### adc.read([, callback])
* `callback: Function(err, vaule)`
 * `err: Error`
 * `value: Number` - analog value

Reads the analog value from pin asynchronously.

`callback` will be called after read the analog value.


#### adc.readSync()
* Return: `Number` - analog value

Reads the analog value from pin synchronously.


#### adc.unexport([callback])
* `callback: Function(err)`
 * `err: Error`

ADC device is released. This function must be called after the work of ADC finished.

`callback` will be called after ADC device is released.
