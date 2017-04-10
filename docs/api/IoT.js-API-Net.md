<a id="markdown-platform-support" name="platform-support"></a>
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

### Contents

- [Net](#net)
    - [Module Functions](#module-functions)
        - [`net.connect(options[, connectListener])`](#netconnectoptions-connectlistener)
        - [`net.connect(port[, host][, connectListener])`](#netconnectport-host-connectlistener)
        - [`net.createConnection(options[, connectListener])`](#netcreateconnectionoptions-connectlistener)
        - [`net.createConnection(port[, host][, connectListener])`](#netcreateconnectionport-host-connectlistener)
        - [net.createServer([options][, connectionListener])](#netcreateserveroptions-connectionlistener)
- [Class: net.Server](#class-netserver)
    - [Prototype Funtions](#prototype-funtions)
        - [`server.close([closeListener])`](#serverclosecloselistener)
        - [`server.listen(port[, host][, backlog][, listenListener])`](#serverlistenport-host-backlog-listenlistener)
        - [`server.listen(options[, listenListener])`](#serverlistenoptions-listenlistener)
    - [Events](#events)
        - [`'close'`](#close)
        - [`'connection(socket)'`](#connectionsocket)
        - [`'error'`](#error)
        - [`'listening'`](#listening)
- [Class: net.Socket](#class-netsocket)
    - [Constructor](#constructor)
        - [new net.Socket([options])](#new-netsocketoptions)
    - [Prototype Functions](#prototype-functions)
        - [`socket.connect(options[, connectListener])`](#socketconnectoptions-connectlistener)
        - [`socket.connect(port[, host][, connectListener])`](#socketconnectport-host-connectlistener)
        - [`socket.destroy()`](#socketdestroy)
        - [`socket.end([data][, callback])`](#socketenddata-callback)
        - [`socket.pause()`](#socketpause)
        - [`socket.resume()`](#socketresume)
        - [`socket.setKeepAlive([enable][, initialDelay])`](#socketsetkeepaliveenable-initialdelay)
        - [`socket.setTimeout(timeout[, callback])`](#socketsettimeouttimeout-callback)
        - [`socket.write(data[, callback])`](#socketwritedata-callback)
    - [Events](#events-1)
        - [`'connect'`](#connect)
        - [`'close'`](#close-1)
        - [`'data'`](#data)
        - [`'drain'`](#drain)
        - [`'end'`](#end)
        - [`'error'`](#error-1)
        - [`'lookup'`](#lookup)
        - [`'timeout'`](#timeout)

<a id="markdown-net" name="net"></a>
# Net

IoT.js provides asynchronous networking through Net module. You can use this module with `require('net')` and create both servers and clients.

<a id="markdown-module-functions" name="module-functions"></a>
## Module Functions

<a id="markdown-netconnectoptions-connectlistener" name="netconnectoptions-connectlistener"></a>
### `net.connect(options[, connectListener])`
<a id="markdown-netconnectport-host-connectlistener" name="netconnectport-host-connectlistener"></a>
### `net.connect(port[, host][, connectListener])`
<a id="markdown-netcreateconnectionoptions-connectlistener" name="netcreateconnectionoptions-connectlistener"></a>
### `net.createConnection(options[, connectListener])`
<a id="markdown-netcreateconnectionport-host-connectlistener" name="netcreateconnectionport-host-connectlistener"></a>
### `net.createConnection(port[, host][, connectListener])`
* `options <Object>`
* `port <Number>`
* `host <String>` Default: 'localhost'
* `connectListener <Function()>`

Creates a `net.Socket` and connects to the supplied host.

It is equivalent to `new net.Socket()` followed by `socket.connect()`.

<a id="markdown-netcreateserveroptions-connectionlistener" name="netcreateserveroptions-connectionlistener"></a>
### `net.createServer([options][, connectionListener])`
* `options <Object>`
* `connectionListener <Function(connection: net.Socket)>`

Creates a TCP server according to `options`.

`connectionListener` is automatically registered as `connection` event listener.


<a id="markdown-class-netserver" name="class-netserver"></a>
# Class: net.Server

This class is used to create a TCP or local server. You can create `net.Server` instance with `net.createServer()`.

<a id="markdown-prototype-funtions" name="prototype-funtions"></a>
## Prototype Funtions

<a id="markdown-serverclosecloselistener" name="serverclosecloselistener"></a>
### `server.close([closeListener])`
* `closeListener <Function()>`

Stops listening new arriving connection.

Server socket will finally close when all existing connections are closed, then emit 'close' event.

`closeListener` is registered as `close` event listener.

<a id="markdown-serverlistenport-host-backlog-listenlistener" name="serverlistenport-host-backlog-listenlistener"></a>
### `server.listen(port[, host][, backlog][, listenListener])`
<a id="markdown-serverlistenoptions-listenlistener" name="serverlistenoptions-listenlistener"></a>
### `server.listen(options[, listenListener])`
* `port <Number>`
* `host <String>`
* `backlog <Number>`
* `listenListener <Function()>`

Starts listening and accepting connections on specified port and host.

<a id="markdown-events" name="events"></a>
## Events

<a id="markdown-close" name="close"></a>
### `'close'`
* `callback <Function()>`

Emitted when server closed.

Note that this event will be emitted after all existing connections are closed.


<a id="markdown-connectionsocket" name="connectionsocket"></a>
### `'connection(socket)'`
* `callback <Function(socket)>`
* `socket <net.Socket>`

Emitted when new connection is established.

<a id="markdown-error" name="error"></a>
### `'error'`
* `callback <Function()>`

Emitted when an error occurs.


<a id="markdown-listening" name="listening"></a>
### `'listening'`
* `callback <Function()>`

Emitted when server has been started listening.





<a id="markdown-class-netsocket" name="class-netsocket"></a>
# Class: net.Socket

This object is an abstraction of a TCP or local socket. net.Socket inherits [`Stream.Duplex`](IoT.js-API-Stream.md). They can be created by the user and used as a client (with connect()) or they can be created by IoT.js and passed to the user through the 'connection' event of a server.

<a id="markdown-constructor" name="constructor"></a>
## Constructor
<a id="markdown-new-netsocketoptions" name="new-netsocketoptions"></a>
### `new net.Socket([options])`
* `options <Object>`

Creates a new socket object.

`options` is an object specifying following information:
* `allowHalfOpen <Boolean>`


<a id="markdown-prototype-functions" name="prototype-functions"></a>
## Prototype Functions

<a id="markdown-socketconnectoptions-connectlistener" name="socketconnectoptions-connectlistener"></a>
### `socket.connect(options[, connectListener])`
<a id="markdown-socketconnectport-host-connectlistener" name="socketconnectport-host-connectlistener"></a>
### `socket.connect(port[, host][, connectListener])`
* `options <Object>`
* `port <Number>`
* `host <String>` Default: 'localhost'

Opens the connection with supplied port and host.

`options` is an object specifying following information:
* `port <Number>` - port connect to (required)
* `host <String>` - host connect to (optional, default: `'127.0.0.1'`)

`connectionListener` is automatically registered as `connect` event listener which will be emitted when the connection is established.

<a id="markdown-socketdestroy" name="socketdestroy"></a>
### `socket.destroy()`

Destroys the socket.


<a id="markdown-socketenddata-callback" name="socketenddata-callback"></a>
### `socket.end([data][, callback])`

* `data <String> | <Buffer>`
* `callback <Function()>`

Half-closes the socket.

If `data` is given it is equivalent to `socket.write(data)` followed by `socket.end()`.

* `data <String> | <Buffer>`


<a id="markdown-socketpause" name="socketpause"></a>
### `socket.pause()`

Pauses reading data.


<a id="markdown-socketresume" name="socketresume"></a>
### `socket.resume()`

Resumes reading data after a call to `pause()`.


<a id="markdown-socketsetkeepaliveenable-initialdelay" name="socketsetkeepaliveenable-initialdelay"></a>
### `socket.setKeepAlive([enable][, initialDelay])`

* `enable <Boolean>`
* `initialDelay <Number>` Default: '0'

Enables or disables keep-alive functionality.


<a id="markdown-socketsettimeouttimeout-callback" name="socketsettimeouttimeout-callback"></a>
### `socket.setTimeout(timeout[, callback])`
* `timeout <Number>`
* `callback <Function()>`

Sets timeout for the socket.

If the socket is inactive for `timeout` milliseconds, `'timeout'` event will emit.

`callback` is registered as `timeout` event listener.


<a id="markdown-socketwritedata-callback" name="socketwritedata-callback"></a>
### `socket.write(data[, callback])`
* `data <String> | <Buffer>`
* `callback <Function()>`

Sends `data` on the socket.

`callback` function will be called after given data is flushed through the connection.


<a id="markdown-events-1" name="events-1"></a>
## Events


<a id="markdown-connect" name="connect"></a>
### `'connect'`
* `callback <Function()>`

Emitted after connection is established.


<a id="markdown-close-1" name="close-1"></a>
### `'close'`
* `callback <Function()>`

Emitted when the socket closed.


<a id="markdown-data" name="data"></a>
### `'data'`
* `callback <Function(data)>`
* `data <Buffer> | <String>`

Emitted when data is received from the connection.


<a id="markdown-drain" name="drain"></a>
### `'drain'`
* `callback <Function()>`

Emitted when the write buffer becomes empty.


<a id="markdown-end" name="end"></a>
### `'end'`
* `callback <Function()>`

Emitted when FIN packet received.


<a id="markdown-error-1" name="error-1"></a>
### `'error'`
* `callback <Function()>`

Emitted when an error occurs.


<a id="markdown-lookup" name="lookup"></a>
### `'lookup'`
* `callback <Function(err, address, family)>`
  * `err <Error> | Null`
  * `address <String>`
  * `family <String> | Null`

Emitted after resolving hostname.


<a id="markdown-timeout" name="timeout"></a>
### `'timeout'`
* `callback <Function()>`

Emitted when the connection remains idle for specified timeout.
