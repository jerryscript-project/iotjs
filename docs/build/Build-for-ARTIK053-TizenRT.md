### About

This directory contains files to run IoT.js on [TizenRT](https://github.com/Samsung/TizenRT).

WARNING: **This document is not 100% accurate since Artik05x board with tooling is not available yet**

### How to build

#### 1. Set up the build environment for Artik05x board

Clone IoT.js and TizenRT into iotjs-tizenrt directory

```bash
$ mkdir iotjs-tizenrt
$ cd iotjs-tizenrt
$ git clone https://github.com/Samsung/iotjs.git
$ git clone https://github.com/Samsung/TizenRT.git tizenrt
```
The following directory structure is created after these commands

```bash
iotjs-tizenrt
  + iotjs
  |  + config
  |      + tizenrt
  |          + artik05x
  + tizenrt
```

#### 2. Add IoT.js as a builtin application for TizenRT

```bash
$ cd tizenrt/apps/system
$ mkdir iotjs
$ cp ../../../iotjs/config/tizenrt/artik05x/app/* ./iotjs/
```

**WARNING: Manual modification is required**

**WARNING: Below two bullet points are subject to change**

* change tizenrt/apps/system/Kconfig to include iotjs folder
    ```
    menu "IoT.js node.js like Javascript runtime"
    source "$APPSDIR/system/iotjs/Kconfig"
    endmenu
    ```
* Libraries required to link iotjs have to be supplied in some way
    ```
    EXTRA_LIBS = -lhttpparser -liotjs -ljerrycore -ltuv -ljerry-libm
    ```


#### 3. Configure TizenRT

```bash
$ cd tizenrt/os/tools
$ ./configure.sh sidk_s5jt200/hello_with_tash

$ cd ..
# might require to run "make menuconfig" twice
$ make menuconfig
```

#### 4. Build IoT.js for TizenRT

```bash
$ cd iotjs
$ ./tools/build.py --target-arch=arm --target-os=tizenrt --target-board=artik05x --sysroot=../tizenrt/os

```

#### 5. Build TizenRT

```bash
$ cd tizenrt/os
IOTJS_LIB_DIR=../iotjs/build/arm-tizenrt/debug/lib make
```
Binaries are available in `tizenrt/build/output/bin`

#### 6. Flashing

Yet to be announced on [TizenRT page](https://github.com/Samsung/TizenRT#board)
