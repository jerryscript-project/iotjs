
### 1. Tizen on RPi3 GBS build

#### Prerequisites

* Install SDB tool (https://developer.tizen.org/development/tizen-studio/download)

  It is required to send file to target, which is in Tizen Studio.

* Install GBS

  It is required to create Tizen RPM package.

``` bash
sudo apt-get install gbs
```

* Prepare a GBS configuration file.

 You can use sample gbs configuration in config/tizen/sample.gbs.conf.
 (Please add your Tizen.org id and password on this conf file.)

 See https://source.tizen.org/documentation/reference/git-build-system/configuration-file for details.


#### Building
* You can modify IoT.js build option on the spec file.  
 (config/tizen/packaging/iotjs.spec)  
* Run gbsbuild.sh at first.
Compile:
``` bash
./config/tizen/gbsbuild.sh
```
After finishing build, move to a new working directory at '../iotjs_tizen_gbs/'.  
Next time, build with this basic command.
```bash
gbs -c config/tizen/sample.gbs.conf build -A armv7l --include-all
```

### 2. Bring up RPi3 with Tizen
This guide for the RPi3 is a temporary solution until releasing official guide of Tizen sites.

#### Install pv package
``` bash
$ sudo apt-get install pv
```

#### Downloading fusing-script and firmwares
``` bash
  $ wget https://git.tizen.org/cgit/platform/kernel/u-boot/plain/scripts/tizen/sd_fusing_rpi3.sh?h=tizen --output-document=sd_fusing_rpi3.sh
  $ chmod 755 sd_fusing_rpi3.sh
  $ wget https://github.com/RPi-Distro/firmware-nonfree/raw/master/brcm80211/brcm/brcmfmac43430-sdio.bin
  $ wget https://github.com/RPi-Distro/firmware-nonfree/raw/master/brcm80211/brcm/brcmfmac43430-sdio.txt
  $ wget https://github.com/OpenELEC/misc-firmware/raw/master/firmware/brcm/BCM43430A1.hcd
```

#### Downloading TizenIoT Core Image for RPi3

Kernel & Module Image
http://download.tizen.org/snapshots/tizen/unified/latest/images/standard/iot-boot-arm64-rpi3/

Tizen Platform Image
http://download.tizen.org/snapshots/tizen/unified/latest/images/standard/iot-headless-2parts-armv7l-rpi3/

#### Fusing images to sd-card
``` bash
  $ sudo ./sd_fusing_rpi3.sh -d /dev/sdb --format
  $ sudo ./sd_fusing_rpi3.sh -d /dev/sdb -b tizen-unified_20170704.1_common-iot_core-2parts-armv7l-rpi3.tar.gz
  $ sudo ./sd_fusing_rpi3.sh -d /dev/sdb -b tizen-unified_20170704.1_common-boot-arm64-rpi3.tar.gz
```

#### Copying firmwares for wifi and bluetooth
``` bash
 $ mkdir rootfs
 $ sudo mount /dev/sdb2 rootfs
 $ sudo mkdir -p rootfs/usr/etc/bluetooth
 $ sudo cp BCM43430A1.hcd rootfs/usr/etc/bluetooth
 $ sudo mkdir -p rootfs/usr/lib/firmware/brcm
 $ sudo cp brcmfmac43430-sdio.* rootfs/usr/lib/firmware/brcm
 $ sync
 $ sudo umount rootfs
 $ rmdir rootfs
```

#### Setting up serial port
 Please refer the tizen wiki  https://wiki.tizen.org/Raspberry_Pi#Debugging

#### Setup IP
Setup IP on RPi3 target using serial port
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
(ubuntu)$ sdb push  ~/GBS-ROOT/local/repos/tizen_unified/armv7l/RPMS/iotjs-1.0.0-0.armv7l.rpm /tmp
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
