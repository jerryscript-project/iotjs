It's not that different for ARM-Linux targets. It's almost same as the [Build for Linux](Build-for-Linux.md) page.


### Prerequisite

Install arm linux cross compiler.

```
sudo apt-get install gcc-arm-linux-gnueabihf g++-arm-linux-gnueabihf
```


### Raspberry Pi 2 build
Give `target-arch`, `target-os` and `target-board` options to the script named 'build.py', then the script do the rest for you.

```
./tools/build.py --buildtype=release --no-init-submodule \
--target-arch=arm --target-os=linux --target-board=rpi2
```

### Running in Raspberry Pi 2

This script gives you `build/arm-linux/release/iotjs/iotjs` or `build/arm-linux/debug/iotjs/iotjs`.
Copy this binary with your favorite tool or `scp` like below.

```
scp build/arm-linux/release/iotjs/iotjs pi@(your RPi2 IP):/home/pi/.
```

Lastly, open a shell and run with your test program.

```
ssh pi@(your RPi2 IP)
./iotjs (your test program)
```

### Building in Raspberry Pi 2 ?

Not yet. Maybe someone out there?
