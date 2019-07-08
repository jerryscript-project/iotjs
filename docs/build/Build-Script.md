## Overview

build.py help you build IoT.js.

It locates in "./tools" directory of source tree.

It automatically creates a directory where build object and outputs will be generated,
checks configurations, tidiness of source code, licenses, and more.
Also it downloads, updates and builds submodules.
And finally generate IoT.js binary.


## How to use

You can build IoT.js with default setting for your host machine with;
```
./tools/build.py
```
The command will generate runnable IoT.js binary in "./build/<arch-os>/debug/bin/iotjs".

You can also build release binary with;
```
./tools/build.py --buildtype=release
```

### Arguments of IoT.js
The following arguments are related to the IoT.js framework.

---
#### `--buildtype`
* `release` | `debug`

Specify whether build output will be for 'debug' or 'release'.

```
./tools/build.py --buildtype=release
```

---
#### `--builddir`

Specify a directory where build outputs will be generated.

If given path is not exist, build.py will create it.

```
./tools/build.py --builddir=./build
```

---
#### `--buildlib`
With given this option, build.py will generate IoT.js output as a library.

```
./tools/build.py ---buildlib
```

---
#### `--cmake-param`
Specify CMake parameters for IoT.js.

"cmake" command for IoT.js will be executed with the given parameter applied.

If you have multiple parameters, supply it with multiple use of this option;

```
./tools/build.py --cmake-param="..." --cmake-param="..."
```

---
#### `--compile-flag`
Specify C compiler flags for IoT.js.

If you have multiple compile flags, supply it with multiple use of this option;
```
./tools/build.py --compile-flag="..." --compile-flag="..."
```

---
#### `--clean`
With given this option, build.py will clear all the build directory before start new build.

```
./tools/build.py --clean
```

---
#### `--config`
Specify build configuration file path.

```
./tools/build.py --config=build.arm.nuttx.stm32f4dis.config
```

`build.default.config` file is in the source tree for default setting.

If this option is not specified, `build.config` file will be applied. If the file does not exist, it will be copied from `build.default.config`.

Parameters specified by the config file is applied, and then the parameters given by command line overwrite over the settings.

If you need to apply the same set of parameters for each build, making your own config file and trigger build.py with the config file would be more convenient.

---
#### `-e, --experimental`
Enable to build experimental features

```
./tools/build.py --experimental
```

---
#### `--external-include-dir`
Specify external include directory for IoT.js.

If you have multiple external include directoies, supply it with multiple use of this option;
```
./tools/build.py --external-include-dir="..." --external-include-dir="..."
```

---
#### `--external-lib`
Specify external library that will be linked with IoT.js.

If you have multiple such libraries, supply it with multiple use of this option;
```
./tools/build.py --external-lib="libxxx"
```

---
#### `--external-modules`
Specify the path of modules.json files which should be processed (format: path1,path2,...).
See also: ["How to write a new module"](../devs/Writing-New-Module.md)

```
./tools/build.py --external-modules=/home/iotjs/my-modules-directory
```

---
#### `--link-flag`
Specify linker flags for IoT.js.

If you have multiple link flags, supply it with multiple use of this option;
```
./tools/build.py --link-flag="..." --link-flag="..."
```

---
#### `--no-check-valgrind`
Disable test execution with valgrind after build.

```
./tools/build.py --no-check-valgrind
```

---
#### `--no-init-submodule`
With given this option, submoduls will not initialized before start build.

```
./tools/build.py --no-init-submodule
```

---
#### `--no-parallel-build`
With given this option, compilation process will not run in parallel. In other words, executes `make` without `-j` option.

```
./tools/build.py --no-parallel-build
```

---
#### `--nuttx-home`
To build for nuttx os, nuttx home directory must be given.

```
./tools/build.py --target-os=nuttx --target-arch=arm --target-board=stm32f4dis --nuttx-home="..."
```

---
#### `--profile`
With given this option, build.py will use the specified profile for the build.
See also: ["How to write a new module"](../devs/Writing-New-Module.md#profile)

```
./tools/build.py --profile=./profiles/minimal.profile
```

---
#### `--run-test`
* `full` | `quiet`

Execute tests after build, optional argument specifies the level of output for the testrunner.

```
./tools/build.py --run-test=full
```

---
#### `--sysroot`
The location of the development tree root directory (sysroot). Must be compatible with used toolchain.

```
./tools/build.py --sysroot=/home/iotjs/sysroot-directory
```

---
#### `--target-arch`
* `arm` | `x86` | `i686` | `x86_64` | `x64` | `mips` | `noarch`

Specify target architecture.

```
./tools/build.py --target-arch=arm
```

---
#### `--target-board`
* `artik10` | `artik05x` | `rpi2` | `rpi3` | `stm32f4dis` | empty

Specify target board.

```
./tools/build.py --target-os=nuttx --target-arch=arm --target-board=stm32f4dis
```

---
#### `--target-os`
* `linux` | `darwin` | `osx` | `nuttx` | `tizen` | `tizenrt` | `openwrt`

Specify target OS.

```
./tools/build.py --target-os=nuttx --target-arch=arm
```


### Arguments of JerryScript
The following arguments are related to the JavaScript engine under the framework. For example they can change the enabled features of the ECMA-262 standard.

---
#### `--jerry-cmake-param`
Specify CMake parameters for JerryScript.

"cmake" command for JerryScript will be executed with the given parameter applied.

If you have multiple parameters, supply it with multiple use of this option

---
#### `--jerry-compile-flag`
Specify C compiler flags for JerryScript.

If you have multiple cflags, supply it with multiple use of this option

```
./tools/build.py --jerry-compile-flag="-DCONFIG_ECMA_LCACHE_DISABLE"
```

---
#### `--jerry-debugger`
Enable JerryScript debugger. See also ["Use JerryScript Debugger"](../devs/Use-JerryScript-Debugger.md).

```
./tools/build.py --jerry-debugger
```

---
#### `--jerry-heaplimit`
Specify object heap limit for JerryScript engine.

```
./tools/build.py --jerry-heaplimit=80
```

---
#### `--jerry-heap-section`
Specify the name of the JerryScript heap section.

```
./tools/build.py --jerry-heap-section=".ARM.__at_0x20000"
```

---
#### `--jerry-lto`
With given this option, JerryScript will be built with LTO.

```
./tools/build.py --jerry-lto
```

---
#### `--jerry-memstat`
Enable memstat of JerryScript engine.

```
./tools/build.py --jerry-memstat
```

---
#### `--jerry-profile`
* `es5.1` | `es2015-subset | absolute path to a custom profile file`

Specify the profile for JerryScript (default: es5.1). In JerryScript all of the features are enabled by default, so an empty profile file turns on all of the available ECMA features. See also the related [README.md](https://github.com/jerryscript-project/jerryscript/blob/master/jerry-core/profiles/README.md).

E.g.:
**/home/iotjs/my-jerry-profile.profile**
```
# Turn off every ES2015 feature EXCEPT the arrow functions
CONFIG_DISABLE_ES2015_BUILTIN
CONFIG_DISABLE_ES2015_PROMISE_BUILTIN
CONFIG_DISABLE_ES2015_TEMPLATE_STRINGS
CONFIG_DISABLE_ES2015_TYPEDARRAY_BUILTIN
```

```
./tools/build.py --jerry-profile=/home/iotjs/my-jerry-profile.profile
```

---
#### `--js-backtrace`
Enable/disable backtrace information of JavaScript code (default: ON in debug and OFF in release build).

```
./tools/build.py --js-backtrace
```
