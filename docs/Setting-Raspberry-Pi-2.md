There is nothing special about setting Raspberry Pi 2 (RPi2 for short) for IoT.js, but these are the steps to prepare RPi2 that we work on.

### Preparation
* Raspberry Pi 2 board 
* MicroSD at least 8GB and USB Adaptor
* Network

### Download image file
Raspbian is used throughout this page. See [downloads page](https://www.raspberrypi.org/downloads/).
Download URL is http://downloads.raspberrypi.org/raspbian_latest

### Write to MicroSD
Follow the steps as described in [Installing-images](https://www.raspberrypi.org/documentation/installation/installing-images/README.md) page.

### First Boot
Plugin MicroSD and power up. `raspi-config` will show up.
Read [raspi-config](https://www.raspberrypi.org/documentation/configuration/raspi-config.md) page and look around.
This is what I've done.
* Expand FileSystem
* Change User Password
    * default user id is `pi` and password is `raspberry`
* Change Locale and Timezone
* In advanced options, disable Overscan
* Set hostname, enable SSH

### After reboot
* login with `pi`
* Set Network, default is DHCP. If you prefer static IP
```
sudo vi /etc/network/interfaces
```
in my case;
```
auto lo
iface lo inet loopback
iface eth0 inet static
 
address a.b.c.d
netmask 255.255.255.0
network a.b.c.0
broadcast a.b.c.255
gateway a.b.c.1
 
allow-hotplug wlan0
iface wlan0 inet manual
wpa-roam /etc/wpa_supplicant/wpa_supplicant.conf
iface default inet dhcp
```
set `a.b.c` to your address.
* name server
```
sudo vi /etc/resolv.conf
```
append your nameservers

* and other things
   * proxy environment
   * apt-get proxy


### Reference
* https://www.raspberrypi.org/documentation/installation/installing-images/linux.md

### Using Ubuntu image
You can also use Ubuntu instead of Raspbian.
Setup instructions are the same as above, the only difference is to burn sdcard with Ubuntu image.
You can download the image from [here](https://wiki.ubuntu.com/ARM/RaspberryPi).
