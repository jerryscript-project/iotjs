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

## Class: Websocket.Server
Create a new Websocket server instance.

### Websocket.Server(options[, callback])
Create a new server instance. One of `port` or `server` must be provided or an error is thrown. An HTTP server is automatically created, started, and used if `port` is set. If `secure` is set TLS server is automatically created, started and used. The `tls` module is required or an error is thrown. To use an external HTTP/S server instead, specify only `server`. In this case the HTTP/S server must be started manually.

- `options` {Object}
  - `port` {Number}
  - `host` {String} Optional. Defaults to `localhost`.
  - `server` {Object} Optional.
  - `path` {String} Optional. Defaults to `/`.
  - `secure` {Boolean} Optional.
  - `key` {String} Optional. (Required on `secure` server)
  - `cert` {String} Optional. (Required on `secure` server)
- `callback` {Function} Optional. The function which will be executed when the client successfully connected to the server.

Emits a `connection` event when the connection is established.

**Example**
```js
var websocket = require('websocket');

var options = {
  port: 9999
}

var server = new websocket.Server(options, Listener);

function Listener(ws) {
  console.log('Client connected: handshake done!');
  ws.on('message', function (msg) {
    console.log('Message received: %s', msg.toString());
    ws.send(msg.toString(), {mask: true, binary: false}); //echo
    server.close();
  });
  ws.on('ping', function (msg) {
    console.log('Ping received: %s', msg.toString());
  });
  ws.on('close', function (msg) {
    console.log('Client close :\n'
      'Reason: ' + msg.reason + ' Code: ' + msg.code);
  });
  ws.on('error', function (msg) {
    console.log('Error: %s', msg.toString());
  });

server.broadcast('Message to all clients', {mask: false, binary: false});
};

server.on('error', function (msg) {
  console.log('Error: %s', msg.toString());
});

server.on('close', function (msg) {
  console.log('Server close: \nReason: ' +
              msg.reason + ' Code: ' + msg.code);
});
```

**Example using http server**
```js
var websocket = require('websocket');
var http = require('http');

var httpserver = http.createServer().listen(9999);

options = {
  server: httpserver
};

var wss3 = new websocket.Server(options, Listener);

function Listener(ws) {
  console.log('Client connected: handshake done!');
};
```

### server.address()

Returns an object with `port`, `family`, and `address` properties specifying
the bound address, the family name, and port of the server.

**Example**
```js
var websocket = require('websocket');

var options = {
  port: 9999
}

var server = new websocket.Server(options, function(ws) {
});

console.log(server.address());
```

### server.close([reason], [code])
You can specify a close message and a close code as well. More info on them can be read here: [https://tools.ietf.org/html/rfc6455#section-7.4.1](https://tools.ietf.org/html/rfc6455#section-7.4.1 "The WebSocket Protocol Status Codes")

- `reason` {String} Optional. Defaults to `Connection successfully closed`.
- `code` {Number} Optional. Defaults to `1000`.

Close the Websocket server, terminate all clients and emit the `close` event.

**Example**
```js
var websocket = require('websocket');

var options = {
  port: 9999
}

var server = new websocket.Server(options, Listener);

function Listener(ws) {
  console.log('Client connected: handshake done!');
  server.close('Connection successfully closed', 1000);
};
```

### server.broadcast(message [, options])
You can specify a message that will be sent to every clients.
The `mask` will specify whether the data frame should be masked or not.
The `binary` will specify that if the data frame mode should be text or binary, default to text.
More info on them can be read here: [https://tools.ietf.org/html/rfc6455#section-5.6](https://tools.ietf.org/html/rfc6455#section-5.6 "The WebSocket Protocol Data Frames")

- `message` {String}
- `options` {Object} Optional.
  - `mask` {Boolean} Optional. Defaults to `true`.
  - `binary` {Boolean} Optional. Defaults to `false`.

Send message to all clients.

**Example**
```js
var websocket = require('websocket');

var options = {
  port: 9999
}

var server = new websocket.Server(options, Listener);

function Listener(ws) {
  console.log('Client connected: handshake done!');
};

server.broadcast('Message to receive all client',
                 {mask: true, binary: false});
```

### Event: 'connection'

- `socket` {Websocket}

Emitted when the handshake is complete.

### Event: 'close'

- `message` {Object}
  - `reason` {String}
  - `code` {Number}

Emitted when the server close.

### Event: 'error'

- `error` {Error}

Emmitted when an error occurs on the server.

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
