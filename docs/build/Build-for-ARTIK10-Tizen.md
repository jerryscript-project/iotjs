
### 1. Tizen on ARTIK10 cross-compile

#### Prerequisites

* ARTIK10 with Tizen (https://wiki.tizen.org/wiki/Tizen_On_ARTIK)
* Tizen Studio with Native app development CLI tools
  This is required to get rootstrap for Tizen (set of native libraries).
* arm-linux-gnueabi-gcc cross compiler (can be found in Tizen Studio / Native toolchain)
  Otherwise, you can install on your PC.
```bash
sudo apt-get install gcc-arm-linux-gnueabi g++-arm-linux-gnueabi
```
  
#### Building
1. Make sure arm-linux-gnueabi-gcc is in path
2. Locate Tizen SDK. Default location is: ~/tizen-studio
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
sdb push ./build/arm-tizen/debug/bin/iotjs /home/owner/iotjs/
sdb push ./test/run_pass/test_console.js /home/owner/iotjs/
```

Run the test:
``` bash
sdb shell
$ cd /home/owner/iotjs
$ ./iotjs test_console.js
```


### 2. Build with GBS
#### Prerequisites
* Tizen uses GBS to create RPM packages.
 GBS and SDB tools are in Tizen Studio. Please, install tizen studio. 
 (https://developer.tizen.org/development/tizen-studio/download)
* Tizen Studio with CLI also includes GBS.
* To run GBS, please create a GBS configuration file at '~/.gbs.conf'
 (https://source.tizen.org/documentation/reference/git-build-system/configuration-file)
 You can use this sample, /config/tizen/sample.gbs.conf for GBS build.
 
#### Building
* You can modify IoT.js build option on spec file. (config/tizen/packaging/iotjs.spec)

Compile:
``` bash
cp ./config/tizen/gbsbuild.sh ./
./gbsbuild.sh
```
 
#### Install

Transfer iotjs binary and test file to the device:
``` bash
sdb push  ~/GBS-ROOT/local/repos/tizen_unified/armv7l/RPMS/iotjs-1.0.0-0.armv7l.rpm /tmp
sdb push ./test/run_pass/test_console.js /home/owner/iotjs/
sdb root on
sdb shell
(target)$ cd /tmp
(only in headless Tizen 4.0 target)$ mount -o remount,rw
(target)$ rpm -ivh --force iotjs-1.0.0.rpm
```

Run the test:
``` bash
sdb shell
$ iotjs test_console.js
```