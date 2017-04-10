### Platform Support

The following shows timer module APIs available for each platform.

|  | Linux<br/>(Ubuntu) | Raspbian<br/>(Raspberry Pi) | Nuttx<br/>(STM32F4-Discovery) |
| :---: | :---: | :---: | :---: |
| setTimeout | O | O | O |
| clearTimeout | O | O | O |
| setInterval | O | O | O |
| clearInterval | O | O | O |

### Contents

- [Timer](#timer)
    - [Class: Timeout](#class-timeout)
        - [Prototype Functions](#prototype-functions)
            - [`activate()`](#activate)
            - [`close()`](#close)
    - [Module Functions](#module-functions)
        - [`setTimeout(callback, delay[, args..])`](#settimeoutcallback-delay-args)
        - [`clearTimeout(timeoutObject)`](#cleartimeouttimeoutobject)
        - [`setInterval(callback, delay[, args..])`](#setintervalcallback-delay-args)
        - [`clearInterval(intervalObject)`](#clearintervalintervalobject)

# Timer

The timer module exposes a global API for scheduling functions to be called at some future period of time. Because the timer functions are globals, there is no need to call require('timers') to use the API.

# Class: Timeout

This object is created internally and is returned from setTimeout() and setInterval().

## Prototype Functions

### `activate()`

When called, requests that the IoT.js event loop not exit so long as the Timeout is active.

### `close()`

When called, the active Timeout object will not require the IoT.js event loop to remain active. If there is no other activity keeping the event loop running, the process may exit before the Timeout object's callback is invoked.

## Module Functions

### `setTimeout(callback, delay[, args..])`
* `callback <Function([args..])>`
* `delay <Number>`
* `...args <any>` - Optional arguments to pass when the callback is called.
* Returns: `<Timeout>`

Schedules to call one-time to `callback` function after `delay` milliseconds.

### `clearTimeout(timeoutObject)`
*  `timeoutObject <Timeout>`

Disables a `Timeout` object from `setTimeout` method.

### `setInterval(callback, delay[, args..])`
* `callback <Function([args..])>`
* `delay <Number>`
* `...args <any>` - Optional arguments to pass when the callback is called.
* Returns: `<Timeout>`

Schedules repeated execution of `callback` every `delay` milliseconds. If callback is not a function, a `TypeError` will be thrown.

### `clearInterval(intervalObject)`
* `intervalObject <Timeout>`

Disables a `Timeout` object from `setInterval` method.
