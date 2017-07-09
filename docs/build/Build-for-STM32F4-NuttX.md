
### Target board
We work on STM32F4 board for NuttX and the detail of the reference board is well described at [STM32F4-discovery with BB](http://www.st.com/web/en/catalog/tools/FM116/SC959/SS1532/LN1199/PF255417).

### Relation with STM board?
We do not have any business relation with STM board. It is selected because it has enough RAM and Flash ROM, so that development can be more comfortable. And it has lots of pins to play with.

When IoT.js is built up and optimized, it may work in devices having smaller resource.


### 1. Prepare for prerequisite

#### Linux

```bash
$ sudo apt-get install autoconf libtool gperf flex bison autoconf2.13
$ sudo apt-get install cmake libncurses-dev libusb-1.0-0-dev
$ sudo apt-get install libsgutils2-dev gcc-arm-none-eabi minicom
```

To use menuconfig in NuttX, you may need to install kconfig frontend.

```bash
$ git clone https://github.com/jameswalmsley/kconfig-frontends.git
$ cd kconfig-frontends
$ ./bootstrap
$ ./configure --enable-mconf
$ make
$ sudo make install
$ sudo ldconfig
```

#### macOS

* Install Xcode from the app store and run once Xcode to install components.
* Install Xcode command line tools.
```bash
$ xcode-select --install
```
* Install [Homebrew package manager](http://brew.sh/)
* Install packages
```bash
$ brew tap PX4/homebrew-px4
$ brew update
$ brew install cmake bash-completion pkg-config kconfig-frontends
$ brew install gcc-arm-none-eabi libusb minicom
```


### 2. Set up the build environment for STM32F4-Discovery board

#### Supported Nuttx version
|Repository|Tag Name|
|----------|:------:|
| nuttx | nuttx-7.19 |
| app | nuttx-7.19 |

We only guarantee that the specified version will work well. It is recommended to check out with the specified tag from a git repository.

#### Clone repository

Clone IoT.js and NuttX into iotjs-nuttx directory

```bash
$ mkdir iotjs-nuttx
$ cd iotjs-nuttx
$ git clone https://github.com/Samsung/iotjs.git
$ git clone https://bitbucket.org/nuttx/nuttx.git --branch nuttx-7.19
$ git clone https://bitbucket.org/nuttx/apps.git --branch nuttx-7.19
$ git clone https://github.com/texane/stlink.git
```

Note that we only support the specified git tag from nuttx repository

The following directory structure is created after these commands

```bash
iotjs-nuttx
  + apps
  + iotjs
  |  + config
  |      + nuttx
  |          + stm32f4dis
  + nuttx
  + stlink
```


### 3. Build NuttX (For the first time)

To generate headers which are required to build IoT.js, for the first time, you need to build NuttX at least once. This time NuttX build will be failed. But don't worry at this time. After one execution, you don't need this sequence any more.

#### Add IoT.js as a builtin application for NuttX

```bash
$ cd apps/system
$ mkdir iotjs
$ cp ../../iotjs/config/nuttx/stm32f4dis/app/* ./iotjs/
```

#### Configure NuttX

```bash
# assuming you are in iotjs-nuttx folder
$ cd nuttx/tools

# configure NuttX USB console shell
$ ./configure.sh stm32f4discovery/usbnsh
```

Now you can configure nuttx like either of below. For convenience, we provide built-in configure file for you. (This configure file is equipped with modules specified as `always`. For `optional` modules, you might follow instructions below.)
```bash
$ cd ..
$ cp ../iotjs/config/nuttx/stm32f4dis/.config.default .config
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

* For `i2c` module
  * Enable `System Type -> STM32 Peripheral Support -> I2C1`
  * Enable `Device Drivers -> I2C Driver Support`

* For `spi` module
  * Enable `System Type -> STM32 Peripheral Support -> SPI1`
  * Enable `Device Drivers -> SPI exchange`  
  
#### Build NuttX Context

```bash
# assuming you are in iotjs-nuttx folder
$ cd nuttx/
$ make context
```


### 4. Build IoT.js for NuttX

These options are needed.
```bash
--target-arch=arm
--target-os=nuttx
--nuttx-home=/path/to/nuttx
--target-board=stm32f4dis
--jerry-heaplimit=[..]
```

For example,
```bash
$ ./tools/build.py \
--target-arch=arm --target-os=nuttx --nuttx-home=../nuttx \
--target-board=stm32f4dis --jerry-heaplimit=78
```

Library files will be generated like below when build is successful.

```bash
$ ls build/arm-nuttx/release/lib
libhttpparser.a libiotjs.a libjerrycore.a libtuv.a
```

### 5. Build NuttX

```bash
# assuming you are in iotjs-nuttx folder
$ cd nuttx/
$ make IOTJS_ROOT_DIR=../iotjs
```
For release version, you can type R=1 make on the command shell.


### 6. Flashing

Connect Mini-USB for power supply and connect Micro-USB for `NSH` console.

To configure `stlink` utility for flashing, follow the instructions [here](https://github.com/texane/stlink#build-from-sources).

To install,
```bash
# assuming you are in stlink folder
$ cd stlink
$ make
```

To flash,
```bash
# assuming you are in nuttx folder
$ sudo ../stlink/build/Release/st-flash write nuttx.bin 0x8000000
```


### 7. Run IoT.js

#### USB Connection

There are two USB Connections on the Target board. USB mini CN1 and USB micro CN5. Both USB ports need to be connected to your Host. CN1 is used for power and Flashing, but it will not appear as a device in Linux. CN5 is used for NSH and will appear as `/dev/ttyACM0(linux)` or  `/dev/tty.usbmodem1(macOS)` when things work well.

#### Use minicom

```bash
// linux
$ minicom --device=/dev/ttyACM0
// macOS
$ minicom --device=/dev/tty.usbmodem1

```
You may need to enable _Add Carriage Return_ option.
* Press <kbd>Ctrl-A</kbd> + <kbd>Z</kbd> + <kbd>U</kbd> for short in minicom screen. (For linux user)
* Press <kbd>[Meta](http://osxdaily.com/2013/02/01/use-option-as-meta-key-in-mac-os-x-terminal/)</kbd> + <kbd>Z</kbd> for short in minicom screen. (For macOS user)

Press _Enter_ key several times to trigger NuttShell to start.

If micro SD is enabled, you can copy any script file to it and run with _nsh_, for example;
```
NuttShell (NSH)
nsh> mount -t vfat /dev/mmcsd0 /mnt/sdcard
nsh> iotjs /mnt/sdcard/path_to_file.js
```

If you see
```
+-----------------------------+                  
|                             |                  
|  Cannot open /dev/ttyACM0!  |                  
|                             |                  
+-----------------------------+
```
and it stays on the screen, something is wrong. Blue LED may blink if NuttX is in abnormal state. Press black(reset) button on the board and try again. If you still see this warning message, begin with original NuttX code and check your board, USB line and other softwares.
