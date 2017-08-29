### Platform Support

The following shows net module APIs available for each platform.

|  | Linux<br/>(Ubuntu) | Raspbian<br/>(Raspberry Pi) | NuttX<br/>(STM32F4-Discovery) | TizenRT<br/>(Artik053) |
| :---: | :---: | :---: | :---: | :---: |
| net.createServer | O | O | △ ¹ | - |
| net.connect | O | O | △ ¹ | - |
| net.createConnection | O | O | △ ¹ | - |
| net.Server.listen | O | O | △ ¹ | - |
| net.Server.close | O | O | △ ²| - |
| net.Socket.connect | O | O | △ ¹ | - |
| net.Socket.write | O | O | △ ¹ | - |
| net.Socket.end | O | O | △ ¹ ³ | - |
| net.Socket.destroy | O | O | △ ¹ ³ | - |
| net.Socket.pause | O | O | △ ¹ | - |
| net.Socket.resume | O | O | △ ¹ | - |
| net.Socket.setTimeout | O | O | △ ¹ | - |
| net.Socket.setKeepAlive | X | X | X | - |

1. On NuttX/STM32F4-Discovery, even a couple of sockets/server/requests might not work properly.

2. On NuttX/STM32F4-Discovery, close() may block due to a bug in poll().

3. When writable stream is finished but readable stream is still alive, IoT.js tries to shutdown the socket, not destroy.
However on `NuttX` due to lack of implementation, it does nothing inside.


# Net

IoT.js provides asynchronous networking through Net module. You can use this module with `require('net')` and create both servers and clients.

### net.connect(options[, connectListener])
* `options` {Object} An object which specifies the connection options.
* `connectListener` {Function} Listener for the `'connect'` event.
* Returns {net.Socket}.

Creates a new `net.Socket` and automatically connects with the supplied `options`.
The `options` object specifies the following information:
* `port` {number} Port connect to (required).
* `host` {string} Host connect to (optional, **Default:** `localhost`).
* `family` {number} Version of IP stack.

The `options` are passed to both the `net.Socket` constructor and the `socket.connect` method.
The `connectListener` is automatically registered as a `'connect'` event listener.

**Example**

```js

var net = require('net');

var port = 22702;

var echo_msg = '';

var socket = net.connect({port: port, family: 4}, function() {
  socket.end('Hello IoT.js');
});

socket.on('data', function(data) {
  echo_msg += data;
});

socket.on('end', function() {
  console.log(echo_msg);
});

```

### net.connect(port[, host][, connectListener])
* `port` {number} Port the client should connect to.
* `host` {string} Host the client should connect to. **Default:** `localhost`.
* `connectListener` {Function} Listener for the `'connect'` event.
* Returns {net.Socket}.

Creates a new `net.Socket` and automatically connects to the supplied `port` and `host`.
If host is omitted, `localhost` will be assumed.
The `connectListener` is automatically registered as a `'connect'` event listener.

**Example**

```js

var net = require('net');

var port = 22702;
var host = '127.0.0.1';
var echo_msg = '';

var socket = net.connect(port, host, function() {
  socket.end('Hello IoT.js');
});

socket.on('data', function(data) {
  echo_msg += data;
});

socket.on('end', function() {
  console.log(echo_msg);
});

```

### net.createConnection(options[, connectListener])
* `options` {Object} An object which specifies the connection options.
* `connectListener` {Function} Listener for the `'connect'` event.
* Returns {net.Socket}.

Creates a new `net.Socket` and automatically connects with the supplied `options`.
The `options` are passed to both the `net.Socket` constructor and the `socket.connect` method.
The `options` object specifies the following information:
* `port` {number} Port connect to (required).
* `host` {string} Host connect to (optional, **Default:** `localhost`).
* `family` {number} Version of IP stack.

The `connectionListener` is automatically registered as a `'connect'` event listener.

**Example**

```js

var net = require('net');

var port = 80;

var echo_msg = '';

var socket = net.createConnection({port: port, family: 4}, function() {
  socket.end('Hello IoT.js');
});

socket.on('data', function(data) {
  echo_msg += data;
});

socket.on('end', function() {
  console.log(echo_msg);
});

```

### net.createConnection(port[, host][, connectListener])
* `port` {number} Port the client should connect to.
* `host` {string} Host the client should connect to. **Default:** `localhost`.
* `connectListener` {Function} Listener for the `'connect'` event.
* Returns {net.Socket}.


Creates a new `net.Socket` and automatically connects to the supplied `port` and `host`.
It is equivalent to `new net.Socket()` followed by `socket.connect()`.
If host is omitted, `localhost` will be assumed.
The `connectionListener` is automatically registered as a `'connect'` event listener.

**Example**

```js

var net = require('net');

var port = 22702;
var host = '127.0.0.1';
var echo_msg = '';

var socket = net.createConnection(port, host, function() {
  socket.end('Hello IoT.js');
});

socket.on('data', function(data) {
  echo_msg += data;
});

socket.on('end', function() {
  console.log(echo_msg);
});

```

### net.createServer([options][, connectionListener])
* `options` {Object} An object which specifies the connection options **Default:** `{ allowHalfOpen: false }`.
* `connectListener` {Function} Listener for the `'connection'` event.
* Returns {net.Server}.

Creates a TCP server according to `options`.
The `connectionListener` is automatically registered as a `'connection'` event listener.
If `allowHalfOpen` is true, then the socket becomes non-readable, but still writable. You should call the `socket.end()` method explicitly.

**Example**

```js

var net = require('net');

var port = 22702;

var server = net.createServer(
  {
    allowHalfOpen: true
  },
  function(socket) {
    server.close();
  }
);

server.listen(port);

server.on('connection', function(socket) {
  var msg = '';
  socket.on('data', function(data) {
    msg += data;
  });
  socket.on('end', function() {
    socket.end(msg);
  });
});

```

## Class: net.Server

This class is used to create a TCP or local server. You can create `net.Server` instance with `net.createServer()`.

### server.close([closeListener])
* `closeListener` {Function} Listener for the `'close'` event.

Stops listening new arriving connection.
Server socket will be finally closed when all existing connections are closed, then emits `'close'` event.
The `closeListener` is registered as a `'close'` event listener.

**Example**

```js

var net = require('net');

var server = net.createServer();
var port = 22704;
var timeout = 1000;

server.listen(port);

/* ... */

server.close();

```

### server.listen(port[, host][, backlog][, listenListener])
* `port` {number} Port the client should connect to.
* `host` {string} Host the client should connect to.
* `backlog` {number} The maximum length of the queue of pending connections. **Default:** `511`.
* `listenListener` {Function} Listener for the `'listening'` event.
* Returns: {Object} The self instance of `net.Server`.

Begin accepting connections on the specified port and hostname.
If the hostname is omitted, the server will accept connections on any IPv4 address (0.0.0.0).

**Example**

```js

var net = require('net');

var server = net.createServer();
var port = 22709;

server.listen(port);

```


### server.listen(options[, listenListener])
* options {Object} An object which specifies the connection options.
* `listenListener` {Function} Listener for the `'listening'` event.

It behaves as the `server.listen(port[, host][, backlog][, listenListener])` function above.

The option object supports the following properties:
* `port` {number} Port the client should connect to.
* `host` {string} Host the client should connect to.
* `backlog` {number} The maximum length of the queue of pending connections. **Default:** `511`.

**Example**

```js

var net = require('net');

var server = net.createServer();

server.listen({port: 80, host: 'localhost'});

```

### Event: 'close'
* `callback` {Function}

Emitted when server has closed the connection.

Note that this event will be emitted after all existing connections are closed.

**Example**

```js

var net = require('net');

var serverCloseEvent = 0;
var server = net.createServer();
var port = 80;

server.listen(port);

server.on('connection', function(socket) {
  server.on('close', function() {
    serverCloseEvent++;
  });
});

```

### Event: 'connection(socket)'
* `callback` {Function}
  * `socket` {Net.Socket}

Emitted when new connection is established.

**Example**

```js

var net = require('net');

var server = net.createServer();
var port = 80;

server.listen(port);

server.on('connection', function(socket) {
  var msg = '';
  socket.on('data', function(data) {
    msg += data;
  });

});

```

### Event: 'error'
* `callback` {Function}

Emitted when an error occurs.

**Example**

```js

var assert = require('assert');
var net = require('net');

var port = 80;
var msg = 'Hello IoT.js';
var server = net.createServer();

/* ... */

server.on('error', function() {
  assert.fail();
  server.close();
});

```

### Event: 'listening'
* `callback` {Function}

Emitted when server has been started listening.

**Example**

```js

var net = require('net');

var port = 80;

var server = net.createServer(
  {
    allowHalfOpen: true
  },
  function(socket) {
    server.close();
  }
);

server.listen(port);

server.on('listening', function() {
    console.log('started listening');
});

```

## Class: net.Socket

This object is an abstraction of a TCP or a local socket. `net.Socket` inherits from [`Stream.Duplex`](IoT.js-API-Stream.md). They can be created by the user (used as a client with connect()) or by the IoT.js engine (passed to the user through the 'connection' event of a server).

### new net.Socket([options])
* `options` {Object} An optional object which specifies the socket information.
* Returns {net.Socket}.

Construct a new socket object.
The `options` object specifies only the following information: `allowHalfOpen` {boolean}.

**Example**

```js

var net = require('net');

var socket = new net.Socket();

```

### socket.connect(options[, connectListener])
* `options` {Object} An object which specifies the connection information.
* `connectListener` {Function} Listener for the `'connect'` event.
* Returns {net.Socket}.

Creates a new socket object and automatically connects with the supplied `options`.

The `options` object specifies following information:
* `port` {number} Port connect to (required).
* `host` {string}  Host connect to (optional, **Default:** `localhost`).
* `family` {number} Version of IP stack.

The `connectionListener` is automatically registered as a `'connect'` event listener which will be emitted when the connection is established.

**Example**

```js

var net = require('net');

var port = 22702;

var socket = new net.Socket();

socket.connect({port: port, family: 4}, function() {
  socket.end('Hello IoT.js');
});

```

### socket.connect(port[, host][, connectListener])
* `port` {number} Port the client should connect to.
* `host` {string} Host the client should connect to. **Default:** `localhost`.
* `connectListener` {Function} Listener for the `'connect'` event.
* Returns {net.Socket}.

Creates a new socket and automatically connects with the supplied `port` and `host`.

`connectionListener` is automatically registered as a `'connect'` event listener which will be emitted when the connection is established.

**Example**

```js

var net = require('net');

var port = 80;

var socket = new net.Socket();

socket.connect(port, '127.0.0.1', function() {
  socket.end('Hello IoT.js');
});

```

### socket.destroy()

Ensures that no more I/O activity happens on the socket and destroys the socket as soon as possible.

**Example**
```js

var net = require('net');

var port = 80;

var socket = new net.Socket();

socket.connect(port, '127.0.0.1', function() {
  socket.end('Hello IoT.js');
});

/* ... */

socket.destroy();

```

### socket.end([data][, callback])

* `data` {Buffer|string}
* `callback` {Function}

Half-closes the socket. The socket is no longer writable.
If `data` is given it is equivalent to `socket.write(data)` followed by `socket.end()`.

**Example**
```js

var net = require('net');

var server = net.createServer();
var port = 4010;

server.listen(port);

server.on('connection', function(socket) {
  socket.on('data', function(data) {
    socket.end('Hello IoT.js');
  });
});

```

### socket.pause()

Pauses reading data.

**Example**
```js

var net = require('net');

var server = net.createServer();
var port = 4010;

server.listen(port);

server.on('connection', function(socket) {

  // as soon as connection established, pause the socket
  socket.pause();
});

```

### socket.resume()

Resumes reading data after a call to `pause()`.

**Example**
```js

var net = require('net');
var timers = require('timers');

var server = net.createServer();
var port = 80;

var socket = new net.Socket();
var msg = "";

/* ... */

server.listen(port);

server.on('connection', function(socket) {
  socket.on('data', function(data) {
    msg += data;
    socket.end();
  });
  socket.on('close', function() {
    server.close();
  });

  // as soon as connection established, pause the socket
  socket.pause();

  // resume after 2 secs
  timers.setTimeout(function() {
    socket.resume();
  }, 2000);
});

```

### socket.setKeepAlive([enable][, initialDelay])

* `enable` {boolean} **Default:** `false`.
* `initialDelay {number} **Default:** `0`.

Enables or disables keep-alive functionality.

**Example**
```js

var net = require('net');

var keepalive_sock = new net.Socket();

keepalive_sock.setKeepAlive(true, 1000);

```

### socket.setTimeout(timeout[, callback])
* `timeout` {number} Timeout number.
* `callback` {Function} Registered as a `'timeout'` event listener.

Sets timeout for the socket.

If the socket is inactive for `timeout` milliseconds, `'timeout'` event will emit.

**Example**
```js

var net = require('net');

var server = net.createServer();
var port = 40;
var timeout = 2000;
var msg = '';

server.listen(port);

server.on('connection', function(socket) {
  socket.setTimeout(timeout, function() {
    socket.end();
  });
});

```

### socket.write(data[, callback])
* `data` {Buffer|string} Data to write.
* `callback` {Function} Executed function (when the data is finally written out).

Sends `data` on the socket.

The optional `callback` function will be called after the given data is flushed through the connection.

**Example**
```js

var net = require('net');
var timers = require('timers');

var writeat = 1000;

var socket = new net.Socket();

/* ... */

socket.on('connect', function() {
  timers.setTimeout(function() {
    socket.write('Hello IoT.js');
  }, writeat);
});

```

### Event: 'connect'
* `callback` {Function}

Emitted after connection is established.

**Example**
```js

var net = require('net');

var port = 80;

var count = 40;

/* ... */

for (var i = 0; i < count; ++i) {
  (function(i) {
    var socket = new net.Socket();

    socket.connect(port, "localhost");
    socket.on('connect', function() {
      socket.end(i.toString());
    });

    /* ... */

  })(i);
}

```

### Event: 'close'
* `callback` {Function}

Emitted when the socket has been closed.

**Example**
```js

var net = require('net');

var server = net.createServer();
var port = 80;

server.listen(port);

server.on('connection', function(socket) {

  /* ... */

  socket.on('close', function() {
    server.close();
  });
});

```

### Event: 'data'
* `callback` {Function}

The data is given an argument (data: Buffer|string).

Emitted when data is received from the connection.

**Example**
```js

var net = require('net');

var msg = "";

var socket = new net.Socket();

/* ... */

socket.on('data', function(data) {
  msg += data;
});

```

### Event: 'drain'
* `callback` {Function}

Emitted when the write buffer becomes empty.

**Example**
```js

var net = require('net');

var port = 22703;
var limit = 200;
var server = net.createServer();

server.listen({ port: port });

/* ... */

server.on('connection', function(socket) {
  var i = 0;
  var writing = function() {
    var ok;
    do {
      ok = socket.write("" + (i % 10));
      if (++i == limit) {
        socket.end();
        ok = false;
      }
    } while (ok);
  };
  socket.on('drain', writing);
  writing();
});

```

### Event: 'end'
* `callback` {Function}

Emitted when FIN packet received.

**Example**
```js

var net = require('net');

var socket = new net.Socket();

/* ... */

socket.on('end', function() {
  socket.end();
});

```

### Event: 'error'
* `callback` {Function}
  * `err` {Error}

Emitted when an error occurs.

**Example**
```js

var assert = require('assert');
var net = require('net');

var bad_sock = new net.Socket();
bad_sock.on('error', function(err){
  assert.equal(err instanceof Error, true);
});

```

### Event: 'lookup'
* `callback` {Function}
  * `err` {Error}
  * `address` {string}
  * `family` {string|null}

Emitted after resolving hostname.

**Example**
```js

var socket = new net.Socket();
var msg = "";
var lookupHandled = false;

socket.on('lookup', function(err, ip, family) {
  lookupHandled = true;
});

```

### Event: 'timeout'
* `callback` {Function}`

Emitted when the connection remains idle for the specified timeout.

**Example**
```js

var net = require('net');

var timedout = false;

// Try connect to host that is not exist (Reserved address)
var socket = net.createConnection(11111, '192.0.2.1');

socket.setTimeout(1000);

socket.on('timeout', function() {
  timedout = true;
  socket.destroy();
});

```
