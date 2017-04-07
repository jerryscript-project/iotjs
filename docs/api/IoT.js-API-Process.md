### Platform Support

The following shows process module APIs available for each platform.

|  | Linux<br/>(Ubuntu) | Raspbian<br/>(Raspberry Pi) | Nuttx<br/>(STM32F4-Discovery) |
| :---: | :---: | :---: | :---: |
| process.nextTick | O | O | O |
| process.exit | O | O | O |
| process.cwd | O | O | O |
| process.chdir | O | O | O |

※ On nuttx, you should pass absolute path to `process.chdir`.

`process` is unique global object can be accessed from anywhere.

### Contents

- [Process](#process)
    - [Properites](#properites)
        - [`'arch'`](#arch)
        - [`'argv'`](#argv)
        - [`'board'`](#board)
        - [`'env'`](#env)
        - [`'platform'`](#platform)
    - [Module Functions](#module-functions)
        - [`process.chdir(path)`](#processchdirpath)
        - [`process.cwd()`](#processcwd)
        - [`process.exit(code)`](#processexitcode)
        - [`process.nextTick(callback)`](#processnexttickcallback)
    - [Events](#events)
        - [`'exit'`](#exit)
        - [`'uncaughtException'`](#uncaughtexception)

# Process

The process object is a global that provides information about, and control over, the current IoT.js process. As a global, it is always available to IoT.js applications without using require().

## Properites

### `'arch'`

Target Arch, one of (`arm`|`ia32`|`x64`|`unknown`)

### `'argv'`

Argument vector to be passed to IoT.js


### `'board'`

Target Board, one of (`STM32F4DIS`|`RP2`|`unknown`)

### `'env'`

Environment object holding `HOME` and `NODE_PATH`. On NuttX `NODE_PATH` is `/mnt/sdcard` in default.

### `'platform'`

Target Platform, one of (`linux`|`nuttx`|`darwin`|`unknown`)

## Module Functions

### `process.chdir(path)`
* `path <String>`

Changes current working directory to `path`.

### `process.cwd()`

Returns current working directory.

### `process.exit(code)`
* `code <Number>` - exitCode

Exits executing process with code.

### `process.nextTick(callback)`
* `callback <Function()>`

After current event loop finished, calls the `callback` function. The next tick callback will be called before any I/O events.

## Events

### `'exit'`
* `callback <Function(code)>`
* `code <Number>` - exitCode

Emitted when IoT.js process is about to exit.

### `'uncaughtException'`
* `callback <Function(err)>`
* `err <Error>` - error object uncaught by catch handler

Emitted there's no catch handler for exception.
