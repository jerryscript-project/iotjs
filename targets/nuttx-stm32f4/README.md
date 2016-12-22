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
$ cp ../../iotjs/targets/nuttx-stm32f4/app/* ./iotjs/
```

#### 3. Configure NuttX

```bash
# assuming you are in iotjs-nuttx folder
$ cd nuttx/tools

# configure NuttX USB console shell
$ ./configure.sh stm32f4discovery/usbnsh
```

Now you can configure nuttx like either of below. For convenience, we provide built-in configure file for you. (This configure file is equipped with modules specified as `always`. For `optional` modules, you might follow instructions below.)
```bash
$ cd ..
$ cp ../iotjs/targets/nuttx-stm32f4/nuttx/.config.default .config
```

Or if you want to configure yourself, you can follow guide below.
```bash
$ cd ..
# might require to run "make menuconfig" twice
$ make menuconfig
```

Followings are the options to set:

* Common
 * Change `Build Setup -> Build Host Platform` from _Windows_ to [_Linux_|_OSX_]
 * Enable `System Type -> FPU support`
 * Enable `System Type -> STM32 Peripheral Support -> SDIO`
 * Enable `RTOS Features -> Clocks and Timers -> Support CLOCK_MONOTONIC`
 * Enable `RTOS Features -> Pthread Options -> Enable mutex types`
 * Enable `RTOS Features -> Files and I/O -> Enable /dev/console`
 * Enable `RTOS Features -> Work queue support -> High priority (kernel) worker thread`
 * Disable `Device Drivers -> Disable driver poll interfaces`
 * Enable `Device Drivers -> MMC/SD Driver Support`
 * Enable `Device Drivers -> MMC/SD Driver Support -> MMC/SD SDIO transfer support`
 * Enable `Networking Support -> Networking Support`
 * Enable `Networking Support -> Socket Support -> Socket options`
 * Enable `Networking Support -> Unix Domain Socket Support`
 * Enable `Networking Support -> TCP/IP Networking`
 * Enable `Networking Support -> TCP/IP Networking -> Enable TCP/IP write buffering`
 * Enable `File Systems -> FAT file system`
 * Enable `File Systems -> FAT file system -> FAT upper/lower names`
 * Enable `File Systems -> FAT file system -> FAT long file names`
 * Enable `Device Drivers -> Network Device/PHY Support -> Late driver initialization`
 * Enable `Library Routines -> Standard Math library`
 * Enable `Application Configuration -> System Libraries and NSH Add-ons -> IoT.js`
 * Enable all children of `Application Configuration -> System Libraries and NSH Add-ons -> readline() Support` (for those who wants to use readline)

* For `net` module
 * Enable `System Type -> STM32 Peripheral Support -> Ethernet MAC`
 * Disable `System Type -> STM32 Peripheral Support -> USART2`
 * Enable `System Type -> STM32 Peripheral Support -> USART6`
 * Set `System Type -> Ethernet MAC configuration -> PHY address` to `0`
 * Set `System Type -> Ethernet MAC configuration -> PHY Status Register Address (decimal)` to `31`
 * Enable `System Type -> Ethernet MAC configuration -> PHY Status Alternate Bit Layout`
 * Set `System Type -> Ethernet MAC configuration -> PHY Mode Mask` to `0x001c`
 * Set `System Type -> Ethernet MAC configuration -> 10MBase-T Half Duplex Value` to `0x0004`
 * Set `System Type -> Ethernet MAC configuration -> 100Base-T Half Duplex Value` to `0x0008`
 * Set `System Type -> Ethernet MAC configuration -> 10Base-T Full Duplex Value` to `0x0014`
 * Set `System Type -> Ethernet MAC configuration -> 10MBase-T Full Duplex Value` to `0x0018`
 * Set `System Type -> Ethernet MAC configuration -> RMII clock configuration` to `External RMII clock`
 * Enable `Board Selection -> STM32F4DIS-BB base board`
 * Set `Device Drivers -> Network Device/PHY Support -> Board PHY Selection` to `SMSC LAN8720 PHY`
 * Enable `Networking Support -> Driver buffer configuration -> Use multiple device-side I/O buffers`
 * Enable `Networking Support -> Data link support -> Local loopback`
 * Enable `Networking Support -> TCP/IP Networking -> TCP/IP backlog support`
 * Enable `Networking Support -> ARP Configuration -> ARP send`

* For `dgram`
 * Enable `Networking Support > UDP Networking`

* For `pwm` module
 * Enable `System Type -> STM32 Peripheral Support -> TIM(N)`
 * Enable `System Type -> Timer Configuration -> TIM(N) PWM`
 * Set `System Type -> Timer Configuration -> TIM(N) PWM -> TIM(n) PWM Output Channel` to channel number you want
 * Enable `Device Drivers -> PWM Driver Support`

* For `adc` module
 * Enable `System Type -> STM32 Peripheral Support -> ADC(N)`
 * Enable `System Type -> STM32 Peripheral Support -> TIM(M)`
 * Enable `System Type -> Timer Configuration -> TIM(M) ADC`
 * Enable `Device Drivers -> Analog Device(ADC/DAC) Support`
 * Enable `Device Drivers -> Analog Device(ADC/DAC) Support -> Analog-to-Digital Conversion`

* For `uart` module
 * Enable `System Type -> STM32 Peripheral Support -> U[S]ART(N)`

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
