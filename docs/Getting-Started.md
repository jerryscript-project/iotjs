### Overview
IoT.js is built based on JerryScript(lightweight JavaScript engine) and libuv for asynchronous I/O event handling.

#### Source repositories
* IoT.js: https://github.com/Samsung/iotjs.git
* JerryScript: https://github.com/Samsung/jerryscript.git
* libuv: https://github.com/Samsung/libuv.git

### Build script
There is a script to help you build IoT.js called "[build.py](../tools/build.py)" in source repository.

### Supported platforms
Current supported platforms are **Linux and NuttX**

* [Build for Linux](Build-for-Linux.md): Ubuntu 14.04 is used as a base platform.
* [Build for NuttX](Build-for-NuttX.md)
* [Build for Raspberry Pi 2](Build-for-RPi2.md)

##### Platforms to support
* OSX 10.10 as development host
* [Artik 1 =>](https://www.artik.io/hardware/artik-1) as target board

##### H/W boards
* Current supporting
    * STM32F4-Discovery + BB
    * Raspberry Pi 2
* Plan to support
    * Samsung Artik 1
    * STM32F429-Discovery
    * STM32F411-Nucleo
    * Intel Edison
    * (and your contributions including above plans)

We will support the correct behavior of APIs for above environments. However, since IoT.js is targeting various kind IoT devices and platforms, single implementation cannot be the best practice for every environments. Therefore embedders should be in charge of optimization for their own environments. For more details on optimization, see the [Optimization Tips](Optimization-Tips.md) page.

### For Developers

#### How to Test

When you build ``iotjs`` binary successfully, you can run test driver with this binary.

```bash
/path/to/iotjs tools/check_test.js
```

##### Set test options

Some basic options are provided.

Existing test options are listed as follows;
```
start-from
quiet=yes|no (default is yes)
output-file
skip-module
```

To give options, please use two dashes '--' **once** before the option name as described in the following sections.

Options that may need explanations.
* start-from: a test case file name where the driver starts.
* quiet: a flag that indicates if the driver suppresses console outputs of test case.
* output-file: a file name where the driver leaves output.
* skip-module: a module list to skip test of specific modules.

##### Options example

```bash
build/x86_64-linux/debug/bin/iotjs tools/check_test.js -- start-from=test_console.js quiet=no
```

##### To write a test case

1. Write a test case inside test directory.
2. List up a test case in **test/testsets.js**. (this procedure will be removed after ``fs.readDir[Sync]`` is implemented.)
3. Set attributes on the test case if it needs in **attrs.js** where the directory of your test case belongs.

#### Advanced Topics
You can refer to [Writing new IoT.js builtin module](Writing-New-Builtin-Module.md) and [Optimization Tips](Optimization-Tips.md) pages for detailed information.

### When something goes wrong
Please read the [Logging IoT.js execution](Logging-IoT.js-execution.md) page how to display and add log messages while developing.

### [IoT.js API Reference](IoT.js-API-reference.md)
