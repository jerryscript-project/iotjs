## Jerry-debugger

Detailed description about the debugger is available
[here](https://github.com/jerryscript-project/jerryscript/blob/master/docs/07.DEBUGGER.md).

### Enable debugger support in IoT.js

To enable the debugger support under IoT.js, the `--jerry-debugger` option
should be passed to the `tools/build.py`. The server part of the debugger is
intergrated into the binary of IoT.js.

If you want to specify the port number of the debugger-server (default: 5001),
you can do so with the `--jerry-debugger-port=<PORT>` option.

### Usage

To start the debugger-server: `<iotjs binary> --start-debug-server test.js`

Two clients are included, a [python](https://github.com/jerryscript-project/jerryscript/blob/master/jerry-debugger/jerry-client-ws.py)
and an [HTML](https://github.com/jerryscript-project/jerryscript/blob/master/jerry-debugger/jerry-client-ws.html) variant, they can be found under `deps/jerry/jerry-debugger/`.

*Note*: When snapshot support is enabled, you won't be able to examine js-modules
that are loaded from snapshots.
