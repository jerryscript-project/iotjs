### Overall steps to build for Linux
1. Get the sources
2. Build all at once
3. Execute IoT.js
4. Clean build directory

***

#### Build Host
Ubuntu 14.04 is recommended. Other Unix like platforms can be used. If it doesn't seem to work properly on other platforms, please look into the [Issues](https://github.com/Samsung/iotjs/issues) page. Someone may have already tried. If you can't find any related one, please leave an issue for help.

#### Directory structure

This document assumes 'harmony' as the root directory. _JerryScript_, _libtuv_ and _libuv_ are included as sub-modules in `deps` directory.

* harmony
    * iotjs
        * deps
            * http-parser
            * jerry
            * libuv
            * libtuv


※ harmony? It's from the initial code name of our project. (_Sounds good, isn't it? :)_)

#### Prerequisite

You need to install some packages to build IoT.js, as follows;

```
sudo apt-get install gyp cmake build-essential valgrind libbluetooth-dev
```

gcc/g++ compiler 4.8 or higher versions are required to compile. If you don't know how to do it, you can get some help from [how-to-install-gcc-4-8](http://askubuntu.com/questions/271388/how-to-install-gcc-4-8) or google.

### 1. Get the sources

Clone our repository to look around and test it. If it attracts you and you want to try something interested, please fork it.

To get the source for this repository,
```
cd harmony
git clone https://github.com/Samsung/iotjs.git
cd iotjs
```

Sub-modules(_http-parser_, _JerryScript_, _libuv_ and _libtuv_) will be pulled. And matching hash will be checked out for your current IoT.js version when you run the build script.


### 2. Build all at once

IoT.js and required sub-modules are generated all at once in tools directory with build.py.

```
cd iotjs
./tools/build.py
```


#### Set build options
Some basic options are provided.

Existing build options are listed as follows;
```
buildtype=debug|release (debug is default)
builddir=build (build is default)
clean
buildlib (default is False)
target-arch=x86|x86_64|x64|i686|arm (depends on your host platform)
target-os=linux|nuttx|darwin|osx (linux is default)
target-board
cmake-param
compile-flag
link_flag
external-include-dir
external-static-lib
external-shared-lib
iotjs-include-module
iotjs-exclude-module
jerry-cmake-param
jerry-compile-flag
jerry-link-flag
jerry-lto
jerry-heap-section
jerry-heaplimit (default is 81, may change)
jerry-memstat (default is False)
no-init-submodule (default is init)
no-check-tidy (default is check)
no-check-test (default is check)
no-parallel-build
no-snapshot
nuttx-home= (no default value)
```

To give options, please use two dashes '--' before the option name as described in the following sections.

Options that may need explanations.
* builddir: compile intermediate and output files are generated here.
* buildlib: generating _iotjs_ to a library if True(e.g. for NuttX). give __--buildlib__ to make it True.
* jerry-heaplimit: JerryScript default heap size (as of today) is 256Kbytes. This option is to change the size for embedded systems, nuttx for now, and current default is 81KB. For linux, this has no effect. While building nuttx if you see an error `region sram overflowed by xxxx bytes`, you may have to decrease about that amount.
* jerry-memstat: turn on the flag so that jerry dumps byte codes and literals and memory usage while parsing and execution.
* no-check-tidy: no checks codes are tidy. we recommend to check tidy.
* no-check-test: do not run all tests in test folder after build.
* nuttx-home: it's NuttX platform specific, to tell where the NuttX configuration and header files are.

If you want to know more details about options, please check the [Build Script](https://github.com/Samsung/iotjs/wiki/Build%20Script) page.


#### Include extended module
There are two ways to include [extended module](IoT.js-API-reference.md).

The first way is to modify a property value of module in `build.config` file. You can move a module name from 'exclude' to 'include'.

The second way is by using build options which is `--iotjs-include-module`.
If you enter several modules, separate them with a comma.

```
./tools/build.py --iotjs-include-module=dgram,pin,gpio
```


#### Options example

It's a good practice to build in separate directory, like 'build'. IoT.js generates all outputs into separate **'build'** directory. You can change this by --builddir option. Usually you won't need to use this option. Target and architecture name are used as a name for a directory inside 'build' directory.

To build debug version, type the command like below. And you can find the binary in 'output' directory.
```
./tools/build.py --builddir=output
```

To build 32bit version in x86_64 with debug version as a library, type the command like below.
```
./tools/build.py --target-arch=i686 --buildlib
```

To build release version with different jerry revision, type the command like below. (Assume that you have already checked it out.)
```
./tools/build.py --buildtype=release --no-init-submodule
```



#### Build only IoT.js with given build option

This section for explaining how to build only IoT.js when you did some modification. IoT.js uses [CMake](http://www.cmake.org/) for makefile generation. You can go inside the build folder and build with 'make' command. Go inside where your target platform name is, for example x86_64 linux,
```
cd build/x86_64-linux/release/iotjs
make
```

#### What build script does

1. It will clone sub-modules, this will be done only once when version hash has not changed.
2. Checkout matching version for each sub-modules.
3. Build sub-modules, you can see the outputs at build/(target-arch)-(target-os)/(buildtype)/libs folder.
4. Build IoT.js


### 3. Execute IoT.js

Executable name is **'iotjs'** and resides in (target-arch)-(target-os)/(buildtype)/iotjs.
To run greetings JavaScript in test folder, for example;

```
./build/x86_64-linux/debug/iotjs/iotjs ./test/run_pass/test_console.js
```

#### Set execution Options

Some execution options are provided as follows;
```
memstat
show-opcodes
```

To give options, please use two dashes '--' before the option name as described in following sections.

For more details on options, please see below.
* memstat: dump memory statistics. To get this, must build with __jerry-memstat__ option.
* show-opcodes: print compiled byte-code.


#### Options example

To print memory statistics, follow the below steps;
```
./tools/build.py --jerry-memstat

./build/x86_64-linux/debug/iotjs/iotjs ./test/run_pass/test_console.js --memstat
```

With given `show-opcodes` option, opcodes will be shown.
```
./build/x86_64-linux/debug/iotjs/iotjs ./test/run_pass/test_console.js --show-opcodes
```

### 4. Clean build directory

Just remove the folder as follows;
```
rm -rf build
rm -rf deps/libuv/out
```
