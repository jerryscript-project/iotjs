### Platform Support

The following shows module APIs available for each platform.

|  | Linux<br/>(Ubuntu) | Tizen<br/>(Raspberry Pi) | Raspbian<br/>(Raspberry Pi) | NuttX<br/>(STM32F4-Discovery) | TizenRT<br/>(Artik053) |
| :---: | :---: | :---: | :---: | :---: | :---: |
| require | O | O | O | O | O |

# Module
The `require` function is always available there is no need to import `module` explicitly.

### require(id)
* `id` {string} Module name to be loaded.

Loads the module named `id`.

**Example**

```js
var assert = require('assert');

assert.equal(2, 2);
```

**Loading a module**

If a native module named `id` exists, load it and return.
(_Native module:_ which module came from the IoT.js itself)

`require` function searches for modules in the following order:

1. Current working directory.
2. `iotjs_modules` folder under current working directory.
3. `$HOME/iotjs_modules`
4. `$IOTJS_PATH/iotjs_modules`
5. `$IOTJS_EXTRA_MODULE_PATH`

For each directory in search paths above:

- If a file `id` exists, load it and return.
- If a file `id.js` exists, load it and retun.
- If a directory `id` exists, module system consider the directory as a package:
  - If `id/package.json` contains **main** property, load the file named **main** property.
  - If `id/package.json` exists, but neither the **main** property nor the file named **main** property exist, load `index.js`.
- Extra step for Linux/Tizen targets:
  - If a file with `id.node` exists, try to load it as an N-API native addon and return.

**Changing current working directory**

You can explicitly change current working directory by setting `IOTJS_WORKING_DIR_PATH` environment variable. It is not recommended that you set this variable, if possible.

**Adding extra paths for module loading**

In order to add more directories to look for modules, you can set `IOTJS_EXTRA_MODULE_PATH` as an environment variable of your system. For instance, `./node_modules` and `./my_modules` will be referred if they're declared as follows.

`IOTJS_EXTRA_MODULE_PATH=./node_modules:./my_modules`
