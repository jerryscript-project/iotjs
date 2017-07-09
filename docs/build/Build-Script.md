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

## Parameters Candidates
**NOTE: some parameters are not supported by current version of build.py**

--
#### `--buildtype`
* `release` | `debug`

Specify whether build output will be for 'debug' or 'release'.

```
./tools/build.py --buildtype=release
```

--
#### `--builddir`

Specify a directory where build outputs will be generated.

If given path is not exist, build.py will create it.

```
./tools/build.py --builddir=./build
```

--
#### `--clean`
With given this option, build.py will clear all the build directory before start new build.

```
./tools/build.py --clean
```

--
#### `--buildlib`
With given this option, build.py will generate IoT.js output as a library.

```
./tools/build.py ---buildlib
```

--
#### `--target-arch`
* `arm` | `x86` | `i686` | `x86_64` | `x64`

Specify target architecture.

```
./tools/build.py --target-arch=arm
```
--
#### `--target-os`
* `linux` | `darwin` | `osx` | `nuttx`

Specify target OS.

```
./tools/build.py --target-os=nuttx --target-arch=arm
```

--
#### `--target-board`
* `stm32f4dis` | empty

Specify target board.

```
./tools/build.py --target-os=nuttx --target-arch=arm --target-board=stm32f4dis
```
--
#### `--cmake-param`
Specify CMake parameters for IoT.js.

"cmake" command for IoT.js will be executed with the given parameter applied.

If you have multiple parameters, supply it with multiple use of this option;

```
./tools/build.py --cmake-param="..." --cmake-param="..."
```

--
#### `--compile-flag`
Specify C compiler flags for IoT.js.

If you have multiple compile flags, supply it with multiple use of this option;
```
./tools/build.py --compile-flag="..." --compile-flag="..."
```

--
#### `--link-flag`
Specify linker flags for IoT.js.

If you have multiple link flags, supply it with multiple use of this option;
```
./tools/build.py --link-flag="..." --link-flag="..."
```

--
#### `--external-include-dir`
Specify external include directory for IoT.js.

If you have multiple external include directoies, supply it with multiple use of this option;
```
./tools/build.py --external-include-dir="..." --external-include-dir="..."
```

--
#### `--external-static-lib`
Specify external static library that will be liked with IoT.js statically.

If you have multiple such libraries, supply it with multiple use of this option;
```
./tools/build.py --external-static-lib="libxxx.a"
```

--
#### `--jerry-cmake-param`
Specify CMake parameters for JerryScript.

"cmake" command for JerryScript will be executed with the given parameter applied.

If you have multiple parameters, supply it with multiple use of this option

--
#### `--jerry-compile-flag`
Specify C compiler flags for JerryScript.

If you have multiple cflags, supply it with multiple use of this option

```
./tools/build.py --jerry-compile-flag="-DCONFIG_ECMA_LCACHE_DISABLE"
```

--
#### `--jerry-link-flag`
Specify linker flags for JerryScript.

If you have multiple ldflags, supply it with multiple use of this option

--
#### `--jerry-heaplimit`
Specify object heap limit for JerryScript engine.

```
./tools/build.py --jerry-heaplimit=80
```

--
#### `--jerry-memstat`
Enable memstat of JerryScript engine.

```
./tools/build.py --jerry-memstat
```

--
#### `--jerry-lto`
With given this option, JerryScript will be built with LTO.

```
./tools/build.py --jerry-lto
```

--
#### `--no-init-submodule`
With given this option, submoduls will not initialized before start build.

```
./tools/build.py --no-init-submodule
```

--
#### `--no-check-tidy`
With given this option, tidy checking will not performed.

```
./tools/build.py --no-check-tidy
```

--
#### `--no-check-test`
With given this option, unit test checking will not performed.

```
./tools/build.py --no-check-test
```

--
#### `--no-parallel-build`
With given this option, compilation process will not run in parallel. In other words, executes `make` without `-j` option.

```
./tools/build.py --no-parallel-build
```

--
#### `--nuttx-home`
To build for nuttx os, nuttx home directory must be given.

```
./tools/build.py --target-os=nuttx --target-arch=arm --target-board=stm32f4dis --nuttx-home="..."
```

--
#### `--config`
Specify build configuration file path.

```
./tools/build.py --config=build.arm.nuttx.stm32f4dis.config
```

`build.default.config` file is in the source tree for default setting.

If this option is not specified, `build.config` file will be applied. If the file is not exist, it will be copied from `build.default.config`.

Parameters specified by the config file is applied, and then the parameters given by command line overwrite over the settings.

If you need to apply the same set of parameters for each build, making your own config file and trigger build.py with the config file would be more convenient.