### Platform Support

The following chart shows the availability of each TLS module API function on each platform.

|  | Linux<br/>(Ubuntu) | Tizen<br/>(Raspberry Pi) | Raspbian<br/>(Raspberry Pi) | Nuttx<br/>(STM32F4-Discovery) | TizenRT<br/>(Artik053) |
| :---: | :---: | :---: | :---: | :---: | :---: |
| tls.connect             | O | O | O | O | O | O |
| tls.createServer        | O | O | O | O | O | O |
| tls.createSecureContext | O | O | O | O | O | O |
| tls.Server              | O | O | O | O | O | O |
| tls.TLSSocket           | O | O | O | O | O | O |
| tls.TLSSocket.write     | O | O | O | O | O | O |
| tls.TLSSocket.pause     | O | O | O | O | O | O |
| tls.TLSSocket.end       | O | O | O | O | O | O |
| tls.TLSSocket.resume    | O | O | O | O | O | O |
| tls.TLSSocket.pause     | O | O | O | O | O | O |

As even a couple of sockets/servers/requests require a considerable size of memory, on NuttX/STM32F4-Discovery and TizenRT/Artik053
the number of such sockets are limited.

# TLS

Transport Layer Security makes secure communication over sockets possible.

### tls.connect(options[,callback])
* `options` {Object}
    * `host` {string} Host the client should connect to, defaults to 'localhost'.
    * `port` {number} Port the client should connect to.
    * `socket` {stream.Duplex} Optional, typically an instance of `net.Socket`. If this options is specified, host and port are ignored. The user passing the options is responsible for it connecting to the server. `tls.connect` won't call `net.connect` on it.
    * `rejectUnauthorized` {boolean} Whether the server certificate should be verified against the list of supplied CAs. An `error` event is emitted if verifications fails; `err.code` contains the MbedTLS error code. Defaults to `false`. NOT READY
    * `servername` {string} Server name for the SNI (Server name Indication) TLS extension. NOT READY
    * `session` {Buffer} A `Buffer` containing a TLS session. NOT READY
    * `minDHSize` {number} The minimum size of the DH parameter in bits to accept a TLS connection. If a server offers a DH parameter with a size less than specified, the TLS connection is destroyed and an error is thrown. Defaults to `1024`.
    * `lookup` {Function} Custom lookup. Defaults to `dns.lookup()`.
* `callback` {Function} The callback function will be added as a listener for the `secureConnect` event.

Returns a `tls.TLSSocket` object.

**Example**

```js
var tls = require('tls');

var opts = {
    host: '127.0.0.1',
    port: 443,
    rejectUnauthorized: true
}

var socket = tls.connect(opts, function() {
    socket.write('Hello IoT.js');
    socket.end();
});
```

### tls.connect(port[,host][,options][,callback])
* `port` {number} Port the client should connect to.
* `host` {string} Host the client should connect to, defaults to 'localhost'.
* `options` {Object} See `tls.connect()`.
* `callback` {Function} See `tls.connect()`.

Same as tls.connect() except that port and host can be provided as arguments instead of options.
A port or host option, if specified, will take precedence over any port or host argument.

**Example**

```js
var tls = require('tls');

var socket = tls.connect(443, 'localhost', function() {
    socket.write('Hello IoT.js');
    socket.end();
});
```

### tls.createServer([options][, secureConnectionListener])
* `options` {object} Accepts the same options as the `tls.Server()` and `tls.createSecureContext()`.
* `secureConnectionListener` {Function}
  * `socket` {tls.TLSSocket} The connected TLSSocket.
* Returns {tls.Server}

Create a TLS Server. Behaves the same way as the `new tls.Server(options, secureConnectionListener)`
call.

**Example**

```js

var fs = require('fs');
var tls = require('tls');
var options = {
  key: fs.readFileSync('server.key'),
  cert: fs.readFileSync('server.crt')
};
var server = tls.createServer(options, function(socket) {
  console.log('got connection');
  ...
});

server.listen(8081);
```


### tls.createSecureContext([options])
* `options` {object}
  * `ca` {string | Buffer} Optional trusted CA certificates. No default is provided.
  * `cert` {string | Buffer} Cert chains in PEM format.
  * `key` {string | Buffer} Private keys in PEM format.
* Returns {Object}

The method returns a special object containing the tls context and credential information.

## Class: tls.Server

A server object representing a TLS server. Based on the `net.Server`.
All events, methods and properties are inherited from the `net.Server`.

### new tls.Server([options][, secureConnectionListener])

* `options` {object} Options for the TLS connection.
  * `secureContext` {object} An special object containing the tls credential information.
    This should be only created via a `tls.createSecureContext()` call if needed. If not provided
    a secureContext will be created automatically, using the `options` object. No default value is provided.
  * Additional options are from `tls.createSecureContext()`.
* `secureConnectionListener` {Function}
  * `socket` {tls.TLSSocket}
* Returns {tls.Server}

Creates new `tls.Server` object. The `secureConnectionListener` method is automatically set
as a listener for the `'secureConnection'` event.

To correctly create a TLS Server the server certificates should be provided in the `options`
object.

**Example**

```js
var fs = require('fs');
var tls = require('tls');
var options = {
  key: fs.readFileSync('server.key'),
  cert: fs.readFileSync('server.crt')
};
var server = new tls.Server(options, function(socket) {
  console.log('got connection');
  ...
});

server.listen(8081);
```

## Class: tls.TLSSocket
The `TLSSocket` is responsible for all TLS negotiations and data encryption on a `net.Socket`.

Just like `net.Socket` it uses a `Stream.duplex` interface.

### new tls.TLSSocket(socket[,options])
* `socket` {net.Socket | stream.Duplex}
* `options` {Object}
  * `isServer` {boolean} The TLSSocket must know if it represents the server or client side of the connection. Default: `false`.
  * `secureContext` {Object} The TLS context object. If none provided one will be created with the `tls.createSecureContext` method
    using the given `options` object.
* Returns {tls.TLSSocket}

Creates a new TLSSocket object for an existing TCP socket.

### tlsSocket.address()
Returns an object containing the bound address, family name, and port of the socket.`{port: 443, family: 'IPv4', address: '127.0.0.1'}`

### tlsSocket.authorizationError
Returns the reason why the peer's certificate has not been verified.

### tlsSocket.authorized
Returns `true` if the peer certificate was signed by one of the CAs specified when creating the `tls.TLSSocket` instance, otherwise false.

### tlsSocket.encrypted
Always returns `true`, can be used to distinguish TLS sockets from regular `net.Socket`s.

### tlsSocket.getProtocol()
Returns a string containing the negotiated SSL/TLS protocol version of the connection. If the handshaking has not been complete, `unknown` will be returned. The value `null` will be returned for server sockets or disconnected client sockets.

### tlsSocket.localAddress
Returns a string representing the local IP address.

### tlsSocket.localPort
Returns a number representing the local port.

### tlsSocket.remoteAddress
Returns a string representing the remote IP address.

### tlsSocket.remoteFamily
Returns a string representing the remote IP family.

### tlsSocket.remotePort
Returns a number representing the remote port.
