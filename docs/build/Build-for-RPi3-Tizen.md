
### 1. Tizen on RPi3 GBS build

#### Prerequisites

* Install SDB tool (https://developer.tizen.org/development/tizen-studio/download)

  It is required to send file to target, which is in Tizen Studio.

* Modify sources.list of Ubuntu

  It is a buil system to create Tizen RPM package.
  To install gbs in your system, it's required to add the Tizen tools repository to the source list
  ```
  $ sudo vim /etc/apt/sources.list
  ```

  In Ubuntu 16.04, append the following line to the source list:  
  ``` bash
  deb [trusted=yes] http://download.tizen.org/tools/latest-release/Ubuntu_16.04/ /
  ```

  In Ubuntu 14.04, append the follwing line to the source list:
  ``` bash
  deb http://download.tizen.org/tools/latest-release/Ubuntu_14.04/ /
  ```

* Install GBS

  ``` bash
  sudo apt-get update
  sudo apt-get install gbs
  ```

* Prepare a GBS configuration file.

 You can use sample gbs configuration in config/tizen/sample.gbs.conf.

 See https://source.tizen.org/documentation/reference/git-build-system/configuration-file for details.


#### Building
* You can modify IoT.js build option on the spec file.
(config/tizen/packaging/iotjs.spec)  

* Run gbsbuild.sh at first.
Compile:
``` bash
./config/tizen/gbsbuild.sh
```

The following options are provided.
```
--debug: Build output is 'debug'. If this option is not specified, it is 'release'.
--clean: Make a clean gbs build by deleting the old build root.
```

### 2. Bring up RPi3 with Tizen
Please see the following guide to bring up your RPI3 target with Tizen.
You can refer "Raspberry Pi 3" section of command-line-flash part.

https://developer.tizen.org/development/iot-preview/getting-started/flashing-tizen-images#command-line-flash

#### Setting up serial port
 Please refer the tizen wiki  https://wiki.tizen.org/Raspberry_Pi#Debugging



#### Setting up IP

You can set up IP using WiFi or Ethernet

* Setup IP on RPi3 target using WiFi:

  https://developer.tizen.org/development/iot-preview/getting-started/flashing-tizen-images#wifi-setup


* Setup IP on RPi3 target using ethernet
``` bash
 User id/passwd : root / tizen
 (target)$ ifconfig eth0 down
 (target)$ ifconfig eth0 192.168.1.11 netmask 255.255.255.0 up
 (target)$ route add default gw 192.168.1.1
```

  If you want to use your fixed ip when you reboot, add ip settings in /etc/profile.

  Please make sure to run before modifying /etc/profile.
```
(target) $ mount -o remount,rw /
```
``` bash
 (ubuntu)$ sdb pull /etc/profile
 (ubuntu)$ vi profile

 Adding the following configurations
 ifconfig eth0 down
 ifconfig eth0 192.168.1.11 netmask 255.255.255.0 up
 route add default gw 192.168.1.1

 (ubuntu)$ sdb push profile /etc/
```

#### SDB connection
 Now you can connect RPi3 on Ubuntu PC

``` bash
(ubuntu)$ sdb connect 192.168.1.11
 ```

#### Install
Transfer iotjs binary and test file to the device:
``` bash
(ubuntu)$ sdb push  ~/GBS-ROOT/local/repos/tizen_unified_m1/armv7l/RPMS/iotjs-1.0.0-0.armv7l.rpm /tmp
(ubuntu)$ sdb push ./test/run_pass/test_console.js /home/owner/iotjs/
(ubuntu)$ sdb root on
(ubuntu)$ sdb shell
(target)$ cd /tmp
(only in headless Tizen 4.0 target)$ mount -o remount,rw /
(target)$ rpm -ivh --force iotjs-1.0.0-0.armv7l.rpm
```

#### Run the test:
``` bash
(ubuntu)$ sdb shell
(target)$ iotjs test_console.js
```
