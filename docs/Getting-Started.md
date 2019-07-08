### Supported platforms
Current supported platforms are **Linux, [NuttX][nuttx-site], [Tizen][tizen-site] and [TizenRT][tizenrt-site]**

OSX 10.10 as development host

* [Build for x86 / Linux](build/Build-for-x86-Linux.md): Ubuntu 14.04 is used as a base platform.
* [Build for Raspberry Pi 2 / Linux](build/Build-for-RPi2-Linux.md)
* [Build for Raspberry Pi 3 / Tizen](build/Build-for-RPi3-Tizen.md)
* [Build for Stm32f4 / NuttX](build/Build-for-STM32F4-NuttX.md)
* [Build for ARTIK053 / TizenRT](build/Build-for-ARTIK053-TizenRT.md)
* [Build for ARTIK530 / Tizen](build/Build-for-RPi3-Tizen.md)
* [Build for OpenWrt (non-tested)](build/Build-for-OpenWrt.md)
* [Build for Windows (experimental)](build/Build-for-Windows.md)

#### H/W boards
* Current supporting
    * STM32F4-Discovery + BB
    * Raspberry Pi 2
    * Raspberry Pi 3
    * Samsung ARTIK 053
    * Samsung ARTIK 530

We will support the correct behavior of APIs for above environments. However, since IoT.js is targeting various kind IoT devices and platforms, single implementation cannot be the best practice for every environments. Therefore embedders should be in charge of optimization for their own environments. For more details on optimization, see the [Optimization Tips](devs/Optimization-Tips.md) page.


### Build script
There is a [script](build/Build-Script.md) to help you build IoT.js called "[build.py](https://github.com/jerryscript-project/iotjs/blob/master/tools/build.py)" in source repository. Run `tools/build.py --help` command to check all of the build options.

#### How to Build

```bash
  tools/build.py --clean
```

#### Frequently used build options

`--clean` Clean build directory before build (default: False).

`--no-snapshot` Disable snapshot generation for IoT.js. It is useful for debugging sessions.

`--profile PROFILE` Specify the module profile file for IoT.js. It is used for enable and disable modules. See also ["How to write a new module"](devs/Writing-New-Module.md#profile)

`--run-test [{full,quiet}]` Execute tests after build, optional argument specifies the level of output for the test runner.

`--jerry-debugger` Enable JerryScript debugger, so JavaScript could can be investigated with an available debugger client (eg.: [Python Debugger Console](https://github.com/jerryscript-project/jerryscript/blob/master/jerry-debugger/jerry_client.py) or [IoT.js Code](https://github.com/jerryscript-project/iotjscode/)). See also ["Use JerryScript Debugger"](devs/Use-JerryScript-Debugger.md).

`--js-backtrace {ON,OFF}` Enable/disable backtrace information of JavaScript code (default: ON in debug and OFF in release build).

[nuttx-site]: http://nuttx.org/
[tizen-site]: https://www.tizen.org/
[tizenrt-site]: https://wiki.tizen.org/Tizen_RT
