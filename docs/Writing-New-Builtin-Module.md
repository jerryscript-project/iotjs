Writing New Builtin Module
==========================

This document provides a guide on how to write a builtin module for IoT.js.

Contents:

* Step-by-Step Guide
  - Writing Builtin JavaScript Module
  - Writing Native Module Builtin

* And More Informations
  - Implementation Details
  - Optimization Tips

It will be easier to write a new IoT.js module if you have background on:

- [Node.js module](https://nodejs.org/api/modules.html) (for writing IoT.js JavaScript module)
- [Node.js native addon](https://nodejs.org/api/addons.html) (for writing IoT.js native module builtin)

# Step-by-Step Guide

## Writing Builtin JavaScript Module

Builtin JavaScript module can be written in the same way as writing [Node.js module](https://nodejs.org/api/modules.html). JavaScript file should be located in `src/js/` directory, then our build script will automatically include your module into the final binary.

src/js/mymodule.js:
```javascript
module.exports = {
    foo: function() { console.log("OK"); }
    bar: 123;
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

You can implement some part of the builtin module in C++, to enhance performance and to fully exploit the H/W functionality, etc. It has same concept with [Node.js native addon](https://nodejs.org/api/addons.html), but we have different set of APIs. Node.js uses its own binding layer with v8 API, but we use [our own binding layer](https://github.com/Samsung/iotjs/blob/master/src/iotjs_binding.h) which wraps [JerryScript API](https://github.com/Samsung/JerryScript/blob/master/jerry-core/jerry-api.h).

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
```c++
#define MAP_MODULE_LIST(F) \
  F(BUFFER, Buffer, buffer) \
  F(CONSOLE, Console, console) \
  F(CONSTANTS, Constants, constants) \
  ...
```

Then `JObject* Init##ModuleName()` function will be called automatically when registering native module. We already have its implementation in `src/iotjs_module_console.cpp`:
```c++
JObject* InitConsole() {
  Module* module = GetBuiltinModule(MODULE_CONSOLE);
  JObject* console = module->module;

  if (console == NULL) {
    console = new JObject();
    console->SetMethod("stdout", Stdout);
    console->SetMethod("stderr", Stderr);

    module->module = console;
  }

  return console;
}
```
The return value of initializer function (in this case, `JObject* console`,) will be passed to JavaScript side, as a return value of calling `process.binding(process.binding.modulename)`. Simply calling `new JObject()` will create a JavaScript object in c++ code.

And you might want to define some functions and properties to the newly created object. `JObject::SetMethod()` will register a native handler as a JavaScript function property. (That's how we called `consoleBuiltin.stdout()` in JavaScript.) And `JObject::SetProperty()` will define a non-function property into object. You can find the example of registering a constant value as a JavaScript property in `src/iotjs_module_constants.cpp`.

### Native handler

Native handler reads arguments from JavaScript, executes native operations(, and returns the final value to JavaScript if necessary). Let's see an example in `src/iotjs_module_console.cpp`:
```c++
JHANDLER_FUNCTION(Stdout) {
  JHANDLER_CHECK(handler.GetArgLength() == 1);
  JHANDLER_CHECK(handler.GetArg(0)->IsString());

  String msg = handler.GetArg(0)->GetString();
  fprintf(stdout, "%s", msg.data());
}
```

Calling `JObject* JHandlerInfo::GetArg()` method inside `JHANDLER_FUNCTION()` will read JS-side argument. Since JavaScript values can have dynamic types, you must check if argument has valid type. You can use `JHANDLER_CHECK` macro, which throws JavaScript TypeError when given condition is not satisfied. After validating the type, you can convert the argument into the type you want. In this case, to print the arguments as string,

Calling `void JHandlerInfo::Return()` method inside `JHANDLER_FUNCTION()` will return value into JS-side. (`undefined` will be returned if `Return` was not called.) You should wrap c++ values into `JObject` or `JVal` type. Console methods doesn't have to return values, but you can easily find more examples from other modules.

See `src/iotjs_binding.*` files to find more APIs to validate and convert the type of JavaScript values.


# More Informations

## Implementation Details

### GC-referencing JavaScript object in c++

JerryScript uses automatic memory management and IoT.js uses manual memory management. You can use `JObjectWrap` between two layers.

(Work in progress)

### Module design: singleton vs non-singleton

You can refer to Node.js module design patterns.

(Work in progress)

## Optimization Tips

### Tracing JerryScript heap usage

Adding below arguments when building and running IoT.js will show you the JerryScript memory status.

```text
$ ./tools/build.py --jerry-memstat
$ ./build/bin/iotjs test.js --memstat
Heap stats:
  Heap size = 262136 bytes
  Allocated = 0 bytes
  Waste = 0 bytes
  Peak allocated = 24288 bytes
  Peak waste = 261 bytes
  Skip-ahead ratio = 2.6059
  Average alloc iteration = 1.1284
  Average free iteration = 19.3718

Pools stats:
  Pool chunks: 0
  Peak pool chunks: 735
  Free chunks: 0
  Pool reuse ratio: 0.3459
```

Note that currently only JerryScript heap usage can be shown, not IoT.js heap usage.

### JerryScript 'external magic string' feature

When parsing and executing JavaScript module, JavaScript strings occupy a huge amount of space in JerryScript heap. To optimize this kind of heap usage, JerryScript has 'external magic string' feature. If you enable snapshot when building, build script will automatically generate `src/iotjs_string_ext.inl.h` file, which includes all of the JavaScript strings used in builtin modules. This file is used by JerryScript to reduce heap usage.

Since same strings will be included only once, you can use this information to get some hints on binary size reduction. Note that only strings with length<32 will be included in this list.
