Inside IoT.js
=============

* [Design](#design)
* [Javascript Binding](#javascript-binding)
  * iotjs_jval_t
  * iotjs_jobjectwrap_t
  * Native handler
  * Embedding API
* [libuv Binding](#libuv-binding)
  * iotjs_handlewrap_t
  * iotjs_reqwrap_t
* [IoT.js Core](#iotjscoe)
  * Life cycle of IoT.js
  * Builtin
  * Native module
  * Event loop

# Design

IoT.js is built on top of [JerryScript](http://jerryscript.net/) and [libuv](http://libuv.org). JerryScript is a lightweight Javascript engine intended to run on small devices for IoT and libuv is a library for supporting asynchronous I/O. There is a layer that binds JerryScript and libuv to IoT.js.
We will deals with the layer in [Javascript Binding](#javascript-binding) and [libuv Binding](#javascript-binding) section on this document respectively.

IoT.js core layer locates above these binding layer.
This core layer plays a central role in this project providing upper layer with fundamental functionality of running main event loop, interacting with Javascript engine, managing I/O resources via libuv, managing life cycle of objects, providing builtin modules, and so forth.
[IoT.js Core](#iotjs-core) section deals with the layer in detail.

IoT.js provides APIs for user applications to help creating IoT friendly services.
You can see the list of API from [IoT.js API Reference](../api/IoT.js-API-reference.md).

# Javascript Binding

Many modern Javascript Engines come with [embedding API](#embedding-api) to provide functionality for compiling and executing Javascript program, accessing Javascript object and its value, handling errors, managing lifecyles of objects and so on.

You can think of Javascript binding layer as an interface between upper layer (IoT.js core) and  underlying Javascript engine.
Although IoT.js only supports JerryScript for now, there will be a chance that we extend supporting Javascript engine (such as [Duktape](http://duktape.org/) or [V8](https://code.google.com/p/v8/)) in the future.
For this reason, we want to keep the layer independent from a specific Javascript engine.
You can see interface of the layer in [iotjs_binding.h](../../src/iotjs_binding.h).

## iotjs_jval_t

`iotjs_jval_t` struct stands for a real Javascript object. Upper layers will access Javascript object via this struct.
This struct provides following functionalities:

* Creating a Javascript object using `iotjs_jval_create_*()` constructor.
* Creating a Javascript object by a value.
* Creating a Javascript function object where its body is implemented in C.
* Creating a Javascript Error object.
* Creating reference for a Javascript object increasing reference count.
* Increasing reference count.
* Decreasing reference count.
* Checking object type.
* Retrieving value.
* Calling a Javascript function.
* Evaluating a Javascript script.
* Set and Get corresponding native data to the Javascript object.

## iotjs_jobjectwrap_t

You can refer Javascript object from C code side using `iotjs_jval_t` as saw above.
When a reference for a Javascript object was made using `iotjs_jval_t`, it will increase the reference count and will decrease the count when it goes out of scope.

```c
{
  // Create JavaScript object
  // It increases reference count in JerryScript side.
  iotjs_jval_t jobject = iotjs_jval_create();

  // Use `jobject`
  ...

  // Before jobject goes out of scope, destroy it.
  // It decreases reference count in JerryScript side so that it can be GC-ed.
  iotjs_jval_destroy(&jobject)
}
```

But the situation is different if you want to refer a Javascript object through out program execution until the object is live.
You may write code like this:

```c
  iotjs_jval_t* jobject = (iotjs_jval_t*)malloc(sizeof(iotjs_jval_t)); // Not allowed
```

Unfortunately, we strongly do not recommend that kind of pattern. We treat pointer-types variables in special way. (See [Validated Struct](Inside-IoT.js-Validated-Struct.md) for more details.)

To achieve your wish, we recommend using `iotjs_jobjectwrap_t` for that purpose.
`iotjs_jobjectwrap_t` is kind of weak pointer to a Javascript Object.
It refers a Javascript object but never increase reference count so that Javascript engine can collect the object when it turns into garbage.
The `iotjs_jobjectwrap_t` instance will be released at the time the corresponding Javascript object is being reclaimed.

Do not hold pointer to the wrapper in native code side globally because even if you are holding a wrapper by pointer, Javascript engine probably releases the corresponding Javascript object resulting deallocation of wrapper. Consequentially your pointer will turned into dangling.

The only safe way to get wrapper is to get it from Javascript object. When a wrapper is being created, it links itself with corresponding Javascript object with `iotjs_jval_set_object_native_handle()` method of `iotjs_jval_t`. And you can get the wrapper from the object with `iotjs_jval_get_object_native_handle()` method of `iotjs_jval_t` later when you need it.


## Native handler

Some operations - such as file I/O, networking, device control, multitasking, and etc - can not be performed by pure Javascript.
IoT.js uses a mechanism called "native handler" to enable such operations performed from Javascript.
You can regard native handler as Javascript function object with its body implemented in C.

You might think it is somewhat similar to [FFI](https://en.wikipedia.org/wiki/Foreign_function_interface).
In a wide sense, it's true for native handler is for calling C function from Javascript.
But in a narrow sense, it's not true.

Usually main purpose of [FFI](https://en.wikipedia.org/wiki/Foreign_function_interface) is to call a routine written in one language from a program written in another.
After a routine was invoked, it is common that the routine just do what it is supposed to do without knowing the surrounding context except arguments.
Whereas native handler does know that it is being called from Javascript (actually it is a Javascript function although not written in Javascript) and does access surrounding Javascript execution context using [embedding API](#embedding-api).

## Embedding API

Many Javascript engines these days provide embedding API. IoT.js uses the API to create [builtin module](#builtin) and [native handler](#native-handler). See following link if you want further information about the API:
 * [JerryScript API](http://jerryscript.net/api-reference)
 * [Duktape API](http://duktape.org/api.html)
 * [V8 embedder's guide](https://developers.google.com/v8/embed)
 * [SpiderMonkey API](https://developer.mozilla.org/en-US/docs/Mozilla/Projects/SpiderMonkey/JSAPI_reference)

# libuv Binding

IoT.js is using [libuv](http://libuv.org/) to perform various asynchronous I/O and threading.
Because IoT.js adopts asynchronous programming model, libuv plays very important role in this project. Actually the [main loop](#event-loop) of IoT.js is libuv event loop waiting I/O event, picks the event, and dispatches it to corresponding event handler function.

You can read [libuv design document](http://docs.libuv.org/en/v1.x/design.html) to get detailed information about asynchronous programming model based on libuv.

## iotjs_handlewrap_t

`iotjs_handlewrap_t` is to bind a Javascript object and a libuv handle (e.g. file descriptor) together.
`iotjs_handlewrap_t` inherits `iotjs_jobjectwrap_t` since it is linked with a Javascript object.

Unlike `iotjs_jobjectwrap_t`, `iotjs_jobjectwrap_t` increases RC for the Javascript object when an instance of it is created to prevent GC while the handle is alive. The reference counter will be decreased after the handle is closed, allowing GC.

## iotjs_reqwrap_t

`iotjs_reqwrap_t` is for wrapping libuv request data and Javascript callback function. And make sure that the Javascript callback function is alive during the I/O operation.

Let's look at how asynchronous I/O are treated in IoT.js:

1. Javascript module calls builtin function to perform I/O applying arguments including callback.
2. Builtin creates `iotjs_reqwrap_t` to wrap `uv_req_s` and Javascript callback function.
3. Builtin calls libuv to perform the I/O.
4. After I/O finished, libuv calls builtin after handler.
5. Builtin after handler takes `iotjs_reqwrap_t` containing I/O result and Javascript callback function.
6. Builtin after handler calls Javascript callback.
7. Builtin after handler release `iotjs_reqwrap_t` by calling `iotjs_*reqwrap_dispatch()`

`iotjs_reqwrap_t` does not inherits `iotjs_handlewrap_t` for wrapping the callback function object.
Note that `HandleWrap` does not increase reference count of wrapping object. It does not guarantee guarantee liveness of the object even if the wrapper is alive.

On the other hand, `iotjs_reqwrap_t` increases the reference count for the callback function and decreases when it is being freed to guarantee the liveness of callback function during the request is ongoing.
After request is finished and `iotjs_reqwrap_t` released by calling `iotjs_*reqwrap_dispatch()`, the callback function could be collected by GC when it need to be.

# IoT.js Core

## Life cycle of IoT.js

_Note:_
_We are currently focusing on implementing IoT.js upon JerryScript engine._
_Implementation of initializing process depends on JerryScript API._
_That could be changed when we adopt other Javascript engines._
_Anyway, in this chapter we will explain initialization process based on current implementation._

The process of IoT.js can be summarized as follow:

1. Initialize JerryScript engine.
2. Execute empty script
 * Create initial Javascript context.
3. Initialize builtin modules.
 * Create builin modules including ['process'](../api/IoT.js-API-Process.md).
4. Evaluate ['iotjs.js'](../../src/js/iotjs.js).
 * Generate entry function.
5. Run the entry function passing 'process'.
 1. Initialize 'process' module.
 2. Load user application script.
 3. Run user application.
6. Run [event loop](#event-loop) until there are no more events to be handled.
7. Clean up.

## Builtin

"Builtin" is Javascript objects fully implemented in C using [embedding API](#embedding-api).
The list of builtin objects can be found at `MAP_MODULE_LIST` macro in ['iotjs_module.h'](../../src/iotjs_module.h).

Because methods of builtin modules are implemented as [native handler](#native-handler),
are able to access underlying system using libuv, C library, and system call.
Also, builtin modules could be used for optimizing performance of CPU bound routine or reduce binary size.

Builtin modules are initialized during [intializing step of IoT.js](#life-cycle-of-iotjs) and released just before program terminates.

## Native module

The [basic modules and extended modules](../api/IoT.js-API-reference.md) provided by IoT.js are called 'native module' because it will be included IoT.js as binary format.(not as script).
There is a [tool](../../tools/js2c.py) that transfer Javascript script source file into C file.

Usually a native module needs help from couple of [builtin](#builtin) modules which are implemented in C thus able to access underlying system.

Some native modules are bound to global object while others are on demand.
On demand modules will be created at the moment when it is first required and will not released until the program terminates.

## Event loop

_Note:_
_It would be helpful to read [libuv design overview](http://docs.libuv.org/en/v1.x/design.html) to understand asynchronous I/O programming model if you are not familiar with it._

_Note:_
_In this section we will see simple file open example and relevant code segment. You can find the source files at ['iotjs.c'](../../src/iotjs.c), [`iotjs_module_fs.c`](../../src/module/iotjs_module_fs.c) and ['fs.js'](../../src/js/fs.js)_


IoT.js follows asynchronous I/O programming model proposed by libuv to perform non-blocking, single-threaded, asynchronous I/O.

You can find main loop of the program at the file ['iotjs.c'](../../src/iotjs.c) in the source tree. It looks like this:

```c
  // Run event loop.
  bool more;
  do {
    more = uv_run(iotjs_environment_loop(env), UV_RUN_ONCE);
    more |= iotjs_process_next_tick();
    if (more == false) {
      more = uv_loop_alive(iotjs_environment_loop(env));
    }
  } while (more);
```

While running a IoT.js application, it could make requests for I/O operations using [IoT.js API](../api/IoT.js-API-reference.md).
For example, You can write a code for opening 'hello.txt' file and printing file descriptor out like this:
```js
fs.open('hello.txt', 'r', function(err, fd) {
  console.log('fd:' + fd);
});
conosle.log('requested');
```

To handle the request, IoT.js will wrapping the request and callback function using `iotjs_reqwrap_t`.
```c
  iotjs_fsreqwrap_t* req_wrap = iotjs_fsreqwrap_create(jcallback);
```

libuv will take charge of actual I/O processing taking the request.
```c
  uv_fs_t* fs_req = iotjs_fsreqwrap_req(req_wrap);
  int err = uv_fs_open(iotjs_environment_loop(env),
                       fs_req,
                       path, flags, mode,
                       AfterAsync);
```

Since all I/O are treated as non-blocking, calling for async I/O API returns immediately right after request was sent to libuv.
And then next line of javascript program will be executed immediately without waiting the I/O.
Thus in the above example 'requested' will be printed out right after file open request was made.

If there were I/O requests, `uv_run()` in the main loop waits by polling the requests until at least one of the request processing were finished.
When a result for a request was produced, internal part of libuv calls corresponding handler function (let it be after function) back.

Usually, the after function retrieves I/O result and `iotjs_reqwrap_t` object from request data.
And calling the javascript callback function with the result.

```c
  iotjs_fsreqwrap_t* req_wrap = (iotjs_fsreqwrap_t*)(req->data); // get request wrapper
  const iotjs_jval_t* cb = iotjs_fsreqwrap_jcallback(req_wrap); // javascript callback function

  iotjs_jargs_t jarg = iotjs_jargs_create(2);
  iotjs_jargs_append_null(&jarg); // in case of success.
  iotjs_jargs_append_number(req->result); // result - file descriptor for open syscall

  // callback
  iotjs_jhelper_make_callback(cb, iotjs_jval_get_null(), &jarg);

  // cleanup
  iotjs_jargs_destroy(&jarg);
  iotjs_fsreqwrap_dispatched(req_wrap);
```

For above file open example, calling javascript callback function would result execution of the handler.
```js
function(err, fd) {
  console.log('fd:' + fd);
}
```

One iteration of event loop is finished and `uv_run()` finally returns after all results were handled.
Next, it calls next tick handler.
```c
    more |= iotjs_process_next_tick();
```
And for next step, main event loop checks if there were no more request to be treated.
```c
    if (more == false) {
      more = uv_loop_alive(iotjs_environment_loop(env));
    }
```
If there are another iteration of the loop executed. Otherwise, main event loop ends.
