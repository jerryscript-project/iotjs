### Platform Support

The following shows PWM module APIs available for each platform.

|  | Linux<br/>(Ubuntu) | Raspbian<br/>(Raspberry Pi) | Nuttx<br/>(STM32F4-Discovery) |
| :---: | :---: | :---: | :---: |
| pwm.open | O | O | O |
| pwmpin.setPeriod | O | O | O |
| pwmpin.setPeriodSync | O | O | O |
| pwmpin.setFrequency | O | O | O |
| pwmpin.setFrequencySync | O | O | O |
| pwmpin.setDutyCycle | O | O | O |
| pwmpin.setDutyCycleSync | O | O | O |
| pwmpin.setEnable | O | O | O |
| pwmpin.setEnableSync | O | O | O |
| pwmpin.close | O | O | O |
| pwmpin.closeSync | O | O | O |


## Contents
* [Pin](#pin)
* [PWM](#pwm)
  * [Constructor](#pwm-constructor)
    * [`new PWM()`](#pwm-new)
  * [Prototype methods](#pwm-prototype-methods)
    * [`pwm.open(configuration[, callback])`](#pwm-open)
* [PWMPin](#pwmpin)
  * [Prototype methods](#pwmpin-prototype-methods)
    * [`pwmpin.setPeriod(period[, callback])`](#pwmpin-period)
    * [`pwmpin.setPeriodSync(period)`](#pwmpin-period-sync)
    * [`pwmpin.setFrequency(frequency[, callback])`](#pwmpin-frequency)
    * [`pwmpin.setFrequencySync(frequency)`](#pwmpin-frequency-sync)
    * [`pwmpin.setDutyCycle(dutyCycle[, callback])`](#pwmpin-dutycycle)
    * [`pwmpin.setDutyCycleSync(dutyCycle)`](#pwmpin-dutycycle-sync)
    * [`pwmpin.setEnable(enable[, callback])`](#pwmpin-enable)
    * [`pwmpin.setEnableSync(enable)`](#pwmpin-enable-sync)
    * [`pwmpin.close([callback])`](#pwmpin-close)
    * [`pwmpin.closeSync()`](#pwmpin-close-sync)


### `pin` <a name="pin"></a>
* On Linux, `pin` is a pwm number which is 0 or 1.
* On Nuttx, you have to know pin name. The pin name is defined in target board module. For more module information, please see below list.
  * [STM32F4-discovery](../../targets/nuttx-stm32f4/Stm32f4dis.md#pwm-pin)


## Class: PWM <a name="pwm"></a>


## Constructor <a name="pwm-constructor"></a>


### `new PWM()` <a name="pwm-new"></a>

Returns a new PWM object which can open PWM pin.


## Prototype methods <a name="pwm-prototype-methods"></a>


### `pwm.open(configuration[, callback])` <a name="pwm-open"></a>
* `configuration <Object>`
  * `pin <Number>`, mandatory configuration
  * `chip <Number>`, chip number, Default: 0, only Linux configuration
  * `period <Number>`, seconds(positive integer) 
  * `frequency <Number>`, Hz(positive integer)
  * `dutyCycle <Number>`, between 0 and 1
* `callback <Function(err: Error | null)>`
* Returns: `<PWMPin>`

Opens PWM pin with the specified configuration.


**Example**
```js
var Pwm = require('pwm');
var pwm = new Pwm();
var config = {
  pin: 0,
  period: 0.1,
  dutyCycle: 0.5
}
var pwm0 = pwm.open(config, function() {
  if (err) {
    throw err;
  }
});
```


## Class: PWMPin <a name="pwmpin"></a>


## Prototype methods <a name="pwmpin-prototype-methods"></a>


### `pwmpin.setPeriod(period[, callback])` <a name="pwmpin-period"></a>
* `period <Number>`, seconds(positive integer) 
* `callback <Function(err: Error | null)>`

Sets period that is the duration of one cycle in a repeating signal. It is given in seconds asynchronously.

`callback` will be called after period is set.

**Example**
```js
pwm0.setPeriod(1, function(err) {
  if (err) {
    throw err;
  }
  console.log('done');
});
```


### `pwmpin.setPeriodSync(period)` <a name="pwmpin-period-sync"></a>
* `period <Number>`, seconds(positive integer) 

Sets period that is the duration of one cycle in a repeating signal synchronously.

**Example**
```js
pwm0.setPeriodSync(1);
console.log('done');
```


### `pwmpin.setFrequency(frequency[, callback])` <a name="pwmpin-frequency"></a>
* `frequency <Number>`, Hz(positive integer)
* `callback <Function(err: Error | null)>`

Sets frequency that is a measurement of how often repeating signal per unit time asynchronously.

`callback` will be called after frequency is set.

**Example**
```js
pwm0.setFrequency(1, function(err) {
  if (err) {
    throw err;
  }
  console.log('done');
});
```


### `pwmpin.setFrequencySync(frequency)` <a name="pwmpin-frequency-sync"></a>
* `frequency <Number>`, Hz(positive integer)

Sets frequency that is a measurement of how often repeating signal per unit time synchronously.

**Example**
```js
pwm0.setFrequencySync(1);
console.log('done');
```


### `pwmpin.setDutyCycle(dutyCycle[, callback])` <a name="pwmpin-dutycycle"></a>
* `dutyCycle <Number>`, between 0 and 1
* `callback <Function(err: Error | null)>`

Sets duty cycle that is the ratio of pulse width in one period asynchronously.

`callback` will be called after duty-cycle is set.

**Example**
```js
pwm0.setDutyCycle(1, function(err) {
  if (err) {
    throw err;
  }
  console.log('done');
});
```


### `pwmpin.setDutyCycleSync(dutyCycle)` <a name="pwmpin-dutycycle-sync"></a>
* `dutyCycle <Number>`, between 0 and 1

Sets duty cycle that is the ratio of pulse width in one period synchronously.

**Example**
```js
pwm0.setDutyCycleSync(1);
console.log('done');
```


### `pwmpin.setEnable(enable[, callback])` <a name="pwmpin-enable"></a>
* `enable <Boolean>`
* `callback <Function(err: Error | null)>`

Enables or Disables PWM asynchronously.

`callback` will be called after enable is set.

**Example**
```js
pwm0.setEnable(true, function(err) {
  if (err) {
    throw err;
  }
  console.log('done');
});
```


### `pwmpin.setEnableSync(enable)` <a name="pwmpin-enable-sync"></a>
* `enable <Boolean>`

Enables or Disables PWM synchronously.

**Example**
```js
pwm0.setEnableSync(false);
console.log('done');
```


### `pwmpin.close([callback])` <a name="pwmpin-close"></a>
* `callback <Function(err: Error | null)>`

Closes PWM pin asynchronously.

`callback` will be called after PWM device is released.

**Example**
```js
pwm0.close(function(err) {
  if (err) {
    throw err;
  }
  console.log('done');
});
```


### `pwmpin.closeSync()` <a name="pwmpin-close-sync"></a>

Closes PWM pin synchronously.

**Example**
```js
pwm0.closeSync();
console.log('done');
```