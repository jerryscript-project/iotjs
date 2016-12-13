## Module: dgram

### Platform Support

The following shows dgram module APIs available for each platform.

|  | Linux<br/>(Ubuntu) | Raspbian<br/>(Raspberry Pi) | Nuttx<br/>(STM32F4-Discovery) |
| :---: | :---: | :---: | :---: |
| dgram.createSocket | O | O | O |
| dgram.Socket.bind | O | O | O |
| dgram.Socket.close | O | O | O |
| dgram.Socket.send | O | O | O |
| dgram.Socket.address | O | O | X |
| dgram.Socket.setBroadcast | O | O | X |
| dgram.Socket.setTTL | O | O | X |
| dgram.Socket.addMembership | O | O | X |
| dgram.Socket.dropMembership | O | O | X |
| dgram.Socket.setMulticastLoopback | O | O | X |
| dgram.Socket.setMulticastTTL | X | X | X |

IoT.js provides udp connections through Dgram module.

You can use this module with `require('dgram')` and create sockets.


### Module Functions


#### dgram.createSocket(options[, createListener])
#### dgram.createSocket(type[, createListener])
* `options: Object`
 * `type: String`: it indicates an address family either `udp4` or `udp6`.
 * `reuseAddr: Boolean`: it indicates to allow socket to bind the address and port used previously.

* `type: String`: it indicates an address family either 'udp4' or 'udp6'.

* `createListener: Function(msg, rinfo)`
 * `msg: Buffer`
 * `rinfo: Object`
   * `address: String`
   * `family: String`: it indicates an address family either `IPv4` or `IPv6`
   * `port: Number`

Creates a `dgram.Socket` according to `options` or `type`. (Currently we only accept `type`, `udp4`.)

`createListener` is automatically registered as `message` event listener.


***


## class: dgram.Socket

You can create `dgram.Socket` instance with `dgram.createSocket()`.


### Events


#### `'close'`
* `callback: Function()`

Emitted when socket closed.


#### `'error'`
* `callback: Function()`

Emitted when an error occurs.


#### `'listening'`
* `callback: Function()`

Emitted when socket is ready to receive data.


#### `'message'`
* `callback: Function(msg, rinfo)`
 * `msg: Buffer`
 * `rinfo: Object`
   * `address: String`
   * `family: String`: it indicates an address family either `IPv4` or `IPv6`
   * `port: Number`

Emitted when message comes to the socket.



### Methods


#### socket.address()

Returns an object with the properties `address`, `port` and `family`. Basically the returned object is same with the object `rinfo` mentioned above.


#### socket.addMembership(multicastAddress[, multicastInterface])
* `multicastAddress: String`
* `multicastInterface: String`

Joins for socket the given multicast group with given `multicastAddress` and `multicastInterface`.


#### socket.dropMembership(multicastAddress[, multicastInterface])
* `multicastAddress: String`
* `multicastInterface: String`

Leaves for socket the given multicast group with given `multicastAddress` and `multicastInterface`.


#### socket.bind([port][, address][, bindListener])
#### socket.bind(options[, bindListener])
* `port: Number`, if it is not specified, OS will bind socket with a random port.
* `options: Object`
 * `port: Number`
 * `address: String`, Default: `0.0.0.0`
* `bindListener: Function()`

Binds `net.Socket` with given `address` and `port`.

`bindListener` is automatically registered as `listening` event listener.


#### socket.close([closeListener])
* `closeListener: Function()`

Stops listening data.

`closeListener` is registered as `close` event listener.


#### socket.setBroadcast(flag)
* `flag: Boolean`

Sets or clears the `SO_BROADCAST` socket option.


#### socket.setTTL(ttl)
* `ttl: Number`, it should be between 1 and 255.

Sets the `IP_TTL` socket option.


#### socket.send(msg, [offset, length], port, address[, sendListener])
* `msg: Buffer | String | Array`
* `offset: Number`, is is only valid when `msg` is Buffer.
* `length: Number`, is is only valid when `msg` is Buffer.
* `port: Number`
* `address: String`, Default: `127.0.0.1` or `::1`
* `sendListener: Function(err, length)`
 * `err: Null | Error`
   * `code: String`, temporally, it is just a String of *"error"*.
   * `errno: String`, it is same with `code`
   * `syscall: Number`
   * `address: String`
   * `port: Number`
 * `length: Number`, it indicates the length of data.

Sends the message to the destination with given `address` and `port`.

If send operation is successfully completed, `sendListener` will be called with Null and the length of data, otherwise with Error object and the length of data.


#### socket.setMulticastLoopback(flag)
* `flag: Boolean`

Sets or clears the `IP_MULTICAST_LOOP` socket option.


#### socket.setMulticastTTL(ttl)
* `ttl: Number`, it should be between 0 and 255.

Sets the `IP_MULTICAST_TTL` socket option.
