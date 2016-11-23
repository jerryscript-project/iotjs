## Build IoT.js with NuttX

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

### 2. Build NuttX (For the first time)

Currently we checked that NuttX with commit id ** 419fd6e ** works well. To generate headers which are required to build IoT.js, for the first time, you need to build NuttX at least once. This time nuttx build will be failed. But don't worry at this time. After one execution, you don't need this sequence any more.

#### Follow the instruction
* [STM32F4-discovery](../targets/nuttx-stm32f4/README.md)


### 3. Build IoT.js for NuttX

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

### 4. Build NuttX

This time make command for NuttX has to be successful unlike above.

#### Follow the instruction
* [STM32F4-discovery](../targets/nuttx-stm32f4/README.md)

### 5. Run IoT.js

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
