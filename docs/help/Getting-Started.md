### Overview
IoT.js is built based on **JerryScript** (lightweight JavaScript engine) and **libtuv** for asynchronous I/O event handling.

#### Source repositories
* IoT.js: https://github.com/Samsung/iotjs.git
* JerryScript: https://github.com/jerryscript-project/jerryscript.git
* libtuv: https://github.com/Samsung/libtuv.git

### Build script
There is a script to help you build IoT.js called "[build.py](../../tools/build.py)" in source repository.

### Supported platforms
Current supported platforms are **Linux and NuttX**

* [Build for Linux](../build/Build-for-Linux.md): Ubuntu 14.04 is used as a base platform.
* [Build for NuttX](../build/Build-for-NuttX.md)
* [Build for Raspberry Pi 2](../build/Build-for-RPi2.md)

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

We will support the correct behavior of APIs for above environments. However, since IoT.js is targeting various kind IoT devices and platforms, single implementation cannot be the best practice for every environments. Therefore embedders should be in charge of optimization for their own environments. For more details on optimization, see the [Optimization Tips](../devs/Optimization-Tips.md) page.

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
output-coverage=yes|no (default is no)
experimental=yes|no (default is no)
```

To give options, please use two dashes '--' **once** before the option name as described in the following sections.

Options that may need explanations.
* start-from: a test case file name where the driver starts.
* quiet: a flag that indicates if the driver suppresses console outputs of test case.
* output-file: a file name where the driver leaves output.
* skip-module: a module list to skip test of specific modules.
* output-coverage: a flag that indicates wether coverage data should be written to disk
* experimental: a flag that indicates if tests for experimental are needed

##### Options example

```bash
build/x86_64-linux/debug/bin/iotjs tools/check_test.js -- start-from=test_console.js quiet=no
```

##### To write a test case

Depend on the purpose of the test case (whether it's a positive or negative one), place it under `test/run_pass` or `test/run_fail` directory. The required external resources should be placed into `test/resources`.

All test case files must be named in the following form `test_<module name>[_<functionallity].js` where `<module name>`
should match one of the JS modules name in the IoT.js. If there is a test case which can not tied to a
module (like some js features) then the `iotjs` name can be used as module name. It is important to
correctly specify the module name as the test executor relies on that information.

1. Write a test case and place it into the proper directory.
2. List up the test case in [test/testsets.json](../../test/testsets.json), and set attributes (timeout, skip, ...) on the test case if it needs.

#### Advanced Topics
You can refer to [Writing new IoT.js builtin module](../devs/Writing-New-Builtin-Module.md) and [Optimization Tips](../devs/Optimization-Tips.md) pages for detailed information.

### When something goes wrong
Please read the [Logging IoT.js execution](../devs/Logging-IoT.js-execution.md) page how to display and add log messages while developing.

### [IoT.js API Reference](../api/IoT.js-API-reference.md)
