### Platform Support

The following chart shows the availability of each TLS module API function on each platform.

|  | Linux<br/>(Ubuntu) | Tizen<br/>(Raspberry Pi) | Raspbian<br/>(Raspberry Pi) | Nuttx<br/>(STM32F4-Discovery) | TizenRT<br/>(Artik053) |
| :---: | :---: | :---: | :---: | :---: | :---: |
| tls.connect  | X | O | O | O | O | O |
| tls.write  | X | O | O | O | O | O |
| tls.pause | X | O | O | O | O | O |
| tls.end | X | O | O | O | O | O |
| tls.resume | X | O | O | O | O | O |
| tls.pause | X | O | O | O | O | O |

# TLS

Transport Layer Security makes secure communication over sockets possible.

## Class: tls.TLSSocket
The `TLSSocket` is responsible for all TLS negotiations and data encryption on a `net.Socket`.

Just like `net.Socket` it uses a `Stream.duplex` interface.

### new tls.TLSSocket(socket[,options])
- `socket` {net.Socket | stream.Duplex}
- `options` {Object}
    - `session` {Buffer} Optional, `Buffer` instance containing a TLS session.

Note: `tls.connect()` must be used to create the socket.

### tls.connect(options[,callback])
- `options` {Object}
    - `host` {string} Host the client should connect to, defaults to 'localhost'.
    - `port` {number} Port the client should connect to.
    - `socket` {stream.Duplex} Optional, typically an instance of `net.Socket`. If this options is specified, host and port are ignored. The user passing the options is responsible for it connecting to the server. `tls.connect` won't call `net.connect` on it.
    - `rejectUnauthorized` {boolean} Whether the server certificate should be verified against the list of supplied CAs. An `error` event is emitted if verifications fails; `err.code` contains the MbedTLS error code. Defaults to `false`. NOT READY
    - `servername` {string} Server name for the SNI (Server name Indication) TLS extension. NOT READY
    - `session` {Buffer} A `Buffer` containing a TLS session. NOT READY
    - `minDHSize` {number} The minimum size of the DH parameter in bits to accept a TLS connection. If a server offers a DH parameter with a size less than specified, the TLS connection is destroyed and an error is thrown. Defaults to `1024`.
    - `lookup` {Function} Custom lookup. Defaults to `dns.lookup()`.
- `callback` {Function} The callback function will be added as a listener for the `secureConnect` event.

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
- `port` {number} Port the client should connect to.
- `host` {string} Host the client should connect to, defaults to 'localhost'.
- `options` {Object} See `tls.connect()`.
- `callback` {Function} See `tls.connect()`.

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
