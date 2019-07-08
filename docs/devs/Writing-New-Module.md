# How to write a new module

This document provides a guide on how to write a module for IoT.js.

Contents
* Writing JavaScript Module
* Writing Native Module
  * Platform dependent native parts
  * Native handler
    * Arguments and Return
    * Wrapping native object with JS object
    * Callback
* Writing "Mixed" Module
  * Using native module in JavaScript module
* Advanced usage
  * Module specific CMake file
  * Writing Dynamically loadable modules (N-API)
* Module structure generator

See also:
* [Inside IoT.js](Inside-IoT.js.md)
* [Native Module vs. JS module](Native-Module-vs-JS-Module.md)
* [Optimization Tips](Optimization-Tips.md)
* [Developer Tutorial](Developer-Tutorial.md)

## Writing JavaScript Module

JavaScript module can be written in the same way as writing [Node.js module](https://nodejs.org/api/modules.html). JavaScript file should be located anywhere on your filesystem.

* Use `./tools/build.py --external-modules=my-module` when building
* Enable your module in a profile or as an additional CMake parameter

**Important:** the name of the module must be in lowercase. It is not allowed to use uppercase characters.

Your new module will look like below:

my-module/js/mymodule.js:
```javascript
module.exports = {
    foo: function() { console.log("OK"); },
    bar: 123
}
```

my-module/modules.json:
```json
{
  "modules": {
    "mymodule": {
      "js_file": "js/mymodule.js",
      "require": ["buffer", "console"]
    }
  }
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
$ ./tools/build.py --external-modules=./my-module --cmake-param=-DENABLE_MODULE_MYMODULE=ON
$ ${PATH_TO}/iotjs user.js
OK
123
```

**Note**: `--cmake-param=-DENABLE_MODULE_MYMODULE=ON` option must be used in case of an
external module, because the default profile enables only the basic and core modules.


### ENABLE_MODULE_[NAME]

An uppercase `ENABLE_MODULE_[NAME]` CMake variable will be generated for every module,
where `NAME` is the name of the module in the `modules.json`. To enable or disable a
module by setting the corresponding `ENABLE_MODULE_[NAME]` to ON or OFF. It will override
the defult settings of the profile.

### Profile

The purpose of the "profile" is to describe the default settings of enabled modules for
the build. A profile file is a list of `ENABLE_MODULE_[NAME]` macros. Those module whos
`ENABLE_MODULE_[NAME]` macro is not listed will be disabled by defult.

my-module/mymodule.profile:
```
ENABLE_MODULE_IOTJS_BASIC_MODULES
ENABLE_MODULE_IOTJS_CORE_MODULES
ENABLE_MODULE_MYMODULE
```

Execute:
```bash
./tools/build.py --external-modules=./my-module --profile=my-module/mymodule.profile
```


## Writing Native Module

You can implement some part of the builtin module in C, to enhance performance and to fully exploit the H/W functionality, etc. It has similar concept with [Node.js native addon](https://nodejs.org/api/addons.html), but we have different set of APIs. Node.js uses its own binding layer with v8 API, but we use [our own binding layer](../../src/iotjs_binding.h) which wraps [JerryScript API](https://github.com/jerryscript-project/jerryscript/blob/master/jerry-core/jerryscript.h). You can see `src/iotjs_binding.*` files to find more APIs to communicate with JS-side values from native-side of you can call JerryScript API functions directly.

* For native modules you must define an `init` function that provides the JS object that represents your module.
* You can define multiple native files.
* Directory of your module will be added to the include path.
* Use `./tools/build.py --external-modules=my-module` when building.
* Enable your module in a profile or as an additional CMake parameter.

Your new module will look like below:

my-module/my_module.c:
```javascript
#include "iotjs_def.h"

jerry_value_t InitMyNativeModule() {
  jerry_value_t mymodule = jerry_create_object();
  iotjs_jval_set_property_string_raw(mymodule, "message", "Hello world!");
  return mymodule;
}
```

my-module/modules.json:
```json
{
  "modules": {
    "mymodule": {
      "native_files": ["my_module.c"],
      "init": "InitMyNativeModule"
    }
  }
}
```

user.js:
```javascript
var mymodule = require('mymodule');
console.log(mymodule.message); // prints "Hello world!"
```

and execute:
```sh
$ ./tools/build.py --external-modules=./my-module --cmake-param=-DENABLE_MODULE_MYMODULE=ON
$ ${PATH_TO}/iotjs user.js
Hello world!
```

### Platform dependent native parts

You can define the platform dependent low level parts in the `modules.json`.

Structure of the directory of the custom module:
```
my_module
 |-- linux
      |-- my_module_platform_impl.c
 |-- nuttx
      |-- my_module_platform_impl.c
 |-- tizenrt
      |-- my_module_platform_impl.c
 |-- other
      |-- my_module_platform_impl.c
 |-- modules.json
 |-- my_module.h
 |-- my_module.c
```

modules.json:
```json
{
  "modules": {
    "mymodule": {
      "platforms": {
        "linux": {
          "native_files": ["linux/my_module_platform_impl.c"]
        },
        "nuttx": {
          "native_files": ["nuttx/my_module_platform_impl.c"]
        },
        "tizenrt": {
          "native_files": ["tizenrt/my_module_platform_impl.c"]
        },
        "undefined": {
          "native_files": ["other/my_module_platform_impl.c"]
        }
      },
      "native_files": ["my_module.c"],
      "init": "InitMyModule"
    }
  }
}
```

**Note**: Undefined platform means a general implementation. If the module does not support your platform then it will use the `undefined` platform implementation.

### Library dependency

It is possible that the external module depends/requires an already compiled third-party shared object or static library.
Such libraries can be specified in the `modules.json` file so they will be linked when the IoT.js module is used.
To specify third-party libraries the `external_libs` key should be used in the module specification.

For example in the `modules.json`:

```json
{
  "modules": {
    "mymodule": {
      "platforms": {
        "linux": {
          "native_files": ["linux/my_module_platform_impl.c"],
          "external_libs": ["curl"]
        }
      },
      "native_files": ["my_module.c"],
      "external_libs": ["lib_shared_on_all_platforms_if_it_truly_exists"],
      "init": "InitMyNativeModule"
    }
  }
}
```

The `external_libs` key can be specified on the module level or for each platform also.

### Native handler

Native handler reads arguments from JavaScript, executes native operations, and returns the final value to JavaScript.

#### Arguments and Return

Let's see an example in `src/module/iotjs_module_console.c`:

```c
JS_FUNCTION(Stdout) {
  DJS_CHECK_ARGS(1, string);

  iotjs_string_t msg = JS_GET_ARG(0, string);
  fprintf(stdout, "%s", iotjs_string_data(&msg));
  iotjs_string_destroy(&msg);

  return jerry_create_undefined();
}
```

Using `JS_GET_ARG(index, type)` macro inside `JS_FUNCTION()` will read JS-side argument. Since JavaScript values can have dynamic types, you must check if argument has valid type with `DJS_CHECK_ARGS(number_of_arguments, type1, type2, type3, ...)` macro, which throws JavaScript TypeError when given condition is not satisfied.

`JS_FUNCTION()` must return with an `jerry_value_t` into JS-side.

#### Wrapping native object with JS object

`console` module is *state-free* module, i.e., console module implementation doesn't have to hold any values with it. It just passes value and that's all it does.

However, there are many cases that module should maintain its state. Maintaining the state in JS-side would be simple. But maintaining values in native-side is not an easy problem, because native-side values should follow the lifecycle of JS-side values. Let's take `Buffer` module as an example. `Buffer` should maintain the native buffer content and its length. And the native buffer content should be deallocated when JS-side buffer variable becomes unreachable.

There's `iotjs_jobjectwrap_t` struct for that purpose. if you create a new `iotjs_jobjectwrap_t` struct with JavaScript object as its argument and free handler, the registered free handler will be automatically called when its corresponding JavaScript object becomes unreachable. `Buffer` module also exploits this feature.

```c
// This wrapper refer javascript object but never increase reference count
// If the object is freed by GC, then this wrapper instance will be also freed.
typedef struct {
  jerry_value_t jobject;
} iotjs_jobjectwrap_t;

typedef struct {
  iotjs_jobjectwrap_t jobjectwrap;
  char* buffer;
  size_t length;
} iotjs_bufferwrap_t;

static void iotjs_bufferwrap_destroy(iotjs_bufferwrap_t* bufferwrap);
IOTJS_DEFINE_NATIVE_HANDLE_INFO(bufferwrap);

iotjs_bufferwrap_t* iotjs_bufferwrap_create(const jerry_value_t* jbuiltin,
                                            size_t length) {
  iotjs_bufferwrap_t* bufferwrap = IOTJS_ALLOC(iotjs_bufferwrap_t);
  iotjs_jobjectwrap_initialize(&_this->jobjectwrap,
                               jbuiltin,
                               &bufferwrap_native_info); /* Automatically called */
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
const jerry_value_t* jbuiltin = /*...*/;
iotjs_bufferwrap_t* buffer_wrap = iotjs_bufferwrap_create(jbuiltin, length);
// Now `jbuiltin` object can be used in JS-side,
// and when it becomes unreachable, `iotjs_bufferwrap_destroy` will be called.
```

#### Callback

Sometimes native handler should call JavaScript function directly. For general function calls (inside current tick), you can use `iotjs_jhelper_call()` function to call JavaScript function from native-side.

And for asynchronous callbacks, after `libtuv` calls your native function, if you want to call JS-side callback you should use `iotjs_make_callback()`. It will not only call the callback function, but also handle the exception, and process the next tick(i.e. it will call `iotjs_process_next_tick()`).

For asynchronous callbacks, you must consider the lifetime of JS-side callback objects. The lifetime of JS-side callback object should be extended until the native-side callback is really called. You can use `iotjs_reqwrap_t` and `iotjs_handlewrap_t` to achieve this.

## Writing "Mixed" Modules

Modules could be a combination of JS and native code. In that case the Javascript file must
export the objects of the module. In such cases the native part will be hidden.

For simple explanation, `console` module will be used as an example.

```
src
 |-- js
      |-- console.js
 |-- modules
      |-- iotjs_module_console.c
      |-- modules.json
```

modules.json
```json
{
  "modules": {
    ...
    "console": {
      "native_files": ["modules/iotjs_module_console.c"],
      "init": "InitConsole",
      "js_file": "js/console.js",
      "require": ["util"]
    },
    ...
  }
}
```

### Using native module in JavaScript module

Logging to console needs native functionality, so `console` JavaScript module in `src/js/console.js` passes its arguments into native handler like:

```javascript
Console.prototype.log = native.stdout(util.format.apply(this, arguments) + '\n');
```

Where `native` is the JS object returned by the native `InitConsole` function in `iotjs_module_console.c`.

**Note**: `native` is undefined if there is no native part of the module.


### Using bridge module to communicate between C and JavaScript module
Bridge module provides two interfaces for sending synchronous and asynchronous message from Javascript to the native module. The Native module simply rersponds back to the requst using a simple inteface that can create return message. Of course you can use the IoT.js and JerryScript APIs to respond directly to the request of JavaScript module, but sometimes using a simpliffied method is more efficient in providing simple functionality in a short time.

For example, JavaScript module can request resource path synchronously,
and native module can simply return a resource path by just calling a function.

in the bridge_sample.js of bridge_sample module
```javascript
bridge_sample.prototype.getResPath = function(){
    return this.bridge.sendSync("getResPath", "");
};
```

in the iotjs_bridge_sample.c of bridge_sample module
```c
if (strncmp(command, "getResPath", strlen("getResPath")) == 0) {
    iotjs_bridge_set_return(return_message, "res/");
    return 0;
}
```
For the complete sample code, please see the bridge_sample in samples/bridge_sample folder.

## Advanced usage

### Module specific CMake file

For each module, it is possible to define one extra cmake file.
This can be done by specifying the `cmakefile` key file for
a module in the related `modules.json` file.

For example:

```json
{
  "modules": {
    "demomod": {
      ...
      "cmakefile": "module.cmake"
    }
  }
}
```

This `module.cmake` is a module-specific CMake file
which will be searchd for in the module's base directory.
In this file it is possible to specify additonal dependecies,
build targets, and other things.

However, there are a few important rules which must be followed in
the CMake file:

* The `MODULE_DIR` and `MODULE_BINARY_DIR` will be set by
  the IoT.js build system. Do NOT overwrite them in the CMake file!

* The `MODULE_NAME` CMake variable must be set.
  Example: `set(MODULE_NAME "demomod")`

* The `add_subdirectory` call must specify the output binary dir.
  Please use this template:
`add_subdirectory(${MODULE_DIR}/lib/ ${MODULE_BASE_BINARY_DIR}/${MODULE_NAME})`
  where `lib/` is a subdirectory of the module directory.

* If there is an extra library which should be used during linking, the
  following template should be used:
  `list(APPEND MODULE_LIBS demo)`
  where `demo` is the extra module which must be linked.
  Any number of modules can be appended to the `MODULE_LIBS` list variable.

* The source files which are specified in the `modules.json` file must NOT
  be specified in the CMake file.


An example module CMake file:
```
set(MODULE_NAME "mymodule")

add_subdirectory(${MODULE_DIR}/myLib/ ${MODULE_BASE_BINARY_DIR}/${MODULE_NAME})

list(APPEND MODULE_LIBS myLib)
```

To ease creation of modules which contains extra CMake files
there is a module generator as described below.


## Writing Dynamically loadable modules (N-API)

IoT.js support N-API for building and loading native addons.
To create such modules the source files must be compiled into
a shared object and must have a special entry point defined.

See also:
  * [N-API in IoT.js](../api/IoT.js-API-N-API.md)
  * [N-API module registration](https://nodejs.org/docs/latest-v10.x/api/n-api.html#n_api_module_registration) macro.

N-API modules are registered in a manner similar to other modules
except that instead of using the `NODE_MODULE` macro the following
is used:

```C
NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)
```

The next difference is the signature for the `Init` method. For a N-API
module it is as follows:

```C
napi_value Init(napi_env env, napi_value exports);
```

The return value from `Init` is treated as the `exports` object for the module.
The `Init` method is passed an empty object via the `exports` parameter as a
convenience. If `Init` returns NULL, the parameter passed as `exports` is
exported by the module. N-API modules cannot modify the `module` object but can
specify anything as the `exports` property of the module.

To add the method `hello` as a function so that it can be called as a method
provided by the addon:

```C
napi_value Init(napi_env env, napi_value exports) {
  napi_status status;
  napi_property_descriptor desc =
    {"hello", NULL, Method, NULL, NULL, NULL, napi_default, NULL};
  status = napi_define_properties(env, exports, 1, &desc);
  if (status != napi_ok) return NULL;
  return exports;
}
```

To set a function to be returned by the `require()` for the addon:

```C
napi_value Init(napi_env env, napi_value exports) {
  napi_value method;
  napi_status status;
  status = napi_create_function(env, "exports", NAPI_AUTO_LENGTH, Method, NULL, &method);
  if (status != napi_ok) return NULL;
  return method;
}
```

To define a class so that new instances can be created (often used with
[Object Wrap][]):

```C
// NOTE: partial example, not all referenced code is included
napi_value Init(napi_env env, napi_value exports) {
  napi_status status;
  napi_property_descriptor properties[] = {
    { "value", NULL, NULL, GetValue, SetValue, NULL, napi_default, NULL },
    DECLARE_NAPI_METHOD("plusOne", PlusOne),
    DECLARE_NAPI_METHOD("multiply", Multiply),
  };

  napi_value cons;
  status =
      napi_define_class(env, "MyObject", New, NULL, 3, properties, &cons);
  if (status != napi_ok) return NULL;

  status = napi_create_reference(env, cons, 1, &constructor);
  if (status != napi_ok) return NULL;

  status = napi_set_named_property(env, exports, "MyObject", cons);
  if (status != napi_ok) return NULL;

  return exports;
}
```

If the module will be loaded multiple times during the lifetime of the Node.js
process, use the `NAPI_MODULE_INIT` macro to initialize the module:

```C
NAPI_MODULE_INIT() {
  napi_value answer;
  napi_status result;

  status = napi_create_int64(env, 42, &answer);
  if (status != napi_ok) return NULL;

  status = napi_set_named_property(env, exports, "answer", answer);
  if (status != napi_ok) return NULL;

  return exports;
}
```

This macro includes `NAPI_MODULE`, and declares an `Init` function with a
special name and with visibility beyond the addon. This will allow IoT.js to
initialize the module even if it is loaded multiple times.


## Module structure generator

As previously shown, there are a few files required to create a module.
These files can be createad manually or by the `tools/iotjs-create-module.py`
script.

The module generator can generate two types of modules:
* basic built-in module which is compiled into the IoT.js binary.
* shared module which can be dynamically loaded via the `require` call.

To generate a module with the IoT.js module generator
the module template should be specified and the name of the new module.

**Important note:** The module name must be in lowercase.

The `template` paramter for the module creator is optional, if it is
not specified basic modules are created.

The generated module(s) have simple examples in it which can be used
to bootstrap ones own module(s). On how to use them please see the
previous parts of this document.

### Basic module generation

Example basic module generation:
```
$ python ./iotjs/tools/iotjs-create-module.py --template basic demomod
Creating module in ./demomod
loading template file: ./iotjs/tools/module_template/module.cmake
loading template file: ./iotjs/tools/module_template/modules.json
loading template file: ./iotjs/tools/module_template/js/module.js
loading template file: ./iotjs/tools/module_template/src/module.c
Module created in: /mnt/work/demomod
```

By default the following structure will be created by the tool:

```
demomod/
 |-- js
      |-- module.js
 |-- module.cmake
 |-- modules.json
 |-- src
      |-- module.c
```

### Shared (N-API) module generation

Example shared module generation:
```
$ python ./iotjs/tools/iotjs-create-module.py --template shared demomod
Creating module in ./demomod
loading template file: ./iotjs/tools/module_templates/shared_module_template/CMakeLists.txt
loading template file: ./iotjs/tools/module_templates/shared_module_template/README.md
loading template file: ./iotjs/tools/module_templates/shared_module_template/js/test.js
loading template file: ./iotjs/tools/module_templates/shared_module_template/src/module_entry.c
Module created in: /mnt/work/demomod
```

The generated `demomod` will have a `CMakeLists.txt` file which contains
path variables to the IoT.js headers and JerryScript headers. These path
variables are absolute paths and required to the module compilation.
Please adapt the paths if required.

Additionnally the `README.md` file contains basic instructions on
how to build and test the new module.
