## class: GPIO

### `pins` and `ports`

* pin number is logical number starts from 1. Thus logical pin number *k* is not necessarily bound to physical pin number *k* in your board.
* port is not supported(Nov. 03, 2015)
* port number is logical number starts from 1. Thus logical port number *k* is not necessarily bound to physical port number *k* in your board.
* 8 logical pin will be bound to a logical port. For example, pin number 1~8 will bound to port 1, pin number 9~16 bound to port 2, and so forth.
* If you write a byte value to a port, the result is the same as writing each bit to corresponding pin. For example, let's say you write (10101011)2 to port 1. the operation will write up bit to pin 1, up bit to pin 2, down bit to pin 3, ... , up bit to pin 8.

### Methods

#### gpio.initialize(callback)

* `callback: Function(err: GpioError | null)`

Initializes GPIO device.
This function must be called before other GPIO functions.

`callback` will be called after after GPIO device is initialized.

'initialize' event will be emitted after GPIO device is initialized.


#### gpio.release(callback)

* `callback: Function(err: GpioError | null)`

Release GPIO device.
After this function any other GPIO function call except `initialize()` will be failed.

`callback` will be called after GPIO device is released.

'release' event will be emitted after GPIO device is released.


#### gpio.setPin(pinNumber, direction[, mode][, callback])

* `pinNumber: Number`: pin number to configure
* `direction: 'in' | 'out' | 'none'`: direction of the pin. `'none'` for releasing GPIO pin.
* `mode: 'pullup' | 'pulldn' | 'float' | 'pushpull' | 'opendrain' | 'none' | '' | undefined` : pin mode. 
* `callback: Function(err: GpioError | null)`.

Set GPIO pin configuration.

`callback` will be called after GPIO pin is set.

'setPin' event will be emitted after pin is set.


#### gpio.writePin(pinNumber, value[, callback])
* `pinNumber: Number` - pin number to wirte
* `value: Boolean`.
* `callback: Function(err: Error | null)`.

Write out a boolean value to a GPIO pin.

`callback` will be called after I/O finishes.

'writePin' event will be emitted after I/O finishes.


#### gpio.readPin(pinNumber[, callback])
* `pinNumber: Number` - pin number to read.
* `callback: Function(err: Error | null, value: Boolean)`.

Read boolean value from a GPIO pin.

`callback` will be called with the value.

'readPin' event will be emitted after I/O finishes.


#### gpio.setPort(portNumber, direction[, mode][, callback])
* `portNumber: Number` - port number to configure.
* `direction: 'in' | 'out' | 'none'` - direction of the port.
* `mode: String` - pin mode.
* `callback: Function(err: Error | null)`.

Set GPIO port configuration.

All pins bound to this port will have the given configuration. 

`callback` will be called after GPIO port is set.

'setport' event will be emitted after port is set.

'setpin' event will be emitted for pins that are bound to the port after port is set.


#### gpio.writePort(portNumber, value[, callback])
* `portNumber: Number` - port number to write
* `value: Number`.
* `callback: Function(err: Error | null)`.

Write out the given value to a GPIO port.

`callback` will be called after the value flushed.


#### gpio.readPort(portNumber, callback)
* `portNumber: Number` - port number to read.
* `callback: Function(err: Error | null, value: Number)`.

Read value from a GPIO port.

`callback` will be called with the value.


#### gpio.query(queryOption, callback)
* `queryOption: Object`.
* `callback: Function(err: Error | null, result: Object)`.
* _Need mode discussion for this_

### Events

#### `'initialize'`

Emitted after GPIO device is successfully initialized.

#### `'release'`

Emitted after GPIO device is successfully released.

#### `'setPin(pin: Number, direction: String, mode:String'`

Emitted after GPIO pin is set.

#### `'writePin(pin: Number, value: Boolean)'`

Emitted after GPIO write pin is finished.

#### `'readPin(pin: Number, value: Boolean)'`

Emitted after GPIO read pin is finished.

#### `'error(error: GpioError)'`

Emitted when there is an error.

### class: GpioError

`GpioError` is for represent error occurs during GPIO processing.
`GpioError` inherits `Error`.

