# Build IoT.js with mraa for Intel Edison

Edison board has Yocto Linux OS, so it is an enhanced case in the [Build for Linux](https://github.com/Samsung/iotjs/wiki/Build-for-Linux) page.
Edison board has mraa inside, so you can implement IoT.js's gpio module with mraa. See the [#295](https://github.com/Samsung/iotjs/pull/295) page.

## Overall steps

1. Get the libmraa.so and related headers in Edison's Yocto OS, and put them into the host PC (Ubuntu 14.04)
2. Build the iotjs with mraa in the host PC
3. Copy the binary into the Edison Yocto OS.

## Prerequisite

* mkdir a folder in iotjs for storing mraa related files.

```
git clone https://github.com/Samsung/iotjs.git
cd iotjs
mkdir mraa
```

## 1. Get the mraa library and headers

Step 1, 3 and 4 of [this official page](https://software.intel.com/en-us/get-started-edison-linux) describe how to get started with edison and how to setup its network. Then we can get Edison's IP.

In this step, we use `scp` to get the mraa library/headers in Edison. You can also use SD-card, USB-stick or other network tools.

In host PC,
```
cd mraa
scp root@{edison ip}:/usr/lib/libmraa.so .
scp root@{edison ip}:/usr/lib/libft4222.so .
scp root@{edison ip}:/usr/include/mraa.h .
scp -r root@{edison ip}:/usr/include/mraa .
```

## 2. Build IoT.js with mraa and i686

In the `iotjs` directory,
```
./tools/build.py \
--target-arch=i686 \
--compile-flag="-DUSING_MRAA" \
--external-include-dir="/home/jzd/edison/mraa" \
--external-shared-lib="/home/jzd/edison/mraa/libmraa.so"
```
**NOTE:** You should replace the above path with actual path in your host.
The Yocto mraa.h will generate problems with check_tidy.py if it is inside the iotjs project.

## 3. Copy the binary into Edison

```
scp build/i686-linux/debug/iotjs/iotjs  root@{edison ip}:/home/root/iotjs
```

## Example

In the Edison, you can run the following code to turn on the board LED (for Arduino Extended board)

led_on.js
```javascript
var gpio = require("gpio");
gpio.initialize(init_done);
function init_done() {
  print("init done!");
  gpio.setPin(13, "out", set_done);// the led is in the board pin 13
}
function set_done() {
  print("set done!");
  gpio.writePin(13, 1);
}
```
 Then in the Edison shell, run
```
cd /home/root
./iotjs led_on.js
```

## Optional steps

For better experiences in Edison, you can upgrade its firmware to the latest version. See https://software.intel.com/en-us/flashing-your-firmware-edison for more details.
