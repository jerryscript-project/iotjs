Writing New Builtin Module
==========================

This document provides a guide on how to write a builtin module for IoT.js.

Contents:

* Writing Builtin JavaScript Module
* Writing Native Module Builtin
  - Using native module in JavaScript module
  - Registering native module
  - Native handler
    * Arguments and Return
    * Wrapping native object with JS object
    * Callback

You can see more information on the [Optimization Tips](Optimization-Tips.md) page.

It will be easier to write a new IoT.js module if you have background on:

- [Node.js module](https://nodejs.org/api/modules.html) (for writing IoT.js JavaScript module)
- [Node.js native addon](https://nodejs.org/api/addons.html) (for writing IoT.js native module builtin)

## Writing Builtin JavaScript Module

Builtin JavaScript module can be written in the same way as writing [Node.js module](https://nodejs.org/api/modules.html). JavaScript file should be located in `src/js/` directory, and you should notify to our build script that your module should be included in one of following ways:

* Use `./tools/build.py --iotjs-include-module mymodule` when building
* Add your module in `build.config` file

Your new module will look like below:

src/js/mymodule.js:
```javascript
module.exports = {
    foo: function() { console.log("OK"); },
    bar: 123
}
```

user.js:
```javascript
var mymodule = require('mymodule');
mymodule.foo();            // prints "OK"
console.log(mymodule.bar); // prints "123"
```

and execute:
```sh
$ ./tools/build.py
$ ${PATH_TO}/iotjs user.js
OK
123
```

## Writing Native Module Builtin

You can implement some part of the builtin module in C, to enhance performance and to fully exploit the H/W functionality, etc. It has similar concept with [Node.js native addon](https://nodejs.org/api/addons.html), but we have different set of APIs. Node.js uses its own binding layer with v8 API, but we use [our own binding layer](https://github.com/Samsung/iotjs/blob/master/src/iotjs_binding.h) which wraps [JerryScript API](https://github.com/Samsung/JerryScript/blob/master/jerry-core/jerry-api.h). You can see `src/iotjs_binding.*` files to find more APIs to communicate with JS-side values from native-side.

For simple explanation, `console` module will be used as an example.

### Using native module in JavaScript module

Logging to console needs native functionality, so `console` JavaScript module in `src/js/console.js` passes its arguments into native handler like:

```javascript
var consoleBuiltin = process.binding(process.binding.console);
...
Console.prototype.log = consoleBuiltin.stdout(util.format.apply(this, arguments) + '\n');
```

### Registering native module

According to the code above, `process.binding.console` should be defined before evaluating JavaScript code. IoT.js source code can automatically register native module if some functions are implemented as expected. First you should register your new module into `MODULE_LIST` macro in `src/iotjs_module.h`:
```c
#define MAP_MODULE_LIST(F) \
  E(F, BUFFER, Buffer, buffer) \
  E(F, CONSOLE, Console, console) \
  E(F, CONSTANTS, Constants, constants) \
  ...
```

Then `iotjs_jval_t Init##ModuleName()` function will be called automatically when registering native module. We already have its implementation in `src/module/iotjs_module_console.c`:
```c
iotjs_jval_t InitConsole() {
  iotjs_jval_t console = iotjs_jval_create_object();

  iotjs_jval_set_method(&console, "stdout", Stdout);
  iotjs_jval_set_method(&console, "stderr", Stderr);

  return console;
}
```
The return value of initializer function (in this case, `iotjs_jval_t console`,) will be passed to JS-side, as a return value of calling `process.binding(process.binding.modulename)`. Calling `iotjs_jval_create_object()` will create a JavaScript object in c code.

And you might want to define some functions and properties to the newly created object. `iotjs_jval_set_method()` will register a native handler as a JavaScript function property. (That's how we was able to call `consoleBuiltin.stdout()` in JavaScript.) And `iotjs_jval_set_property_*()` will define a non-function property into object. You can find the example of registering a constant value as a JavaScript property in `src/module/iotjs_module_constants.c`.

### Native handler

Native handler reads arguments from JavaScript, executes native operations, and returns the final value to JavaScript.

#### Arguments and Return

Let's see an example in `src/module/iotjs_module_console.c`:

```c
JHANDLER_FUNCTION(Stdout) {
  JHANDLER_CHECK_ARGS(1, string);

  iotjs_string_t msg = JHANDLER_GET_ARG(0, string);
  fprintf(stdout, "%s", iotjs_string_data(&msg));
  iotjs_string_destroy(&msg);
}
```

Using `JHANDLER_GET_ARG(index, type)` macro inside `JHANDLER_FUNCTION()` will read JS-side argument. Since JavaScript values can have dynamic types, you must check if argument has valid type with `JHANDLER_CHECK_ARGS(number_of_arguments, type1, type2, type3, ...)` macro, which throws JavaScript TypeError when given condition is not satisfied.

Calling `void iotjs_jhandler_return_*()` function inside `JHANDLER_FUNCTION()` will return value into JS-side. `undefined` will be returned if you didn't explicitly returned something, like normal JavaScript function does. Console methods doesn't have to return values, but you can easily find more examples from other modules.

#### Wrapping native object with JS object

`console` module is *state-free* module, i.e., console module implementation doesn't have to hold any values with it. It just passes value and that's all it does.

However, there are many cases that module should maintain its state. Maintaining the state in JS-side would be simple. But maintaining values in native-side is not an easy problem, because native-side values should follow the lifecycle of JS-side values. Let's take `Buffer` module as an example. `Buffer` should maintain the native buffer content and its length. And the native buffer content should be deallocated when JS-side buffer variable becomes unreachable.

There's `iotjs_jobjectwrap_t` struct for that purpose. if you create a new `iotjs_jobjectwrap_t` struct with JavaScript object as its argument and free handler, the registered free handler will be automatically called when its corresponding JavaScript object becomes unreachable. `Buffer` module also exploits this feature.

```c
// This wrapper refer javascript object but never increase reference count
// If the object is freed by GC, then this wrapper instance will be also freed.
typedef struct {
  iotjs_jval_t jobject;
} iotjs_jobjectwrap_t;

typedef struct {
  iotjs_jobjectwrap_t jobjectwrap;
  char* buffer;
  size_t length;
} iotjs_bufferwrap_t;

iotjs_bufferwrap_t* iotjs_bufferwrap_create(const iotjs_jval_t* jbuiltin,
                                            size_t length) {
  iotjs_bufferwrap_t* bufferwrap = IOTJS_ALLOC(iotjs_bufferwrap_t);
  iotjs_jobjectwrap_initialize(&_this->jobjectwrap,
                               jbuiltin,
                               (JFreeHandlerType)iotjs_bufferwrap_destroy); /* Automatically called */
  ...
}

void iotjs_bufferwrap_destroy(iotjs_bufferwrap_t* bufferwrap) {
  ...
  iotjs_jobjectwrap_destroy(&_this->jobjectwrap);
  IOTJS_RELEASE(bufferwrap);
}
```

You can use this code like below:

```c
const iotjs_jval_t* jbuiltin = /*...*/;
iotjs_bufferwrap_t* buffer_wrap = iotjs_bufferwrap_create(jbuiltin, length);
// Now `jbuiltin` object can be used in JS-side,
// and when it becomes unreachable, `iotjs_bufferwrap_destroy` will be called.
```

#### Callback

Sometimes native handler should call JavaScript function directly. For general function calls (inside current tick), you can use `iotjs_jhelper_call()` function to call JavaScript function from native-side.

And for asynchronous callbacks, after `libtuv` calls your native function, if you want to call JS-side callback you should use `iotjs_make_callback()`. It will not only call the callback function, but also handle the exception, and process the next tick(i.e. it will call `iotjs_process_next_tick()`).

For asynchronous callbacks, you must consider the lifetime of JS-side callback objects. The lifetime of JS-side callback object should be extended until the native-side callback is really called. You can use `iotjs_reqwrap_t` and `iotjs_handlewrap_t` to achieve this.

(Work In Progress)
