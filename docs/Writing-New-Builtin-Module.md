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

You can implement some part of the builtin module in C++, to enhance performance and to fully exploit the H/W functionality, etc. It has same concept with [Node.js native addon](https://nodejs.org/api/addons.html), but we have different set of APIs. Node.js uses its own binding layer with v8 API, but we use [our own binding layer](https://github.com/Samsung/iotjs/blob/master/src/iotjs_binding.h) which wraps [JerryScript API](https://github.com/Samsung/JerryScript/blob/master/jerry-core/jerry-api.h). You can see `src/iotjs_binding.*` files to find more APIs to communicate with JS-side values from native-side.

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
The return value of initializer function (in this case, `JObject* console`,) will be passed to JS-side, as a return value of calling `process.binding(process.binding.modulename)`. Simply calling `new JObject()` will create a JavaScript object in c++ code.

And you might want to define some functions and properties to the newly created object. `JObject::SetMethod()` will register a native handler as a JavaScript function property. (That's how we called `consoleBuiltin.stdout()` in JavaScript.) And `JObject::SetProperty()` will define a non-function property into object. You can find the example of registering a constant value as a JavaScript property in `src/iotjs_module_constants.cpp`.

### Native handler

Native handler reads arguments from JavaScript, executes native operations, and returns the final value to JavaScript.

#### Arguments and Return

Let's see an example in `src/iotjs_module_console.cpp`:

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

#### Wrapping native object with JS object

`console` module is *state-free* module, i.e., console module implementation doesn't have to hold any values with it. It just passes value and that's all it does.

However, there are many cases that module should maintain its state. Maintaining the state in JS-side would be simple. But maintaining values in native-side is not an easy problem, because native-side values should follow the lifecycle of JS-side values. Let's take `Buffer` module as an example. `Buffer` should maintain the native buffer content and its length. And the native buffer content should be deallocated when JS-side buffer variable becomes unreachable.

There's `JObjectWrap` class for that purpose. if you create a new `JObjectWrap` instance with JavaScript object as its argument, its destructor will be automatically called when its corresponding JavaScript object becomes unreachable. `Buffer` module also exploits this feature.

```
// This wrapper refer javascript object but never increase reference count
// If the object is freed by GC, then this wrapper instance will be also freed.
class JObjectWrap {
 public:
  explicit JObjectWrap(JObject& jobject);
  virtual ~JObjectWrap();
  ...
};

class BufferWrap : public JObjectWrap {
 public:
  BufferWrap(JObject& jbuiltin, size_t length);
  virtual ~BufferWrap(); // destructor will deallocate native buffer
  ...
 protected:
  char* _buffer;
  size_t _length;
};
```

You can use this class like below:

```
JObject* jbuiltin = /*...*/;
BufferWrap* buffer_wrap = new BufferWrap(*jbuiltin, length);
// Now `jbuiltin` object can be used in JS-side,
// and when it becomes unreachable, destructor of `buffer_wrap` will be called.
```

#### Callback

Sometimes native handler should call JavaScript function directly. For general function calls (inside current tick), you can use `iotjs::JObject::Call(JObject& this_, JArgList& arg)` function to call JavaScript function from native-side.

And for asyncronous callbacks, after `libtuv` calls your native function, if you want to call JS-side callback you should use `iotjs::MakeCallback(JObject& function, JObject& this_, JArgList& args)`. It will not only call the callback function, but also handle the exception, and process the next tick(i.e. it will call `ProcessNextTick()`).

For asyncronous callbacks, you must consider the lifetime of JS-side callback objects. The lifetime of JS-side callback object should be extended until the native-side callback is really called. You can use `ReqWrap` and `HandleWrap` to achieve this.

(Work In Progress)
