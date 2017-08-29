### Platform Support

The following shows timer module APIs available for each platform.

|  | Linux<br/>(Ubuntu) | Raspbian<br/>(Raspberry Pi) | NuttX<br/>(STM32F4-Discovery) | TizenRT<br/>(Artik053) |
| :---: | :---: | :---: | :---: | :---: |
| setTimeout | O | O | O | - |
| clearTimeout | O | O | O | - |
| setInterval | O | O | O | - |
| clearInterval | O | O | O | - |


# Timers

The `timer` module exposes a global API for scheduling functions to be called at some future period of time. Because the timer functions are globals, there is no need to call require('timers') to use the API.


### setTimeout(callback, delay[, args..])

* `callback` {Function} The function to call when the timer elapses.
  * `...args` {any}
* `delay` {number} The number of milliseconds to wait before calling the `callback`.
* `...args` {any} - Optional arguments to pass when the `callback` is called.
* Returns: {Timeout}

Schedules execution of a one-time `callback` after `delay` milliseconds. Returns a `Timeout` for use with `clearTimeout()`. If `callback` is not a function, a `TypeError` will be thrown.

**Example**

```js
var timeout = setTimeout(function() {
  // Do something which will be executed after one second.
}, 1000);
```

### clearTimeout(timeout)

* `timeout` {Timeout} A Timeout object returned by `setTimeout()`.

Cancels a `Timeout` object created by `setTimeout()`.

**Example**

```js
var timeout = setTimeout(function() { }, 1000);
...
clearTimeout(timeout);
```

### setInterval(callback, delay[, args..])
* `callback` {Function} The function to call when the timer elapses.
  * `...args` {any}
* `delay` {number} The number of milliseconds to wait before calling the `callback`.
* `...args` {any} - Optional arguments to pass when the `callback` is called.
* Returns: {Timeout}

Schedules repeated execution of `callback` every `delay` milliseconds. Returns a `Timeout` object for use with `clearInterval()`. If `callback` is not a function, a `TypeError` will be thrown.

**Example**

```js
var timeout = setInterval(function() {
  // Do something which will be executed repeatadly one time per second.
}, 1000);
```

### clearInterval(timeout)

* `timeout` {Timeout} A Timeout object as returned by `setInterval()`.

Cancels a `Timeout` object created by `setInterval()`.

**Example**

```js
var timeout = setInterval(function() { }, 1000);
...
clearInterval(timeout);
```

## Class: Timeout

This object is created internally and is returned from `setTimeout()` and `setInterval()`.


### timeout.ref()

When called, requests that the IoT.js event loop should not exit as long as the `Timeout` is active.

### timeout.unref()

When called, the active `Timeout` object will not force the IoT.js event loop to remain active. If there are no other scheduled activites, the process may exit, the process may exit before the `Timeout` object's callback is invoked.
