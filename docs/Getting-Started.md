### Supported platforms
Current supported platforms are **Linux and NuttX**

OSX 10.10 as development host

* [Build for x86 / Linux](build/Build-for-x86-Linux.md): Ubuntu 14.04 is used as a base platform.
* [Build for Stm32f4 / NuttX](build/Build-for-STM32F4-NuttX.md)
* [Build for Raspberry Pi 2 / Linux](build/Build-for-RPi2-Linux.md)

#### H/W boards
* Current supporting
    * STM32F4-Discovery + BB
    * Raspberry Pi 2
* Plan to support
    * Samsung Artik 053
    * Samsung Artik 10
    * (and your contributions including above plans)

We will support the correct behavior of APIs for above environments. However, since IoT.js is targeting various kind IoT devices and platforms, single implementation cannot be the best practice for every environments. Therefore embedders should be in charge of optimization for their own environments. For more details on optimization, see the [Optimization Tips](devs/Optimization-Tips.md) page.


### Build script
There is a script to help you build IoT.js called "[build.py](https://github.com/Samsung/iotjs/blob/master/tools/build.py)" in source repository.

