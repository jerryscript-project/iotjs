# Enabling Experimental Features

This document provides a guide on how to write and build experimental features.

## What's experimental build?

Experimental build is an executable IoT.js including features that are not yet ready for wide use, so they are protected by an experimental status. Developers can opt in to enabling these features when building IoT.js, but they should be used with caution. Because the changes in experimental build can include not only a brand new module but also the existing modules stably used. So developers and users may face unexpected side effects. You should be aware that all the features handled in experimental build may change, be broken, or be removed in the future.

## How to make IoT.js experimental build

You need to make IoT.js using our build script, ["build.py"](https://github.com/Samsung/iotjs/blob/master/tools/build.py), with `--experimental` or `-e` option.

 ```bash
 tools/build.py --experimental

 tools/build.py -e --iotjs-include-module experimental-module

 tools/build.py -e --config=build.experimental.config
 ```

 For selecting modules to be included, you need to notify the script where your modules exist. You can use `--iotjs-include-module` or `--config` option for that. For further information, please refer to [Writing Builtin JavaScript Module](https://github.com/Samsung/iotjs/blob/master/docs/devs/Writing-New-Builtin-Module.md#writing-builtin-javascript-module).

## Writing Code

### Identifier for C Code

Once you make IoT.js with `--experimental` option, a symbolic constant named `EXPERIMENTAL` is predefined in compile stage. You can use the identifier to seperate your experimental code from others as follows.

```c
#ifdef EXPERIMENTAL
  // experimental
#else
  // normal
#endif

#ifndef EXPERIMENTAL
  // normal
#else
  // experimental
#endif
```

### Identifier for JavaScript Code

In the case of javascript code, you can refer to `process.env.IOTJS_ENV` to check if running IoT.js is built with experimental features.

```javascript
if (process.env.IOTJS_ENV === 'experimental') {
  // experimental
} else {
  // normal
}
```

## Documentation

When documenting a guide or an API reference about your experimental module, it's required to explicitly indicate that the features are experimental. Please put the same caution below in every single document.

> :exclamation: This document describes an experimental feature and considerations. Please be aware that every experimental feature may change, be broken, or be removed in the future without any notice.
