## Build IoT.js with RPi2

### Prerequisite
#### Linux

Install arm linux cross compiler.

``` bash
sudo apt-get install gcc-arm-linux-gnueabihf g++-arm-linux-gnueabihf
```

#### macOS

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

### Raspberry Pi 2 build
Give `target-arch`, `target-os` and `target-board` options to the script named 'build.py', then the script do the rest for you.

``` bash
./tools/build.py --buildtype=[release|debug] --target-arch=arm \
 --target-os=linux --target-board=rpi2
```

### Running in Raspberry Pi 2

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

### Building in Raspberry Pi 2 ?

Just executing below command will build IoT.js and run our testsuite.

``` bash
./tools/build.py --target-board rpi2
```
