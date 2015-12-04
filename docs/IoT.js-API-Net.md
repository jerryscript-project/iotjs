## Module: net

IoT.js provides asynchronous networking through Net module.

You can use this module with `require('net')` and create both servers and clients.

### Methods

#### net.createServer([options][, connectionListener])
* `options: Object`
* `connectionListener: Function(connection: net.Socket)`

Creates a TCP server according to `options`.

`connectionListener` is automatically registered as `connection` event listener.


#### net.connect(options[, connectListener])
#### net.connect(port[, host][, connectListener])
#### net.createConnection(options[, connectnListener])
#### net.createConnnection(port[, host][, connectListener])
* `options: Object`
* `port: Number`
* `host: String`, Default: `localhost`
* `connectListner: Function()`

Creates a `net.Socket` and connects to the supplied host.

It is equivalent to `new net.Socket()` followed by `socket.connect()`.

***

## class: net.Server

You can create `net.Server` instance with `net.createServer()`.

### Instance Methods

#### server.listen(port[, host][, backlog][, listenListener])
#### server.listen(options[, listenListener])

* `port: Number`
* `host: String`
* `backlog: Number`
* `listenListener: Function()`

Start listening and accepting connections on specified port and host.


#### server.close([closeListener])

* `closeListener: Function()`

Stop listening new arriving connection.

Server socket will finally close when all existing connections are closed, then emit 'close' event.

`closeListener` is registered as `close` event listener.

### Event

#### `'listening'`
Emittied when server has been started listening.

#### `'connection(socket)'`
* `socket: net.Socket`

Emitted when new connection is established.

#### `'close'`
Emitted when server closed.

Note that this event will be emitted after all existing connections are closed.

#### `'error'`
Emitted when an error occurs.

***

## class: net.Socket

### Constructor

#### new net.Socket([options])

* `options: Object`

Creates a new socket object.

`options` is an object specifying following information:

* `allowHalfOpen: Boolean`

### Instance Methods

#### socket.connect(options[, connectListener])
#### socket.connect(port[, host][, connectListener])
* `options: Object`
* `port: Number`
* 'host: String`, Default: `'localhost'`

Opens the connection with supplied port and host.

`options` is an object specifying following information:
* `port: Number` - port connect to (required)
* `host: String` - host connect to (optional, default: `'127.0.0.1'`)

`connectionListner` is automatically registered as `connect` event listener which will be emitted when the connection is established.

#### socket.write(data[, callback])

* `data: String | Buffer`
* `callback: Funciton()`

Sends `data` on the socket.

`callback` function will be called after given data is flushed through the connection.

#### socket.end([data][, callback])

* `data: String | Buffer`
* `callback: Funciton()`

Half-closes the socket.

If `data` is given it is equivalent to `socket.write(data)` followed by `socket.end()`.

* `data: String | Buffer`

#### socket.destroy()

Destroys the socket.

#### socket.pause()

Pauses reading data.

#### socket.resume()

Resumes reading data after a call to `pause()`.

#### socket.setTimeout(timeout[, callback])

* `timeout: Number`
* `callback: Function()`

Sets timeout for the socket.

If the socket is inactive for `timeout` milliseconds, `'timeout'` event will emit.

`callback` is registered as `timeout` event listener.

#### socket.setKeepAlive([enable][, initialDelay])

* `enable: Boolean`
* `initilalDelay: Number`, Default: `0`

Enables or disables keep-alive functionality.

### Events

#### `'lookup(err, address, family)'`
* `err: Error | Null`
* `address: String`
* `family: String | Null`

Emitted after resolving hostname.

#### `'connect'`

Emitted after connection is established.

#### `'data(data)'`
* data: Buffer | String

Emitted when data is received from the connection.

#### `'drain'`

Emitted when the write buffer becomes empty.

#### `'end'`

Emitted when FIN packet received.

#### `'timeout'`

Emitted when the connection remains idle for specified timeout.

#### `'close'`

Emitted when the socket closed.

#### `'error'`

Emitted when an error occurs.

