# IoT.js for Windows build guide

> :exclamation: This document describes an experimental feature and considerations.
Please be aware that every experimental feature may change, be broken,
or be removed in the future without any notice.


The document presents the steps required to compile the IoT.js
for Windows.
Tested on Windows 10 and with Visual Studio 2017 Community Edition.

## Build IoT.js for Windows

### 0. Check environment

Check if the following tools are installed:
 * GIT
 * Visual Studio 2017 with C++ support
 * Python 3

Additionally clone the IoT.js repository into a convenient directory.
In the document the `C:\dev\iotjs` path will be used as an example.

### 1. Run the build script

To create the required Visual Studio solution file(s) the build scripts needs to be
executed. Please start a Command Prompt and navigate to the source directory where
the IoT.js is cloned.

In the IoT.js directory issue the following command:

```sh
C:\dev\iotjs> .\tools\build.py --experimental
```

Currently for Windows the `--experimental` option is a must. Additionally if
other options are required it can be specified after this option.

This command will create the solution files in the build directory.
Specifically in the `build\i686-windows\debug` directory in case of debug build.
In case of release build the solution files will be in the `build\i686-windows\release\`
directory.

Please note that currently only the `i686` target is supported.

### 2. Open the IoT.js solution file

In the `build\i686-windows\debug` directory the `IOTJS.sln` file should be opened
with Visual Studion 2017.

### 3. Build

After the IoT.js solution file is opened the Visual Studio can now start the build.
Press CTRL+SHIFT+B to build the whole solution.

The resulting iotjs.exe will be placed in the build's `bin\Debug` or `bin\Release`
directory depending on the configuration chosen in the Visual Studio.

### Extra

On Windows the test runner can also be executed. To do this the following steps are required:

1. Have a built iotjs.exe
2. Start a command prompt
3. Navigate to the IoT.js source directory
4. Execute the test runner. Ex.:
```sh
C:\dev\iotjs> tools\testrunner.py build\i686-windows\debug\bin\Debug\iotjs.exe
```
