### Platform Support

The following shows PWM module APIs available for each platform.

|  | Linux<br/>(Ubuntu) | Raspbian<br/>(Raspberry Pi) | NuttX<br/>(STM32F4-Discovery) | TizenRT<br/>(Artik053) |
| :---: | :---: | :---: | :---: | :---: |
| pwm.open | O | O | O | - |
| pwmpin.setPeriod | O | O | O | - |
| pwmpin.setPeriodSync | O | O | O | - |
| pwmpin.setFrequency | O | O | O | - |
| pwmpin.setFrequencySync | O | O | O | - |
| pwmpin.setDutyCycle | O | O | O | - |
| pwmpin.setDutyCycleSync | O | O | O | - |
| pwmpin.setEnable | O | O | O | - |
| pwmpin.setEnableSync | O | O | O | - |
| pwmpin.close | O | O | O | - |
| pwmpin.closeSync | O | O | O | - |


## Class: PWM

### new PWM()

Returns a new PWM object which can open a PWM pin.

This object allows the developer to specify a pin and generate a pulse-width modulatated (PWM)
signal through that.

### pwm.open(configuration[, callback])
* `configuration` {Object} Configuration object which can have the following properties.
  * `pin` {number} The pin number to use with this PWM object (mandatory configuration).
  * `chip` {number} The PWM chip number (only on Linux). **Default:** `0`.
  * `period` {number} The period of the PWM signal, in seconds (positive number).
  * `frequency` {integer} In Hz (positive integer).
  * `dutyCycle` {number} The active time of the PWM signal, must be within the `0.0` and `1.0` range.
* `callback` {Function} Callback function.
  * `err` {Error|null} The error object or `null` if there were no error.
* Returns: `<PWMPin>`

Opens PWM pin with the specified configuration.

To correctly open a PWM pin one must know the correct pin number:
* On Linux, `pin` is a number which is `0` or `1`.
* On NuttX, you have to know pin name. The pin name is defined in target board module. For more module information, please see below list.
  * [STM32F4-discovery](../targets/nuttx/stm32f4dis/IoT.js-API-Stm32f4dis.md#pwm-pin)


**Example**
```js
var Pwm = require('pwm');
var pwm = new Pwm();
var config = {
  pin: 0,
  period: 0.1,
  dutyCycle: 0.5
}
var pwm0 = pwm.open(config, function(err) {
  if (err) {
    throw err;
  }
});
```


## Class: PWMPin

### pwmpin.setPeriod(period[, callback])
* `period` {number} The period of the PWM signal, in seconds (positive number).
* `callback` {Function}
  * `err` {Error|null} The error object or `null` if there were no error.

The `setPeriod` method allows the configuration of the period of the PWM signal in seconds.
The `period` argument must specified and it should be a positive number.

Configuration is done asynchronously and the `callback` method is invoked after the
period is configured to the new value or if an error occured.

**Example**
```js
pwm0.setPeriod(1, function(err) {
  if (err) {
    throw err;
  }
  console.log('done');
});
// prints: do
console.log('do');
// prints: done
```


### pwmpin.setPeriodSync(period)
* `period` {number} The period of the PWM signal, in seconds (positive number).

The `setPeriodSync` method allows the configuration of the period of the PWM signal in seconds.
The `period` argument must specified and it should be a positive number.

Configuration is done synchronously and will block till the period is configured.

**Example**
```js
pwm0.setPeriodSync(1);
// prints: done
console.log('done');
```


### pwmpin.setFrequency(frequency[, callback])
* `frequency` {integer} In Hz (positive integer).
* `callback` {Function}
  * `err` {Error|null} The error object or `null` if there were no error.

The `setFequency` method congifures the frequency of the PWM signal.
`frequency` is the measurement of how often the signal is repeated in a single period.

Configuration is done asynchronously and the `callback` method is invoked after the
frequency is configured to the new value or if an error occured.

**Example**
```js
pwm0.setFrequency(1, function(err) {
  if (err) {
    throw err;
  }
  console.log('done');
});
// prints: do
console.log('do');
// prints: done
```


### pwmpin.setFrequencySync(frequency)
* `frequency` {integer} In Hz (positive integer).

The `setFequencySync` method congifures the frequency of the PWM signal.
`frequency` is the measurement of how often the signal is repeated in a single period.

Configuration is done synchronously and will block till the frequency is configured.

**Example**
```js
pwm0.setFrequencySync(1);
// prints: done
console.log('done');
```


### pwmpin.setDutyCycle(dutyCycle[, callback])
* `dutyCycle` {number} Must be within the `0.0` and `1.0` range.
* `callback` {Function}
  * `err` {Error|null} The error object or `null` if there were no error.

The `setDutyCycle` method allows the configuration of the duty cycle of the PWM signal.
The duty cycle is the ratio of the pulse width in one period.

Configuration is done asynchronously and the `callback` method is invoked after the
duty cycle is configured to the new value or if an error occured.

**Example**
```js
pwm0.setDutyCycle(1, function(err) {
  if (err) {
    throw err;
  }
  console.log('done');
});
// prints: do
console.log('do');
// prints: done
```


### pwmpin.setDutyCycleSync(dutyCycle)
* `dutyCycle` {number} Must be within the `0.0` and `1.0` range.

The `setDutyCycleSync` method allows the configuration of the duty cycle of the PWM signal.
The duty cycle is the ratio of the pulse width in one period.

Configuration is done synchronously and will block till the duty cycle is configured.

**Example**
```js
pwm0.setDutyCycleSync(1);
// prints: done
console.log('done');
```


### pwmpin.setEnable(enable[, callback])
* `enable` {boolean}
* `callback` {Function}
  * `err` {Error|null} The error object or `null` if there were no error.

The `setEnable` method can turn on/off the generation of the PWM signal.
If the `enable` argument is `true` then the PWN signal generation is turned on.
Otherwise the signal generation is turned off.

After enabling/disabling the signal generation the `callback` is invoked.

**Example**
```js
pwm0.setEnable(true, function(err) {
  if (err) {
    throw err;
  }
  console.log('done');
});
// prints: do
console.log('do');
// prints: done
```


### pwmpin.setEnableSync(enable)
* `enable` {boolean}

The `setEnableSync` method can turn on/off the generation of the PWM signal.
If the `enable` argument is `true` then the PWN signal generation is turned on.
Otherwise the signal generation is turned off.

**Example**
```js
pwm0.setEnableSync(false);
// prints: done
console.log('done');
```


### pwmpin.close([callback])
* `callback` {Function}
  * `err` {Error|null} The error object or `null` if there were no error.

The `close` method closes the PWM pin asynchronously.

The `callback` method will be invoked after the PWM device is released.

**Example**
```js
pwm0.close(function(err) {
  if (err) {
    throw err;
  }
  console.log('done');
});
// prints: do
console.log('do');
// prints: done
```


### pwmpin.closeSync()

The `closeSync` method closes the PWM pin synchronously.

**Example**
```js
pwm0.closeSync();
// prints: done
console.log('done');
```
