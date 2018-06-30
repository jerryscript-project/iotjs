### Platform Support

The following chart shows the availability of each WebSocket module API function on each platform.

|  | Linux<br/>(Ubuntu) | Tizen<br/>(Raspberry Pi) | Raspbian<br/>(Raspberry Pi) | Nuttx<br/>(STM32F4-Discovery) | TizenRT<br/>(Artik053) |
| :---: | :---: | :---: | :---: | :---: | :---: |
| websocket.connect | O | O | O | X | O |
| websocket.close | O | O | O | X | O |
| websocket.ping | O | O | O | X | O |
| websocket.send  | O | O | O | X | O |

# WebSocket

WebSocket provides full-duplex communication over a TCP connection. It is designed to work over HTTP ports 80 and 443.

### Requirements
WebSocket requires you to enable both the `TLS` and the `WebSocket` module. This can be done by compiling IoT.js with `--cmake-param=-DENABLE_MODULE_WEBSOCKET=ON`. Currently WebSocket only works if TLS is enabled as well.

## Class: Websocket
The `Websocket` client can simultaneously receive and send data. Both `net` and `TLS` sockets are supported, however the latter is recommended, since `websocket` itself doesn't provide a secure enough context to communicate sensitive data.

### websocket.connect([host], [port], [path], [callback])
Connects to a `websocket` server, host names can be prefixed with `ws://` or `wss://`.
- `host` {string} Optional. Defaults to `localhost`.
- `port` {number} Optional. Defaults to `80` if the `host` begins with `ws://` or `443` with `wss://`.
- `path` {Buffer | string} Optional. Defaults to `/`. If given, the client connects to that `endpoint`.
- `callback` {function} Optional. The function which will be executed when the client successfully connected to the server.

Emits an `open` event when the connection is established.

**Example**
```js
var ws = require('websocket');

var my_websocket = new ws.Websocket();

my_websocket.connect('wss://127.0.0.1', 443, '/my_endpoint', function() {
  my_websocket.close('Successful connection', 1000);
});

```

### websocket.close([message], [code], [callback])
Closes the `websocket` connection with the server. You can specify a close `message` and a close `code` as well. More info on them can be read here: https://tools.ietf.org/html/rfc6455#section-7.4.1
- `message` {Buffer | string} Optional. This `message` is sent to the server as a close message, mostly for explaining the status `code`.
- `code` {number} Optional. The `code` indicates the reason why the `websocket` connection was closed. Defaults to 1000.
- `callback` {function} Optional. The function will be executed when the `websocket` connection is closed.

Emits a `close` event when the connection is closed.

### websocket.ping([message], [mask], [callback])
Sends a `ping` to the server. If there is no response in the next 30 seconds, the connection is closed.
- `message` {Buffer | string} Optional. The `message` is used to identify the `ping` frame.
- `mask` {boolean} Optional. Defaults to `false`. Sets to mask the `message` or not.
- `callback` {function} Optional. The function to be executed when the server sends a response to the `ping`.

**Example**
```js
my_websocket.ping('Ping frame 1', true, function(msg) {
  console.log('Pong frame successfully received for frame ' + msg);
});

```

### websocket.send([message], [options], [callback])
Sends data to the server. It can be either `binary` or `utf8` data.
- `message` {Buffer | string} The `message` to be sent to the server.
- `options` {Object}
  - `mask` {boolean} Optional. Defaults to `false`. If set, the `message` is masked.
  - `binary` {boolean} Optional. Defaults to `false`. If set, the `message` is expected to be binary data.
- `callback` {function} Optional. The function to be executed when the `frame` is successfully sent.

**Example**
```js
my_websocket.send('My first WebSocket message!', {mask: true, binary: false}, function() {
  console.log('The data was successfully written to the socket!');
});
```

## Events

### `close`
Having closed the `websocket` connection, with the server, a `close` is emitted.

### `error`
If an `error` occurs, the `error` event is emitted, with the corresponding error message.

### `message`
The `message` event is emitted when the client receives data from the server.

### `open`
Emitted when the client established a `websocket` connection with the server.
