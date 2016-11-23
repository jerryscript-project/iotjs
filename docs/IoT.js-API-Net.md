## Module: net

### Platform Support

The following shows net module APIs available for each platform.

|  | Linux<br/>(Ubuntu) | Raspbian<br/>(Raspberry Pi) | Nuttx<br/>(STM32F4-Discovery) |
| :---: | :---: | :---: | :---: |
| net.createServer | O | O | O |
| net.connect | O | O | O |
| net.createConnection | O | O | O |
| net.Server.listen | O | O | O |
| net.Server.close | O | O | O |
| net.Socket.connect | O | O | O |
| net.Socket.write | O | O | O |
| net.Socket.end | O | O | O |
| net.Socket.destroy | O | O | O |
| net.Socket.pause | O | O | O |
| net.Socket.resume | O | O | O |
| net.Socket.setTimeout | O | O | O |
| net.Socket.setKeepAlive | X | X | X |

※ When writable stream is finished but readable stream is still alive, IoT.js tries to shutdown the socket, not destroy.
However on `nuttx` due to lack of implementation, it does nothing inside.

IoT.js provides asynchronous networking through Net module.

You can use this module with `require('net')` and create both servers and clients.

### Module Functions

#### net.connect(options[, connectListener])
#### net.connect(port[, host][, connectListener])
#### net.createConnection(options[, connectListener])
#### net.createConnection(port[, host][, connectListener])
* `options: Object`
* `port: Number`
* `host: String`, Default: `localhost`
* `connectListener: Function()`

Creates a `net.Socket` and connects to the supplied host.

It is equivalent to `new net.Socket()` followed by `socket.connect()`.


#### net.createServer([options][, connectionListener])
* `options: Object`
* `connectionListener: Function(connection: net.Socket)`

Creates a TCP server according to `options`.

`connectionListener` is automatically registered as `connection` event listener.


***

## class: net.Server

You can create `net.Server` instance with `net.createServer()`.


### Events

#### `'close'`
* `callback: Function()`

Emitted when server closed.

Note that this event will be emitted after all existing connections are closed.


#### `'connection(socket)'`
* `callback: Function(socket)`
 * `socket: net.Socket`

Emitted when new connection is established.


#### `'error'`
* `callback: Function()`

Emitted when an error occurs.


#### `'listening'`
* `callback: Function()`

Emitted when server has been started listening.


### Methods

#### server.close([closeListener])
* `closeListener: Function()`

Stops listening new arriving connection.

Server socket will finally close when all existing connections are closed, then emit 'close' event.

`closeListener` is registered as `close` event listener.


#### server.listen(port[, host][, backlog][, listenListener])
#### server.listen(options[, listenListener])
* `port: Number`
* `host: String`
* `backlog: Number`
* `listenListener: Function()`

Starts listening and accepting connections on specified port and host.


***

## class: net.Socket

net.Socket inherits [`Stream.Duplex`](IoT.js-API-Stream.md)

### Constructor

#### new net.Socket([options])
* `options: Object`

Creates a new socket object.

`options` is an object specifying following information:

* `allowHalfOpen: Boolean`



### Events


#### `'connect'`
* `callback: Function()`

Emitted after connection is established.


#### `'close'`
* `callback: Function()`

Emitted when the socket closed.


#### `'data'`
* `callback: Function(data)`
 * `data: Buffer | String`

Emitted when data is received from the connection.


#### `'drain'`
* `callback: Function()`

Emitted when the write buffer becomes empty.


#### `'end'`
* `callback: Function()`

Emitted when FIN packet received.


#### `'error'`
* `callback: Function()`

Emitted when an error occurs.


#### `'lookup'`
* `callback: Function(err, address, family)`
 * `err: Error | Null`
 * `address: String`
 * `family: String | Null`

Emitted after resolving hostname.


#### `'timeout'`
* `callback: Function()`

Emitted when the connection remains idle for specified timeout.


### Methods

#### socket.connect(options[, connectListener])
#### socket.connect(port[, host][, connectListener])
* `options: Object`
* `port: Number`
* 'host: String`, Default: `'localhost'`

Opens the connection with supplied port and host.

`options` is an object specifying following information:
* `port: Number` - port connect to (required)
* `host: String` - host connect to (optional, default: `'127.0.0.1'`)

`connectionListener` is automatically registered as `connect` event listener which will be emitted when the connection is established.


#### socket.destroy()

Destroys the socket.


#### socket.end([data][, callback])

* `data: String | Buffer`
* `callback: Function()`

Half-closes the socket.

If `data` is given it is equivalent to `socket.write(data)` followed by `socket.end()`.

* `data: String | Buffer`


#### socket.pause()

Pauses reading data.


#### socket.resume()

Resumes reading data after a call to `pause()`.


#### socket.setKeepAlive([enable][, initialDelay])

* `enable: Boolean`
* `initialDelay: Number`, Default: `0`

Enables or disables keep-alive functionality.


#### socket.setTimeout(timeout[, callback])
* `timeout: Number`
* `callback: Function()`

Sets timeout for the socket.

If the socket is inactive for `timeout` milliseconds, `'timeout'` event will emit.

`callback` is registered as `timeout` event listener.


#### socket.write(data[, callback])
* `data: String | Buffer`
* `callback: Function()`

Sends `data` on the socket.

`callback` function will be called after given data is flushed through the connection.
