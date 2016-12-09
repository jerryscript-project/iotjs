## Build IoT.js with Raspberry Pi 2

IoT.js supports two build types:

1. Build on your desktop. We support Linux(Ubuntu) and macOS. - Cross compile
2. Build on Raspberry Pi 2.

### Setting Raspberry Pi

IoT.js officially supports Raspbian. For more information, please visit [the official site](https://www.raspberrypi.org/downloads/raspbian/).

#### Enable the I2C interface

To use I2C module, the I2C interface must be enabled.

From the command line type:
```bash
sudo raspi-config
```
This will launch raspi-config utility.
   * Select "9 Advanced Options"
   * Select "A6 I2C"

The screen will ask you to enable I2C interface.
   * Select "Yes"
   * Select "Ok"
   * Select "Finish" to return to the command line.

Reboot your Raspberry Pi.

#### Enable the PWM interface

Raspberry Pi2 has two PWM outputs on the following pins.

| PWM Number | GPIO PIN(FUNC) |
| :---: | :---: |
| PWM0 | GPIO12(4), GPIO18(2) |
| PWM1 | GPIO13(4), GPIO19(2) |

To use PWM module, you must add PWM overlays in `/boot/config.txt` file.

For example, to get a single PWM on GPIO18, add overlays like below.
```
dtoverlay=pwm,pin=18,func=2
```

For example, to get multi PWM on GPIO18 and GPIO19, add overlays like below.
```
dtoverlay=pwm-2chan,pin=18,func=2,pin2=19,func2=2
```

For more information about overlays, refer to [README](https://github.com/raspberrypi/linux/blob/rpi-4.9.y/arch/arm/boot/dts/overlays/README).

#### Enable the UART interface

To use UART module, the UART interface must be enabled.

In `/boot/config.txt` file, change the value of enable_uart from 0 to 1.
```
enable_uart=1
```

To disable the serial console, edit the file `/boot/cmdline.txt`.
remove the word phase ```"console=serial0,115200"``` or ```"console=ttyAMA0,115200"```

To enable the serial console, edit the file `/boot/cmdline.txt`.
add the word phase ```"console=serial0,115200"``` or ```"console=ttyAMA0,115200"```

Reboot your Raspberry Pi.

* Note for Raspberry Pi 3 : You should use /dev/ttyS0 instead of /dev/ttyAMA0 in RPI3.

### Build IoT.js on your desktop.

#### Prerequisite
##### Linux

Install arm linux cross compiler.

``` bash
sudo apt-get install gcc-arm-linux-gnueabihf g++-arm-linux-gnueabihf
```

##### macOS

Install arm linux cross compiler via [this site](http://www.welzels.de/blog/en/arm-cross-compiling-with-mac-os-x/).

The default location for arm linux compiler toolchain is **"/usr/local/linaro/arm-linux-gnueabihf-raspbian"**.

Then you need to locate c_compiler and cxx_compiler.
In **"./cmake/config/arm-linux.cmake"**,
``` cmake
SET(EXTERNAL_CMAKE_C_COMPILER
    /usr/local/linaro/arm-linux-gnueabihf-raspbian/bin/arm-linux-gnueabihf-gcc)
SET(EXTERNAL_CMAKE_CXX_COMPILER
    /usr/local/linaro/arm-linux-gnueabihf-raspbian/bin/arm-linux-gnueabihf-g++)
```
In **"./deps/libtuv/cmake/config/config_arm-linux.cmake"**,
``` cmake
SET(CMAKE_C_COMPILER
    /usr/local/linaro/arm-linux-gnueabihf-raspbian/bin/arm-linux-gnueabihf-gcc)
SET(CMAKE_CXX_COMPILER
    /usr/local/linaro/arm-linux-gnueabihf-raspbian/bin/arm-linux-gnueabihf-g++)
```

#### Build IoT.js (Cross compile)
Give `target-arch`, `target-os` and `target-board` options to the script named 'build.py', then the script do the rest for you.

``` bash
./tools/build.py --buildtype=[release|debug] --target-arch=arm \
 --target-os=linux --target-board=rpi2
```

#### Running in Raspberry Pi 2

This script gives you `build/arm-linux/release/iotjs/iotjs` or `build/arm-linux/debug/iotjs/iotjs`.
Copy this binary with your favorite tool or `scp` like below.

``` bash
scp build/arm-linux/release/iotjs/iotjs pi@(your RPi2 IP):/home/pi/.
```

Lastly, open a shell and run with your test program.

``` bash
ssh pi@(your RPi2 IP)
./iotjs (your test program)
```

### Build IoT.js on Raspberry Pi 2

#### Prerequisite
Install cmake.
```bash
sudo apt-get update
sudo apt-get install cmake
```

#### Build IoT.js
Executing below command will build IoT.js and run our testsuite.

``` bash
./tools/build.py --target-board=rpi2
```
