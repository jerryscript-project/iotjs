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
$ git clone https://github.com/Samsung/TizenRT.git
```
The following directory structure is created after these commands

```bash
iotjs-tizenrt
  + iotjs
  |  + config
  |      + tizenrt
  |          + artik05x
  + TizenRT
```

#### 2. Add IoT.js as a builtin application for TizenRT

```bash
$ cp iotjs/config/tizenrt/artik05x/app/ TizenRT/apps/system/iotjs -r
$ cp iotjs/config/tizenrt/artik05x/configs/ TizenRT/build/configs/artik053/iotjs -r
$ cp iotjs/config/tizenrt/artik05x/romfs.patch TizenRT/
```

#### 3. Configure TizenRT

```bash
$ cd TizenRT/os/tools
$ ./configure.sh artik053/iotjs
```

#### 4. Configure ROMFS of TizenRT

```bash
$ cd ../../
$ patch -p0 < romfs.patch
$ cd build/output/
$ mkdir res
# You can add files in res folder
# The res folder is later flashing into the target's /rom folder
```

#### 5. Build IoT.js for TizenRT

```bash
$ cd os
$ make context
$ cd ../../iotjs
$ ./tools/build.py --target-arch=arm --target-os=tizenrt --sysroot=../TizenRT/os --target-board=artik05x --clean
```

#### 6. Build TizenRT

```bash
$ cd ../TizenRT/os
$ make
$ genromfs -f ../build/output/bin/rom.img -d ../build/output/res/ -V "NuttXBootVol"
```
Binaries are available in `TizenRT/build/output/bin`

#### 7. Flashing

```bash
$ make download ALL
```
