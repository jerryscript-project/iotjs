### Platform Support

The following shows dgram module APIs available for each platform.

|  | Linux<br/>(Ubuntu) | Raspbian<br/>(Raspberry Pi) | NuttX<br/>(STM32F4-Discovery) | TizenRT<br/>(Artik053) |
| :---: | :---: | :---: | :---: | :---: |
| dgram.createSocket | O | O | △ ¹ | - |
| dgram.Socket.addMembership | O | O | X | - |
| dgram.Socket.address | O | O | X | - |
| dgram.Socket.bind | O | O | △ ¹ | - |
| dgram.Socket.close | O | O | △ ² | - |
| dgram.Socket.dropMembership | O | O | X | - |
| dgram.Socket.send | O | O | △ ¹ | - |
| dgram.Socket.setBroadcast | O | O | X | - |
| dgram.Socket.setMulticastLoopback | O | O | X | - |
| dgram.Socket.setMulticastTTL | X | X | X | - |
| dgram.Socket.setTTL | O | O | X | - |

1. On NuttX/STM32F4-Discovery, even a couple of sockets/server/requests might not work properly.

2. On NuttX/STM32F4-Discovery, close() may block due to a bug in poll().

# Dgram

The dgram module provides an implementation of UDP Datagram sockets.

The following example creates a UDP Datagram server.

**Example**

```js
var dgram = require('dgram');
var server = dgram.createSocket('udp4');

server.on('error', function (err) {
  console.log('Error: ' + err);
  server.close();
});

server.on('message', function(msg, rinfo) {
  // prints: message received
  console.log('server got: ' + msg);
});

server.on('listening', function() {
  console.log('server listening at ' + server.address().port);
});

server.bind(41234);
```

### dgram.createSocket(options[, callback])
* `options` {Object}
  * `type` {string}
  * `reuseAddr` {boolean}
* `callback` {Function} (optional)

Creates a new `dgram.Socket` object. The type of the connection
is specified by `options.type`. Currently only `udp4` is supported.

If `reuseAddr` is true the `socket.bind()` call reuses the address
even if this address has been bound by another process.

The optional 'callback' function is attached to the
[`'message'`](#event-message) event.

**Example**

```js
var dgram = require('dgram');
var server = dgram.createSocket({ type: 'udp4', reuseAddr: true});
```

### dgram.createSocket(type[, callback])
* `type` {string}
* `callback` {Function} (optional)

Creates a new `dgram.Socket` object. The type of the connection
is specified by the `type` argument. Currently only `udp4` is
supported.

The optional 'callback' function is attached to the
[`'message'`](#event-message) event.

**Example**

```js
var dgram = require('dgram');
var server = dgram.createSocket('udp4');
```

## Class: dgram.Socket

The `dgram.Socket` object is an `EventEmitter` that encapsulates the
datagram functionality.

New instances of `dgram.Socket` are created using `dgram.createSocket()`.
The new keyword must not be used to create dgram.Socket instances.

Supported events:

### Event: 'close'

The `'close'` event is emitted after a socket is closed with `close()`.
Once triggered, no new `'message'` events will be emitted on this socket.

### Event: 'error'

* `exception` {Error}

The `'error'` event is emitted whenever any error occurs. A single Error
object is passed to the event handler.

### Event: 'listening'

The `'listening'` event is emitted whenever a socket begins listening
for datagram messages. This occurs as soon as UDP sockets are created.

### Event: 'message'

* `msg` {Buffer} The message.
* `rinfo` {Object} Remote address information.
  * `address` {string} The sender address.
  * `family` {string} The address family ('IPv4').
  * `port` {number} The sender port.
  * `size` {number} The message size.

The `'message'` event is emitted when a new datagram is received by
the socket. The `msg` argument contains the message data and the `rinfo`
argument contains the message properties.


### socket.addMembership(multicastAddress[, multicastInterface])
* `multicastAddress` {string}
* `multicastInterface` {string}

Joins the multicast group specified by `multicastAddress` and
`multicastInterface`. If `multicastInterface` is undefined the
operating system will choose one add interface and will add
membership to it. To add membership to every available interface,
call addMembership multiple times, once per interface.


**Example**

```js
var dgram = require('dgram');
var multicast_address = '230.255.255.250';
var server = dgram.createSocket('udp4');

server.bind(12345 /* port */, function() {
  server.addMembership(multicast_address);
});
```


### socket.address()
* Returns: {Object}

Returns an object with the properties `address`, `port` and `family`.

**Example**

```js
var dgram = require('dgram');
var server = dgram.createSocket('udp4');

server.on('listening', function () {
  var address = server.address();
  // prints: address and port of the server address
  console.log('Addr: ' + address.address + ' port: ' + address.port);
});

server.bind(12345 /* port */);
```

### socket.bind([port][, address][, bindListener])
* `port` {number}
* `address` {string} **Default:** `0.0.0.0`
* `bindListener` {Function}

Assign the `port` and `address` to an UDP socket. If `port` is
not specified the operating system selects a random unused
port.

The optional 'bindListener' function is attached to the
[`'listening'`](#event-listening) event.

**Example**

```js
var dgram = require('dgram');
var socket = dgram.createSocket({ type: 'udp4', reuseAddr: true });

var port = 12345;

socket.bind(port, function () {
  // prints: Listening for packets
  console.log('Listening for packets');
});
```


### socket.bind(options[, bindListener])
* `options` {Object}
  * `port` {number}
  * `address` {string} **Default:** `0.0.0.0`
* `bindListener` {Function}

Assign `options.port` and `options.address` to an UDP socket.
If `options.port` is not specified the operating system selects
a random unused port.

The optional 'bindListener' function is attached to the
[`'listening'`](#event-listening) event.

**Example**

```js
var dgram = require('dgram');
var socket = dgram.createSocket({ type: 'udp4', reuseAddr: true });

socket.bind({ port:12345 }, function () {
  // prints: Listening for packets
  console.log('Listening for packets');
});
```


### socket.close([closeListener])
* `closeListener` {Function}

Close the underlying socket and stop listening for data on it.

The optional 'closeListener' function is attached to the
[`'close'`](#event-close) event.

**Example**

```js
var dgram = require('dgram');
var socket = dgram.createSocket({ type: 'udp4', reuseAddr: true });

// prints: Close!
socket.close(function () {
  // prints: Socket is closed
  console.log('Socket is closed');
});
```


### socket.dropMembership(multicastAddress[, multicastInterface])
* `multicastAddress` {string}
* `multicastInterface` {string}

Leaves for socket the given multicast group with given `multicastAddress` and `multicastInterface`.

**Example**

```js
var dgram = require('dgram');
var server = dgram.createSocket('udp4');
var multicast_address = '230.255.255.250';

server.bind(12345 /* port */, function() {
  server.addMembership(multicast_address);
});

server.on('message', function(data, rinfo) {
  // Drop membership when a message arrived.
  server.dropMembership(multicast_address);
});
```


### socket.setBroadcast(flag)
* `flag` {boolean}

Sets or clears the `SO_BROADCAST` socket option. When `flag` is
true UDP packets may be sent to a local interface's broadcast
address.

**Example**

```js
var dgram = require('dgram');
var socket = dgram.createSocket({ type: 'udp4', reuseAddr: true });
var port = 41237;

socket.bind(port, function() {
  socket.setBroadcast(true);
});
```


### socket.send(msg, [offset, length,] port [, address] [, sendListener])
* `msg` {Buffer|string|array}
* `offset` {integer} Only valid if `msg` is Buffer.
* `length` {integer} Only valid if `msg` is Buffer.
* `port` {integer}
* `address` {string} **Default:** `127.0.0.1` or `::1`
* `sendListener` {Function}
  * `Error` {Object|null}
    * `code` {string} Currently it is always `"error"`.
    * `errno` {string} Same as `code`.
    * `syscall` {integer}
    * `address` {string}
    * `port` {integer}
  * `length` {integer} Length of data.

Transmits a message to the destination socket specified by
`address` and `port` arguments. The `msg` argument contains
the data to be sent. It can be a {Buffer}, a {string} converted
to UTF-8 bytes, or an array of {Buffer} and {string} values.
In the latter case the items of the array are concatenated
into a single {Buffer} before sending.

If send operation is successfully completed, `sendListener` will
be called with `null` and the length of data. Otherwise an
Error {Object} is passed along with the length of data.

**Example**

```js
var dgram = require('dgram');
var socket = dgram.createSocket({ type: 'udp4', reuseAddr: true });
var broadcast_address = '255.255.255.255';
var port = 41237;

socket.bind(port, function() {
  socket.setBroadcast(true);
  socket.send('Hello IoT.js', port, broadcast_address);
});
```

### socket.sendto(msg, offset, length, port [, address] [, sendListener])
* `msg` {Buffer|string|array}
* `offset` {integer}
* `length` {integer}
* `port` {integer}
* `address` {string} **Default:** `127.0.0.1` or `::1`
* `sendListener` {Function}

Legacy function. It is the same as
[`socket.send`](#socket-send-msg-offset-length-port-address-sendListener)
except `offset` and `length` arguments are mandatory.

### socket.setMulticastLoopback(flag)
* `flag` {boolean}

Sets or clears the `IP_MULTICAST_LOOP` socket option. When `flag` is
`true` multicast packets will also be received on the local interface.

**Example**

```js
var dgram = require('dgram');
var socket = dgram.createSocket({ type: 'udp4', reuseAddr: true });
var port = 41237;

socket.bind(port, function() {
  socket.setMulticastLoopback(true);
});
```


### socket.setMulticastTTL(ttl)
* `ttl` {integer} This value must be between 0 and 255.

Sets the `IP_MULTICAST_TTL` socket option which pecifies the
number of IP hops that a packet is allowed to travel through,
specifically for multicast traffic. Each router or gateway
that forwards a packet decrements its TTL. When TTL reaches
zero the packet is not forwarded anymore.

The default on most systems is 1 but can vary.


**Example**

```js
var dgram = require('dgram');
var socket = dgram.createSocket({ type: 'udp4', reuseAddr: true });
var port = 41237;

socket.bind(port, function() {
  socket.setMulticastTTL(1);
});
```


### socket.setTTL(ttl)
* `ttl` {integer} This value must be between 1 and 255.

Sets the `IP_TTL` socket option which specifies the number of IP
hops that a packet is allowed to travel through. Each router or
gateway that forwards a packet decrements its TTL. When TTL reaches
zero the packet is not forwarded anymore.

The default on most systems is 64 but can vary.

**Example**

```js
var dgram = require('dgram');
var socket = dgram.createSocket({ type: 'udp4', reuseAddr: true });
var port = 41237;

socket.bind(port, function() {
  socket.setTTL(64);
});
```
