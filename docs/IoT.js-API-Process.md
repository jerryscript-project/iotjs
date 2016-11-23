## process

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

### Events


#### `'exit'`
* `callback: Function(code)`
 * `code: Number - exitCode`

Emitted when IoT.js process is about to exit.


#### `'uncaughtException'`
* `callback: Function(err)`
 * `err: Error - error object uncaught by catch handler`

Emitted there's no catch handler for exception.


### Methods


#### process.chdir(path)
* `path: String`

Changes current working directory to `path`.


#### process.cwd()

Returns current working directory.


#### process.exit(code)
* `code: Number - exitCode`

Exits executing process with code.


#### process.nextTick(callback)
* `callback: Function()`

After current event loop finished, calls the `callback` function. The next tick callback will be called before any I/O events.


### Member Variable


#### `'arch'`

Target Arch, one of (`arm`|`ia32`|`x64`|`unknown`)


#### `'argv'`

Argument vector to be passed to IoT.js


#### `'board'`

Target Board, one of (`STM32F4DIS`|`RP2`|`unknown`)


#### `'env'`

Environment object holding `HOME` and `NODE_PATH`. On NuttX `NODE_PATH` is `/mnt/sdcard` in default.


#### `'platform'`

Target Platform, one of (`linux`|`nuttx`|`darwin`|`unknown`)
