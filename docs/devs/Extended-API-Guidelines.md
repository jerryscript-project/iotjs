Basically, our basic APIs are based on node.js. They will follow same form with node.js because of compatibility.<br>
However, extended APIs need a guideline because they are implemented by many contributor. (for consistent usability)


# Ground Rules

## API naming rules
1. The APIs which have similar role should have same API name. 
2. Basically, all APIs are async API. If you want to make sync API, you need to add `Sync` as a suffix.<br>For example, `readSync()`, `writeSync()`, and so on.

## Generating an object 
1. The module object should be generated using `open()` API for consistent usability.
2. `open()` API should have configurable as first argument and callback function as second argument.<br>callback function is always optional.

For example, GPIO module generate an object like below:
```javascript
var Gpio = require('gpio');
var gpio = new Gpio();
var gpio10 = gpio.open({pin: 10, direction: gpio.DIRECTION.OUT},
                       function(err){console.log(err);});
gpio10.writeSync(1);


```

## Minimize event generation
1. The response of the API call uses callback function.
2. Only generate event when user need to know something without any API call.
3. The event which have similar role should have same event name.

## Error generation
1. `error` can be generated in both JS/native side.
2. The `error` shoud be created in the place where it occurs.

For example, error can be generated like below:

In native side,
```c
iotjs_jargs_t jargs = iotjs_jargs_create(2);

// kGpioErrRead: int
if (result == kGpioErrRead) {
  iotjs_jargs_append_error_with_code(&jargs, "GPIO Error", kGpioErrRead);
}
```

In JavaScript side,
```javascript
if (!util.isNumber(value)) {
  throw new TypeError('Bad arguments');
}
```

## Type checking
1. Type checking of API argument is possible both in JS and Native.
2. To prevent type checking multiple times, perform type checking on the first function that receives the variable.
3. Throw `error` when type checking is failed.


# Recommended Rules
1. Call `close()` api when process module occur `exit` event.
2. If it is possible, use the functions provided by `libtuv` (File open, read, write, etc.)
3. Callback function in API argument should be always optional.
