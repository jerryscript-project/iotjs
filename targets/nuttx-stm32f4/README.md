### About

This directory contains files to run IoT.js on
[STM32F4-Discovery board](http://www.st.com/content/st_com/en/products/evaluation-tools/product-evaluation-tools/mcu-eval-tools/stm32-mcu-eval-tools/stm32-mcu-discovery-kits/stm32f4discovery.html) with [NuttX](http://nuttx.org/)

### How to build

#### 1. Set up the build environment for STM32F4-Discovery board

Clone IoT.js and NuttX into iotjs-nuttx directory

```bash
$ mkdir iotjs-nuttx
$ cd iotjs-nuttx
$ git clone https://github.com/Samsung/iotjs.git
$ git clone https://bitbucket.org/nuttx/nuttx.git
$ git clone https://bitbucket.org/nuttx/apps.git
$ git clone https://github.com/texane/stlink.git
```

The following directory structure is created after these commands

```bash
iotjs-nuttx
  + apps
  + iotjs
  |  + targets
  |      + nuttx-stm32f4
  + nuttx
  + stlink
```

#### 2. Add IoT.js as a builtin application for NuttX

```bash
$ cd apps/system
$ mkdir iotjs
$ cp ../../iotjs/targets/nuttx-stm32f4/* ./iotjs/
```

#### 3. Configure NuttX

```bash
# assuming you are in iotjs-nuttx folder
$ cd nuttx/tools

# configure NuttX USB console shell
$ ./configure.sh stm32f4discovery/usbnsh

$ cd ..
# might require to run "make menuconfig" twice
$ make menuconfig
```

We must set the following options:

* Change `Build Setup -> Build Host Platform` from _Windows_ to [_Linux_|_OSX_]
* Enable `System Type -> FPU support`
* Enable `System Type -> STM32 Peripheral Support -> SDIO`
* Enable `RTOS Features -> Clocks and Timers -> Support CLOCK_MONOTONIC`
* Enable `RTOS Features -> Pthread Options -> Enable mutex types`
* Enable `RTOS Features -> Files and I/O -> Enable /dev/console`
* Enable `RTOS Features -> Work queue support -> High priority (kernel) worker thread`
* Disable `Device Drivers -> Disable driver poll interfaces`
* Enable `Device Drivers -> MMC/SD Driver Suport`
* Enable `Device Drivers -> MMC/SD Driver Suport -> MMC/SD SDIO transfer support`
* Enable `Networking Support -> Networking Support`
* Enable `Networking Support -> Socket Support -> Socket options`
* Enable `Networking Support -> Unix Domain Socket Support`
* Enable `Networking Support -> TCP/IP Networking`
* Enable `Networking Support -> TCP/IP Networking -> Enable TCP/IP write buffering`
* Enable `File Systems -> FAT file system`
* Enable `File Systems -> FAT file system -> FAT upper/lower names`
* Enable `File Systems -> FAT file system -> FAT long file names`
* Enable `Device Drivers -> Network Device/PHT Support -> Late driver initialization`
* Enable `Library Routines -> Standard Math library`
* Enable `Application Configuration -> System Libraries and NSH Add-ons -> IoT.js`
* Enable all childs of `Application Configuration -> System Libraries and NSH Add-ons -> readline() Support` (for those who wants to use readline)


#### 4. Build IoT.js for NuttX

##### Follow the instruction
* [Build-for-NuttX](../../docs/Build-for-NuttX.md)

#### 5. Build NuttX

```bash
# assuming you are in iotjs-nuttx folder
$ cd nuttx/
$ make
```
For release version, you can type R=1 make on the command shell.

#### 6. Flashing

Connect Mini-USB for power supply and connect Micro-USB for `NSH` console.

To configure `stlink` utility for flashing, follow the instructions [here](https://github.com/texane/stlink#build-from-sources).

To flash,
```bash
# assuming you are in nuttx folder
$ sudo ../stlink/build/st-flash write nuttx.bin 0x8000000
```
