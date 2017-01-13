## Module: GPIO

### Platform Support

The following shows GPIO module APIs available for each platform.

|  | Linux<br/>(Ubuntu) | Raspbian<br/>(Raspberry Pi) | Nuttx<br/>(STM32F4-Discovery) |
| :---: | :---: | :---: | :---: |
| gpio.initialize | O | O | O |
| gpio.release | O | O | O |
| gpio.open | △ | △ | O |
| gpio.write | O | O | O |
| gpio.read | O | O | O |


### `pins`

* On Linux, pin number is logical number starts from 1. Thus logical pin number *k* is not necessarily bound to physical pin number *k* in your board.
* On Nuttx, you have to know pin name. The pin name is defined in target board module. For more module information, please see below list.
 * [STM32F4-discovery](../targets/nuttx-stm32f4/Stm32f4dis-Module.md).


### Events


#### `'error'`
* `callback: Function(error)`
 * `error: GpioError`

Emitted when there is an error.


#### `'initialize'`
* `callback: Function()`

Emitted after GPIO device is successfully initialized.


#### `'open'`
* `callback: Function(pin, direction, mode)`
 * `pin: Number` - opened pin number
 * `direction: String` - direction of the pin
 * `mode: String` - mode of the pin

Emitted after GPIO pin is set.


#### `'read'`
* `callback: Function(pin, value)`
 * `pin: Number` - read pin number
 * `value: Boolean` - read value

Emitted after GPIO read pin is finished.


#### `'release'`
* `callback: Function()`

Emitted after GPIO device is successfully released.


#### `'write'`
* `callback: Function(pin, value)`
 * `pin: Number` - written pin number
 * `value: Boolean` written value

Emitted after GPIO write pin is finished.


### Methods

#### gpio.initialize(callback)
* `callback: Function(err: GpioError | null)`

Initializes GPIO device.
This function must be called before other GPIO functions.

`callback` will be called after GPIO device is initialized.

'initialize' event will be emitted after GPIO device is initialized.


#### gpio.open(pinNumber, direction[, mode][, callback])
* `pinNumber: Number` - pin number to configure
* `direction: 'in' | 'out' | 'none'` - direction of the pin `'none'` for releasing GPIO pin
* `mode: 'pullup' | 'pulldn' | 'float' | 'pushpull' | 'opendrain' | 'none' | '' | undefined` - pin mode
* `callback: Function(err: GpioError | null)`

Sets GPIO pin configuration.

The mode option is not supported in Raspberry PI.

`callback` will be called after GPIO pin is set.

'open' event will be emitted after pin is set.


#### gpio.read(pinNumber[, callback])
* `pinNumber: Number` - pin number to read
* `callback: Function(err: Error | null, value: Boolean)`

Reads boolean value from a GPIO pin.

`callback` will be called with the value.

'read' event will be emitted after I/O finishes.


#### gpio.release(callback)
* `callback: Function(err: GpioError | null)`

Releases GPIO device.
After this function any other GPIO function call except `initialize()` will be failed.

`callback` will be called after GPIO device is released.

'release' event will be emitted after GPIO device is released.


#### gpio.write(pinNumber, value[, callback])
* `pinNumber: Number` - pin number to write
* `value: Boolean`
* `callback: Function(err: Error | null)`

Writes out a boolean value to a GPIO pin.

`callback` will be called after I/O finishes.

'write' event will be emitted after I/O finishes.


### class: GpioError

`GpioError` is for represent error occurs during GPIO processing.
`GpioError` inherits `Error`.
