## Jerry-debugger

Detailed description about the debugger is available
[here](https://github.com/jerryscript-project/jerryscript/blob/master/docs/07.DEBUGGER.md).

### Enable debugger support in IoT.js

To enable the debugger support under IoT.js, the `--jerry-debugger` option
should be passed to the `tools/build.py`. The server part of the debugger is
intergrated into the binary of IoT.js.

### Usage

To start the debugger-server: `<iotjs binary> --start-debug-server test.js`

It is important to note that optional parameters (such as `--debugger-wait-source` or
`--debugger-port <PORT>`) should be specified after `--start-debug-server` in order to work properly.

#### Sending source to the debugger remotely

The `--debugger-wait-source` makes the client wait until the source files are sent by the debugger-client.
The file argument is ignored in this case, therefore doesn't need to be specified. IoT.js is also capable of resetting the context,
thus, there's no need to restart the environment if the remote source is changed.
**Important note**: Remote sources must be sent in correct order! IoT.js compiles them in the order they are received, so file(s) used with `require` should be sent first, and the file(s) using them after.

#### Select Channel and Protocol

There are two available extension-provided channels, websocket and rawpacket, and two protocols, tcp and serial. Each initializes the debugger and blocks until a client connects. If you want to specify the debugger channel (default: websocket) or protocol (default: tcp) over the communication you can do with the `--debug-channel [websocket|rawpacket]` and `--debug-protocol [tcp|serial]` options:

`<iotjs binary> --start-debug-server --debugger-channel rawpacket --debug-protocol tcp test.js`

#### Setting the debugger port

If you want to specify the port number of the debugger-server with tcp connection (default: 5001),
you can do so with the `--debugger-port <PORT>` option:

`<iotjs binary> --start-debug-server --debugger-port 8080 test.js`

#### Configure the serial port

If you want to configure parameters for serial port (default: /dev/ttyS0,115200,8,N,1), you can do with `--debug-serial-config CONFIG` option:

`<iotjs binary> --start-debug-server --debug-channel rawpacket --debug-protocol serial --debug-serial-config "/dev/ttyUSB0,115200,8,N,1" test.js`


#### Available Clients

* [JerryScript console debugger client](https://github.com/jerryscript-project/jerryscript/blob/master/jerry-debugger/jerry-client-ws.py)
* [Iot.js Code](https://github.com/jerryscript-project/iotjscode)
* [Jerryscript debugger Chrome webtool](https://github.com/jerryscript-project/jerryscript-debugger-ts)

**Note**: When snapshot support is enabled, you won't be able to examine js-modules
that are loaded from snapshots.
