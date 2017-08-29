### About

This directory contains files to run IoT.js on [TizenRT](https://github.com/Samsung/TizenRT).


### How to build

#### 1. Set up the build environment for Artik05x board

* Install toolchain

Get the build in binaries and libraries, [gcc-arm-none-eabi-4_9-2015q3-20150921-linux.tar](https://launchpad.net/gcc-arm-embedded/4.9/4.9-2015-q3-update).

Untar the gcc-arm-none-eabi-4_9-2015q3-20150921-linux.tar and export the path like

```
tar xvf gcc-arm-none-eabi-4_9-2015q3-20150921-linux.tar
export PATH=<Your Toolchain PATH>:$PATH
```

* Get IoT.js and TizenRT sources

Clone IoT.js and TizenRT into iotjs-tizenrt directory

```bash
mkdir iotjs-tizenrt
cd iotjs-tizenrt
git clone https://github.com/Samsung/iotjs.git
git clone https://github.com/Samsung/TizenRT.git
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
cp iotjs/config/tizenrt/artik05x/app/ TizenRT/apps/system/iotjs -r
cp iotjs/config/tizenrt/artik05x/configs/ TizenRT/build/configs/artik053/iotjs -r
cp iotjs/config/tizenrt/artik05x/romfs.patch TizenRT/
```

#### 3. Configure TizenRT

```bash
cd TizenRT/os/tools
./configure.sh artik053/iotjs
```

#### 4. Configure ROMFS of TizenRT

```bash
cd ../../
patch -p0 < romfs.patch
cd build/output/
mkdir res
# You can add files in res folder
# The res folder is later flashing into the target's /rom folder
```

#### 5. Build IoT.js for TizenRT

```bash
cd ../../os
make context
cd ../../iotjs
./tools/build.py --target-arch=arm --target-os=tizenrt --sysroot=../TizenRT/os --target-board=artik05x --clean
```

> :grey_exclamation: Trouble Shooting: Building IoT.js fails: You may encounter `arm-none-eabi-gcc: Command not found` error message while building IoT.js on a 64-bit system. This may be because the above toolchain you set uses 32-bit libs. For this matter, install the below toolchain as alternative.
> ```
> $ sudo apt-get install -y gcc-arm-none-eabi 
> ```


#### 6. Build TizenRT

```bash
cd ../TizenRT/os
make
genromfs -f ../build/output/bin/rom.img -d ../build/output/res/ -V "NuttXBootVol"
```
Binaries are available in `TizenRT/build/output/bin`

#### 7. Flashing

```bash
make download ALL
```
> :grey_exclamation: Trouble Shooting: Flashing the binary via USB fails: Refer to [add-usb-device-rules](https://github.com/Samsung/TizenRT/blob/master/build/configs/artik053/README.md#add-usb-device-rules). Your `VendorID:ProductID` pair can be found in `lsusb` output as the below instance.
>
>```
>$ lsusb
>Bus 004 Device 001: ID 1d6b:0003 Linux Foundation 3.0 root hub
>Bus 003 Device 005: ID 0403:6010 Future Technology Devices International, Ltd >FT2232C Dual USB-UART/FIFO IC
>```