# Sample bridge module

See also:
* [Writing-new-module](Writing-New-Module.md)
* [Native Module vs. JS module](Native-Module-vs-JS-Module.md)
* [Inside IoT.js](Inside-IoT.js.md)
* [Developer Tutorial](Developer-Tutorial.md)


## Description
This sample show you how you can create a 'mixed' module using brige module that has some interfaces to support communicattion between JS and Native code. This sample created using tools/iotjs-create-module.py script.
You can see how you could reduce your effor to create native module using simple methods provided bridge module.


## Build

$ ./tools/build.py --external-modules=./samples/bridge_sample --cmake-param=-DENABLE_MODULE_BRIDGE_SAMPLE=ON

## Testing

$ iotjs samples/bridge_sample/test.js
