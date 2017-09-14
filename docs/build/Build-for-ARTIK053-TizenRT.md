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
git clone https://github.com/pmarcinkiew/iotjs.git
export IOTJS_ROOT_DIR="$PWD/iotjs"
```
The following directory structure is created after these commands

```bash
iotjs
  + config
  |   + tizenrt
  |       + artik05x
```

#### 2. Build IoT.js for TizenRT (and ROMFS image)

```bash
cd ${IOTJS_ROOT_DIR}
tools/precommit.py --test=artik053 --buildtype=debug --romfs=tests
```

> :grey_exclamation: Trouble Shooting: Building IoT.js fails: You may encounter `arm-none-eabi-gcc: Command not found` error message while building IoT.js on a 64-bit system. This may be because the above toolchain you set uses 32-bit libs. For this matter, install the below toolchain as alternative.
> ```
> $ sudo apt-get install -y gcc-arm-none-eabi 
> ```


#### 2A. Build ROMFS separately

```bash
genromfs -f ${IOTJS_ROOT_DIR}/deps/tizenrt/output/bin/rom.img -d <romfs contentdir> -V "NuttXBootVol"
```

Binaries (firmare image parts, romfs image file) are available in `TizenRT/build/output/bin`

#### 3. Flashing

```bash
pushd ${IOTJS_ROOT_DIR}/deps/tizenrt/os
make download ALL
popd
```
> :grey_exclamation: Trouble Shooting: Flashing the binary via USB fails: Refer to [add-usb-device-rules](https://github.com/Samsung/TizenRT/blob/master/build/configs/artik053/README.md#add-usb-device-rules). Your `VendorID:ProductID` pair can be found in `lsusb` output as the below instance.
>
>```
>$ lsusb
>Bus 004 Device 001: ID 1d6b:0003 Linux Foundation 3.0 root hub
>Bus 003 Device 005: ID 0403:6010 Future Technology Devices International, Ltd >FT2232C Dual USB-UART/FIFO IC
>```
