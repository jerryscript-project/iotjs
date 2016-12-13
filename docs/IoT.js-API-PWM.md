IoT.js provides PWM module.

## Module: pwm

### Platform Support

The following shows PWM module APIs available for each platform.

|  | Linux<br/>(Ubuntu) | Raspbian<br/>(Raspberry Pi) | Nuttx<br/>(STM32F4-Discovery) |
| :---: | :---: | :---: | :---: |
| pwm.setPeriod | O | O | X |
| pwm.setFrequency | O | X | O |
| pwm.setDutyCycle | O | O | O |
| pwm.setEnable | O | O | O |
| pwm.unexport | O | O | O |

### `pin`
* On Linux, `pin` is a pwm number which is 0 or 1.
* On Nuttx, you have to know pin name. The pin name is defined in target board module. For more module information, please see below list.
 * [STM32F4-discovery](../targets/nuttx-stm32f4/Stm32f4dis-Module.md).


### Constructor

#### new pwm(pin[, options][, callback])
* `pin: Number`
* `options: Object`
* `callback: Function(err: PwmError | null)`

Create PWM object.

`options` is an object specifying following information:
* `period: Number (positive integer)`
* `frequency: Number (positive integer)`
* `dutyCycle: Number (percentage: between 0 and 100)`

`frequency` is not supported in Linux.
`period` is not supported in Nuttx.

For example,
```javascript
var pwm = require('pwm');
var pwm0 = new pwm(0, {period: 10000, dutyCycle: 50}, function() {
  pwm0.setEnable(1);
});
```

### Methods

#### pwm.setPeriod(period[, callback])
* `period: Number (positive integer)`
* `callback: Function(err: PwmError | null)`

Sets period that is the duration of one cycle in a repeating signal. It is given in nanoseconds.

`callback` will be called after period is set.

This function is not supported in Nuttx.


#### pwm.setFrequency(frequency[, callback])
* `frequency: Number (positive integer)`
* `callback: Function(err: PwmError | null)`

Sets frequency that is a measurement of how often repeating signal per unit time.

`callback` will be called after frequency is set.

This function is not supported in Linux.


#### pwm.setDutyCycle(dutyCycle[, callback])
* `dutyCycle: Number (percentage: between 0 and 100)`
* `callback: Function(err: PwmError | null)`

Sets duty cycle that is the ratio of pulse width in one period. It is given in percentage.

`callback` will be called after duty-cycle is set.

#### pwm.setEnable(enable[, callback])
* `enable: Boolean`
* `callback: Function(err: PwmError | null)`

Enables or Disables PWM.

`callback` will be called after enable is set.


#### pwm.unexport([callback])
* `callback: Function(err: PwmError | null)`

PWM device is released.

`callback` will be called after PWM device is released.
