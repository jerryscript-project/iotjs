IoT.js provides PWM module.

## Module: pwm

### Platform Support

The following shows PWM module APIs available for each platform.

|  | Linux<br/>(Ubuntu) | Raspbian<br/>(Raspberry Pi) | Nuttx<br/>(STM32F4-Discovery) |
| :---: | :---: | :---: | :---: |
| pwm.setPeriod | O | O | O |
| pwm.setFrequency | O | O | O |
| pwm.setDutyCycle | O | O | O |
| pwm.setEnable | O | O | O |
| pwm.unexport | O | O | O |

### `pin`
* On Linux, `pin` is a pwm number which is 0 or 1.
* On Nuttx, you have to know pin name. The pin name is defined in target board module. For more module information, please see below list.
  * [STM32F4-discovery](../../targets/nuttx-stm32f4/Stm32f4dis.md#pwm-pin)


### Constructor

#### new pwm(pin[, options][, callback])
* `pin: Number`
* `options: Object`
* `callback: Function(err: PwmError | null)`

Create PWM object.

`options` is an object specifying following information:
* `period: Number (seconds, positive integer)`
* `frequency: Number (Hz, positive integer)`
* `dutyCycle: Number (between 0 and 1)`

For example,
```javascript
var pwm = require('pwm');
var pwm0 = new pwm(0, {period: 0.1, dutyCycle: 0.5}, function() {
  pwm0.setEnable(1);
});
```

### Methods

#### pwm.setPeriod(period[, callback])
* `period: Number (seconds, positive integer)`
* `callback: Function(err: PwmError | null)`

Sets period that is the duration of one cycle in a repeating signal. It is given in seconds.

`callback` will be called after period is set.

This function is not supported in Nuttx.


#### pwm.setFrequency(frequency[, callback])
* `frequency: Number (Hz, positive integer)`
* `callback: Function(err: PwmError | null)`

Sets frequency that is a measurement of how often repeating signal per unit time.

`callback` will be called after frequency is set.


#### pwm.setDutyCycle(dutyCycle[, callback])
* `dutyCycle: Number (between 0 and 1)`
* `callback: Function(err: PwmError | null)`

Sets duty cycle that is the ratio of pulse width in one period.

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
