
### 1. Tizen on ARTIK10 cross-compile

#### Prerequisites

* ARTIK10 with Tizen (https://wiki.tizen.org/wiki/Tizen_On_ARTIK)  
* Tizen Studio with Native app development CLI tools.  
  This is required to get rootstrap for Tizen (set of native libraries).  

* arm-linux-gnueabi-gcc cross compiler (can be found in Tizen Studio / Native toolchain)  
  Otherwise, you can install it on your PC.  
```bash
sudo apt-get install gcc-arm-linux-gnueabi g++-arm-linux-gnueabi
```

#### Building
1. Make sure arm-linux-gnueabi-gcc is in path.
2. Locate Tizen SDK. Default location is: ~/tizen-studio.  
3. In platforms/tizen-3.0/mobile there should be compatible rootstrap (eg. mobile-3.0-device)  

Compile:
* Compile with rootstrap in case you use tizen 3.0 libraries.  
``` bash
tools/build.py \
  --target-arch=arm --target-os=tizen --target-board=artik10 \
  --compile-flag="--sysroot=~/tizen-studio/platforms/tizen-3.0/mobile/rootstraps/mobile-3.0-device.core/"
```

#### Testing
Transfer iotjs binary and test file to the device:  
``` bash
 $ sdb push ./build/arm-tizen/debug/bin/iotjs /home/owner/iotjs/
 $ sdb push ./test/run_pass/test_console.js /home/owner/iotjs/
```

Run the test:
``` bash
sdb shell
$ cd /home/owner/iotjs
$ ./iotjs test_console.js
```
