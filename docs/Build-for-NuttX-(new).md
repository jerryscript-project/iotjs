#### About

This page describes short version to use with latest version of NuttX for STM32F4-Discovery with BB.

Please read [Build for NuttX](https://github.com/Samsung/iotjs/wiki/Build-for-NuttX) page and understand full procedure.

#### Get NuttX for IoT.js

Follow [Building nuttx for iotjs](https://bitbucket.org/seanshpark/nuttx/wiki/Home) page.

You should first build NuttX before building IoT.js, it depends on system header files. When NuttX tells you that it needs `libtuv` then come here again and continue with below sections.

#### Build IoT.js for NuttX

Giving these options are needed
```
--target-arch=arm
--target-os=nuttx
--nuttx-home=(where .config file exist)
--target-board=stm32f4dis
```

for example,
```
./tools/build.py \
--target-arch=arm --target-os=nuttx --nuttx-home=/home/user/harmony/nuttx/nuttx \
--target-board=stm32f4dis --jerry-heaplimit=78 \
--buildtype=release
```

Library files will be copied to nuttx/lib folder when build is success.

Now continue on with NuttX build.

#### NuttX build and flashing

in nuttx
```
make
sudo ../../stlink/st-flash write nuttx.bin 0x8000000
```

#### NuttX configuration

Below is current NuttX configuration so that you can try with IoT.js

1) Ethernet for network, TCP/IP is available
2) Micro SD-Card, to save JavaScript files for your application


#### Trouble shooting

##### conversion to 'unsigned char:1' from 'uint32_t

If you get an error something like this,
```
iotjs/deps/jerry/jerry-core/jerry.cpp:1605:26: 
  error: conversion to 'unsigned char:1' from 'uint32_t {aka unsigned int}' 
  may alter its value [-Werror=conversion]
```
it's about [this, #279](https://github.com/Samsung/iotjs/pull/279). Updating nuttx/configs source to latest version and run `./configure.sh stm32f4discovery/iotjs` in `nuttx/tools` may solve the problem.

##### redeclaration of C++ built-in type 'wchar_t'

If tis error occurs when building IoT.js, you should define preprocessor variable `CONFIG_WCHAR_BUILTIN` when building IoT.js.

cmake/config.cmake:
```cmake
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
 set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions")
+set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DCONFIG_WCHAR_BUILTIN")
 set(IOTJS_CFLAGS "${IOTJS_CFLAGS} ${CFLAGS_COMMON}")
```

##### unknown type name 'wint_t'
If this error occurs when building jerry, you should define `wint_t` type in source file.

deps/jerry/jerry-core/vm/vm.c:
```c
typedef int wint_t;
```

##### undefined reference to 'uv_cleanup'
If this error occurs when building nuttx, you should replace `uv_cleanup` into `tuv_cleanup`.

/home/user/harmony/nuttx/apps/system/iotjs/iotjs.cxx:
```c
-extern "C" int uv_cleanup(void);
+extern "C" int tuv_cleanup(void);
...
-uv_cleanup();
+tuv_cleanup();
```

##### No rule to make target 'lib/libfdlibm.a', needed by 'pass2deps'
If this error occurs when building nuttx, you should build `libfdlibm.a` and copy it into nuttx lib directory.
```sh
make -C build/arm-nuttx/release/deps/jerry release.jerry-libm.lib
cp ./build/arm-nuttx/release/deps/jerry/jerry-libm/librelease.jerry-libm.lib.a \
    /home/user/harmony/nuttx/nuttx/lib/libfdlibm.a
```
