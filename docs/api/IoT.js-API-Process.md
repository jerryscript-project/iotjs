### Platform Support

The following shows process module APIs available for each platform.

|  | Linux<br/>(Ubuntu) | Raspbian<br/>(Raspberry Pi) | NuttX<br/>(STM32F4-Discovery) | TizenRT<br/>(Artik053) |
| :---: | :---: | :---: | :---: | :---: |
| process.nextTick | O | O | O | - |
| process.exit | O | O | O | - |
| process.cwd | O | O | O | - |
| process.chdir | O | O | O | - |

※ On NuttX, you should pass absolute path to `process.chdir`.

# Process

The `process` object is a global that provides information about, and control over, the current IoT.js process.
As a global, it is always available to IoT.js applications without using `require()`.

### process.arch
* {string}

The `arch` property returns the processor architecture identifier that the IoT.js process is currently running on.
For instance `'arm'`, `'ia32'`, `'x64'`, or `'unknown'`.

### process.argv
* {Array}

The `argv` property returns an array containing the command line arguments passed when the IoT.js
process was launched. The first element is the path to the IoT.js program.
The second element is the path of the JavaScript file currently being executed.
The remaining elements will be any additional arguments passed via command line.

**Example**

To print out all command line arguments the following example could be used:

```js
process.argv.forEach(function(val, idx) {
  console.log('index: ' + idx + ' value: ' + val);
});
```

### process.env
* {Object}

The `env` property returns an object containing a few environment variables.
The following environment elements can be accessed:
* `HOME`
* `IOTJS_PATH` which is set to `/mnt/sdcard` on NuttX by default.
* `env` contains `'experimental'` if the IoT.js was build with experimental support.

**Example**

```js
console.log('HOME: ' + process.env.HOME);
// prints: HOME: /home/user
```

### process.exitCode
* {integer} **Default:** `0`

The `exitCode` property can be used to specify the exit code of the IoT.js process.
This will be used when the process exits gracefully, or exited via `process.exit()` without specifying an exit code.

Specifying an exit code for the `process.exit()` call will override any previous setting of `process.exitCode`.

### process.iotjs
* {Object}

The `iotjs` property holds IoT.js related information in an object.
The following keys can be accessed via this property:
* `board` specifies the device type on which the IoT.js is running currently. For instance `'STM32F4DIS'`, `'RP2'`, or `'unknown'`.

**Example**

```js
console.log(process.iotjs.board);
// on Raspberry 2 it prints: RP2
```

### process.platform
* {string}

The `platform` returns the identification of the operating system the IoT.js process
is currently running on. For instance `'linux'`, `'darwin'`, `'nuttx'`, `'tizenrt'`, or `'unknown'`.


### process.chdir(path)
* `path` {string} The path to change working directory to.

The `chdir` method changes the current working directory of the IoT.js process or
throws an exception if the operation fails (for instance the `path` specified does not exist).

**Example**

```js
try {
  process.chdir('/invalid/path');
} catch(err) {
  console.log('invalid path');
}
// prints: invalid path
```

### process.cwd()
* Returns: {string}

The `cwd()` call returns the current working directory of the IoT.js process.

**Example**

```js
console.log('Current dir: ' + process.cwd());
```

### process.exit([code])
* `code` {integer} The exit code. **Default:** `0`

The `exit()` method instructs the IoT.js to terminate the process synchronously with an exit status of `code`.
If `code` is not specified, exit uses the `process.exitCode` value which defaults to `0`.
IoT.js will not exit till all `'exit'` event listeners are called.

The `process.exit()` method call will force the process to exit as quickly as possible,
ignoring if there is any asynchronous operations still pending.

In most situations, it is not necessary to explcitly call `process.exit()`. The IoT.js will exit on its own
if there is no additional work pending in the event loop. The `process.exitCode` property can be set
to exit code when the process exits gracefully.

If it is necessary to terminate the IoT.js process due to an error condition, throwing an
uncaught error and allowing the process to terminate accordingly is advised instead of calling `process.exit()`.

**Example**

To exit with a failure code:

```js
process.exit(1);
```
The shell that executed the IoT.js should see the returned exit code as `1`.

To set the exit code on graceful exit:

```js
doSomeWork()
process.exitCode = 1;
```

### process.nextTick(callback, [...args])
* `callback` {Function}
* `...args` {any} Additional arguments to pass when invoking the callback

The `nextTick` method adds the `callback` method to the "next tick queue".
Once the current turn of the event loop is completed, all callbacks currently in the next tick queue will be called.

This is not a simple alias to `setTimeout` call. It runs before any additional I/O events.

**Example**
```js
console.log('step 1');
process.nextTick(function() {
  console.log('step 2');
});
console.log('step 3');
// prints:
// step 1
// step 3
// step 2
```

### process.version
* {string}

The `version` property returns the version numbering of the currently running IoT.js process as a string.

**Example**
```js
console.log(process.version);
// prints: (in case of version 1.0.0)
// 1.0.0
```

### Event: 'exit'
* `callback` {Function}
  * `code` {integer}  exitCode

The `'exit'` event is emitted when the IoT.js process is about to exit.
This can happen two ways:
* The `process.exit()` method was called explicitly;
* The IoT.js event loop does not have any additional work to perform.

There is no way to prevent the exiting, and once all `'exit'` listeners have finished
running the process will terminate.

The listener callback function is invoked with the exit code specified either by the
`process.exitCode` property, or the `exitCode` argument passed to the `process.exit()` method.

Listener functions *must* only perform *synchronous* operations. The IoT.js process will exit
immediately after calling the `'exit'` event listeners causing any additional work still queued
to be abandoned.

**Example**

```js
process.on('exit', function(code) {
  console.log('exited with: ' + code);
});
```

### Event: 'uncaughtException'
* `callback` {Function}
  * `err` {Error} error object uncaught by catch handler

The `'uncaughtException'` event is emitted when an uncaught JavaScript exception bubbles all the
way back to the event loop. By default, IoT.js handles such exceptions by printing it to `stderr` and exiting.
Adding a handler for the `'uncaughtException'` event overrides the default behavior.

The listener function is called with the `Error` object passed as the only argument.

**Warning: Using the `'uncaughtException'` event correctly**

This event is a crude mechanism for exception handling intended to be used only as a last resort.
The event should not be used as an eqivalent to 'On Error Resume Next'. Unhandled exceptions inherently mean
that a program is in an unknown state. Thus attempting to resume the application code without properly
recovering from the exception can cause unforeseen/unpredictable issues.

Only one exception thrown within the event handler will be caught and will immediately terminate the application.

The correct use of the event is to perform synchronous cleanup of allocated resources (e.g. file descriptors, handles, etc)
before shutting down the process. **It is not safe to resume normal operation after this event**.

To restart a crashed application in a more reliable way, whether uncaughtException is emitted or not,
an external monitoring application should be employed in a separate process to detect application failures and
recover or restart as needed.

**Example**

```js
process.on('uncaughtException', function(err) {
  console.log('Something went wrong: ' + err);
});

nonExistentFunctionCall();
console.log('This will not be printed.');
```
