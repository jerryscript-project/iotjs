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
* Raspberry Pi 2
    * [Setting Raspberry Pi 2](Setting-Raspberry-Pi-2.md)
    * [Build for Raspberry Pi 2](Build-for-RPi2.md)
* [Build for Intel Edison with mraa](Build-for-Edison-with-mraa.md)

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

### Advanced Topics
You can refer to [Writing new IoT.js builtin module](Writing-New-Builtin-Module.md) and [Optimization Tips](Optimization-Tips.md) pages for detailed information.

### When something goes wrong
Please read the [Logging IoT.js execution](Logging-IoT.js-execution.md) page how to display and add log messages while developing.

### [IoT.js API Reference](IoT.js-API-reference.md)
