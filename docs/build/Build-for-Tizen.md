## Build IoT.js for Tizen

### Build with GBS

#### Prerequisites

 * Tizen uses GBS to create RPM packages.
  GBS and SDB tools are in Tizen Studio. Please, install tizen studio.
 (https://developer.tizen.org/development/tizen-studio/download)
 * Tizen Studio with CLI also includes GBS.

 * Create a GBS configuration file at ~/.gbs.conf.
 (https://source.tizen.org/documentation/reference/git-build-system/configuration-file)
 * You can use this sample, /config/tizen/.gbs.conf for GBS build.

#### Building

 * You can modify IoT.js build option on spec file. (config/tizen/packaging/iotjs.spec)
Compile:
``` bash
gbs build -A armv7l --include-all
```

#### Install
Transfer iotjs binary and test file to the device:
``` bash
sdb push  ~/GBS-ROOT/local/repos/tizen_unified/armv7l/RPMS/iotjs-1.0.0-0.armv7l.rpm /tmp
sdb push ./test/run_pass/test_console.js /home/owner/iotjs/
sdb root on
sdb shell
(target)$ cd /tmp
(target)$ rpm -ivh --force iotjs-1.0.0.rpm

```

Run the test:
``` bash
sdb shell
$ cd /home/owner/iotjs
$ ./iotjs test_console.js
```
