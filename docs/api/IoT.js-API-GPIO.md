### Platform Support

The following shows GPIO module APIs available for each platform.

|  | Linux<br/>(Ubuntu) | Raspbian<br/>(Raspberry Pi) | Nuttx<br/>(STM32F4-Discovery) |
| :---: | :---: | :---: | :---: |
| gpio.open | O | O | O |
| gpiopin.write | O | O | O |
| gpiopin.writeSync | O | O | O |
| gpiopin.read | △ | △ | O |
| gpiopin.readSync | O | O | O |
| gpiopin.close | O | O | O |
| gpiopin.closeSync | O | O | O |


## Contents
* [Pin](#pin)
* [GPIO](#gpio)
  * [Constructor](#gpio-constructor)
    * [`new GPIO()`](#gpio-new)
  * [Properties](#gpio-properties)
    * [`DIRECTION`](#gpio-direction)
    * [`MODE`](#gpio-mode)
  * [Prototype methods](#gpio-prototype-methods)
    * [`gpio.open(configuration[, callback])`](#gpio-open)
* [GPIOPin](#gpiopin)
  * [Prototype methods](#gpiopin-prototype-methods)
    * [`gpiopin.write(value[, callback])`](#gpiopin-write)
    * [`gpiopin.writeSync(value)`](#gpiopin-write-sync)
    * [`gpiopin.read([callback])`](#gpiopin-read)
    * [`gpiopin.readSync()`](#gpiopin-read-sync)
    * [`gpiopin.close([callback])`](#gpiopin-close)
    * [`gpiopin.closeSync()`](#gpiopin-close-sync)


## `Pin` <a name="pin"></a>

* On Linux, pin number is logical number starts from 1. Thus logical pin number *k* is not necessarily bound to physical pin number *k* in your board.
* On Nuttx, you have to know pin number. The pin number is defined in target board module. For more module information, please see below list.
  * [STM32F4-discovery](../../targets/nuttx-stm32f4/Stm32f4dis.md#gpio-pin)


## Class: GPIO <a name="gpio"></a>


## Constructor <a name="gpio-constructor"></a>


### `new GPIO()` <a name="gpio-new"></a>

Returns a new GPIO object which can open all GPIO pins.


## Properties <a name="gpio-properties"></a>


### `DIRECTION`<a name="gpio-direction"></a>
* `IN` - I/O input
* `OUT` - I/O output


### `MODE` <a name="gpio-mode"></a>
* `NONE` - none
* `PULLUP` - pull-up (only mode is input)
* `PULLDOWN` - pull-down (only mode is input)
* `FLOAT` - float (only mode is output)
* `PUSHPULL` - push-pull (only mode is output)
* `OPENDRAIN` - open drain (only mode is output)


## Prototype methods <a name="gpio-prototype-methods"></a>


### `gpio.open(configuration[, callback])` <a name="gpio-open"></a>
* `configuration <Object>`
  * `pin <Number>`, pin number to configure, mandatory configuration
  * `direction <GPIO.DIRECTION>`, direction of the pin, Default: `GPIO.DIRECTION.OUT`
  * `mode <GPIO.MODE>`, pin mode, Default: `GPIO.MODE.NONE`
* `callback <Function(err: Error | null)>`
* Returns: `<GPIOPin>`

Opens the specified GPIO pin and sets GPIO pin configuration.

The mode option is not supported on Linux.

`callback` will be called with the value. `err` is null on success, or Error on failure.

 **Example**
 ```js
 var GPIO = require('gpio');
 var gpio = new GPIO();

 var gpio10 = gpio.open({
   pin: 10,
   direction: gpio.DIRECTION.OUT,
   mode: gpio.MODE.NONE
 }, function(err) {
   if (err) {
     throw err;
   }
 });
 ```

## Class: GPIOPin <a name="gpiopin"></a>


### Prototype methods <a name="gpiopin-prototype-methods"></a>


### `gpiopin.write(value[, callback])` <a name="gpiopin-write"></a>
* `value <Boolean>`
* `callback <Function(err: Error | null)>`

Writes out a boolean value to a GPIO pin asynchronously.

`callback` will be called with the value. `err` is null on success, or Error on failure.

**Example**
```js
gpio10.write(1, function(err) {
  if (err) {
    throw err;
  }
});
```


### `gpiopin.writeSync(value)` <a name="gpiopin-write-sync"></a>
* `value <Boolean>`

Writes out a boolean value to a GPIO pin synchronously.

**Example**
```js
gpio10.writeSync(1);
```


### `gpiopin.read([callback])` <a name="gpiopin-read"></a>
* `callback <Function(err: Error | null, value: Boolean)>`

Reads a boolean value from a GPIO pin asynchronously.

`callback` will be called with the value. `err` is null on success, or Error on failure.

**Example**
```js
gpio10.read(function(err, value) {
  if (err) {
    throw err;
  }
  console.log('value:', value);
});
```


### `gpiopin.readSync()` <a name="gpiopin-read-sync"></a>
* Returns: `<Boolean>`

Returns a boolean value from a GPIO pin synchronously.

**Example**
```js
console.log('value:', gpio10.readSync());
```


### `gpiopin.close([callback])` <a name="gpiopin-close"></a>
* `callback <Function(err: Error | null)>`

Closes a GPIO pin asynchronously.

`callback` will be called with the value. `err` is null on success, or Error on failure.

**Example**
```js
gpio10.close(function(err) {
  if (err) {
    throw err;
  }
  console.log('gpio pin is closed');
});
```


### `gpiopin.closeSync()` <a name="gpiopin-close-sync"></a>

Closes a GPIO pin synchronously.

**Example**
```js
gpio10.closeSync();
console.log('gpio pin is closed');
```
