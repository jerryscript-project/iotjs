## class: Timers

### Methods

#### setTimeout(callback, delay[, args..])
* `callback: Function([args..])`
* `delay: Int`
* Return: `timeoutObject`

Schedules to call one-time to `callback` function after `delay` milliseconds.

#### clearTimeout(timeoutObject)
* `timeoutObject: timeoutObject` - A `timeoutObject` returned from `setTimeout()`

Disables timeout call from `setTimeout` method.

#### setInterval(callback, delay[, args..])
* `callback: Function([args..])`
* `delay: Int`
* Return: `intervalObject`

Schedules to call periodic to `callback` function after every `delay` milliseconds.

#### clearInterval(intervalObject) 
* `intervalObject: intervalObject` - A `intervalObject` returned from `setInterval`

Disables timeout call from `setInterval` method.
