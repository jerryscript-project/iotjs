### Platform Support

The following shows module APIs available for each platform.

|  | Linux<br/>(Ubuntu) | Raspbian<br/>(Raspberry Pi) | NuttX<br/>(STM32F4-Discovery) | TizenRT<br/>(Artik053) |
| :---: | :---: | :---: | :---: | :---: |
| require | O | O | O | - |

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

For each directory in search paths above:

- If a file `id` exists, load it and return.
- If a file `id.js` exists, load it and retun.
- If a directory `id` exists, module system consider the directory as a package:
  - If `id/package.json` contains **main** property, load the file named **main** property.
  - If `id/package.json` exists, but neither the **main** property nor the file named **main** property exist, load `index.js`.
