### Platform Support

 The following shows Http module APIs available for each platform.

 |  | Linux<br/>(Ubuntu) | Raspbian<br/>(Raspberry Pi) | NuttX<br/>(STM32F4-Discovery) | TizenRT<br/>(Artik053) |
 | :---: | :---: | :---: | :---: | :---: |
 | http.createServer | O | O | △ ¹ | - |
 | http.request | O | O | △ ¹ | - |
 | http.get | O | O | △ ¹ | - |

1. On NuttX/STM32F4-Discovery, even a couple of sockets/server/requests might not work properly.

# Http

IoT.js provides HTTP to support HTTP server and client enabling users to receive/send HTTP request easily.

### http.createServer([requestListener])
* `requestListener` {Function}
  * request {http.IncomingMessage}
  * response {http.ServerResponse}
* Returns: {http.Server}

The `requestListener` is a function which is automatically added to the `'request'` event.

**Example**

```js
var server = http.createServer(function(request, response) {
  ...
});
```

### http.request(options[, callback])
* `options` {Object}
  * `host` {string} A domain name or IP address of the server to issue the request to. Defaults to 'localhost'.
  * `hostname` {string} Alias for host.
  * `port` {number} Port of remote server. Defaults to 80.
  * `method` {string} A string specifying the HTTP request method. Defaults to 'GET'.
  * `path` {string} Request path. Defaults to '/'. Should include query string if any. E.G. '/index.html?page=12'. An exception is thrown when the request path contains illegal characters. Currently, only spaces are rejected but that may change in the future.
  * `headers` {Object} An object containing request headers.
* `callback` {Function}
  * `response` {http.IncomingMessage}
* Returns: {http.ClientRequest}

**Example**

```js
var http = require('http');

var request = http.request({
  method: 'POST',
  port: 80,
  headers: {'Content-Length': 3}
});

...

request.end();
```

Note that in the example `req.end()` was called. With `http.request()` one must always call `req.end()` to signify that you're done with the request - even if there is no data being written to the request body.

### http.get(options[, callback])
* `options` {Object}
* `callback` {Function}
  * `response` {http.IncomingMessage}
* Returns: {http.ClientRequest}

Same as `http.request` except that `http.get` automatically call `req.end()` at the end.

**Example**

```js
var http = require('http');

http.get({
  port: 80,
}, function(res) {
...
});
```


### http.METHODS
A list of HTTP methods supported by the parser as `string` properties of an `Object`.

## Class: http.Server

This class inherits from `net.Server`.

### Event: 'clientError'
* `exception` {Error}
* `socket` {net.Socket}

If a client connection emits an 'error' event, it will be forwarded here. Listener of this event is responsible for closing/destroying the underlying socket.

Default behavior is to destroy the socket immediately on malformed request.

**Example**

```js
var http = require('http');

var server = http.createServer(function(req, res) {
  res.end();
});

server.on('clientError', function(err, socket) {
  socket.end('HTTP/1.1 400 Bad Request\r\n\r\n');
});

server.listen(8000);
```

### Event: 'close'
This event is emitted when server is closed.

### Event: 'connection'
* `socket` {net.Socket}

This event is emitted when new TCP connection is established.

### Event: 'connect'

Emitted each time a client requests an `HTTP CONNECT` method.

### Event: 'request'
* `request` {http.IncomingMessage}
* `response` {http.ServerResponse}

After request header is parsed, this event will be fired.


### server.timeout
The number of milliseconds of inactivity before a socket is presumed to have timed out. Default value is 120000 (2 minutes).

### server.listen(port[, hostname][, backlog][, callback])
* `port` {number}
* `host` {string}
* `backlog` {number}
* `callback` {Function}

Wait for new TCP connection with specified port and hostname. If no hostname is provided, server accepts any IP address.
`backlog` is maximum pending connections. Default backlog length is 511 (not 512).
`callback` will be called when server has been bound.

**Example**

```js
var http = require('http');

var server = http.createServer(function(req, res) {
  res.end();
});
server.listen(8080, function() {});
```

### server.close([callback])
* `callback` {Function}

Stop accepting new connection to this server. However, the existing connections are preserved. When server is finally closed after all connections are closed, a callback is called.

### server.setTimeout(ms, cb)

* `ms` {number}
* `cb` {Function}

Registers cb for `'timeout'` event and sets socket's timeout value to ms. This event will be triggered by the underlying socket's 'timeout' event.

If cb is not provided, the socket will be destroyed automatically after timeout.
If you provide cb, you should handle the socket's timeout.

Default timeout for server is 2 minutes.

**Example**

```js
var http = require('http');

var server = http.createServer();
server.setTimeout(100, function(socket) {
  socket.destroy();
  server.close();
});
```

## Class: http.ServerResponse

### Event: 'close'
When underlying connection is closed, 'close' event is emitted.

### Event: 'end'
This event is fired when no more data to be sent.

### Event: 'finish'
This event is emitted when the response has been sent. It does not guarantee that client has received data yet.


### response.end([data][, callback])
* `data` {Buffer | string}
* `callback` {Function}

Finishes sending the response.

If `data` is provided, it sends `data` first, and finishes.
If `callback` is specified, it is called when the response stream is finished.

### response.getHeader(name)
* `name` {string}

Returns `name` field of response's header

### response.removeHeader(name)
* `name` {string}

Removes `name` field from response's header

### response.setHeader(name, value)
* `name` {string}
* `value` {string}

Sets response's header field(`name`) to `value`. If the field exists, it overwrites the existing value.

### response.setTimeout(ms, cb)

* `ms` {number}
* `cb` {Function}

Registers cb for 'timeout' event and set socket's timeout value to ms. This event will be triggered by the underlying socket's 'timeout' event.

### response.write(data[, callback])
* `data` {Buffer | string}
* `callback` {Function}

Sends `data` as a response body. `callback` will be called when data is flushed.

### response.writeHead(statusCode[, statusMessage][, headers])
* `statusCode` {number}
* `statusMessage` {string}
* `headers` {Object}

Sets response's header. `headers` is a map between field and value in header.


## Class: http.ClientRequest

This object is created internally and returned from http.request(). It represents an in-progress request whose header has already been queued.

### Event: 'close'
This event is fired when the underlying socket is closed.

### Event: 'error'
* `callback` {Function}
* `err` {Error}

Emitted if something went wrong with making or parsing the request.

### Event: 'finish'
This event is emitted after all the data was sent, meaning header and body all finished sending.

### Event: 'response'
* `response` {http.IncomingMessage}

This event is emitted when server's response header is parsed. ` http.IncomingMessage` object is passed as argument to handler.

### Event: 'socket'
* `socket` {net.Socket}

This event is emitted when a socket is assigned to this request. `net.Socket` object is passed as argument to handler.

After response header is parsed, this event will be fired.


### request.end([data][, callback])
* `data` {Buffer | string}
* `callback` {Function}

Finishes sending the request.

If `data` is provided, it sends `data` first, and finishes.
If `callback` is specified, it is called when the request stream is finished.

### request.setTimeout(ms, cb)

* `ms` {number}
* `cb` {Function}

Registers cb for 'timeout' event and set socket's timeout value to ms. This event will be triggered by the underlying socket's 'timeout' event.

If cb is not provided, the socket will be destroyed automatically after timeout.
If you provides cb, you should handle the socket's timeout.

### request.write(data[, callback])
* `data` {Buffer | string}
* `callback` {Function}

Sends `data` as a request body. `callback` will be called when data is flushed.


## Class: http.IncomingMessage

http.IncomingMessage inherits `Stream.readable`.

### Event: 'close'
When underlying connection is closed, 'close' event is emitted.

### Event: 'end'
This event is fired when no more data to be received.


### message.headers
HTTP header object.

### message.method
Requests method as `string`

### message.socket
Underlying socket

### message.statusCode
HTTP response status code as `number` of 3-digit.

### message.statusMessage
HTTP response status message as `string`

### message.url
Requests URL as `string`

### message.setTimeout(ms, cb)

* `ms` {number}
* `cb` {Function}

Registers cb for 'timeout' event set socket's timeout value to ms. This event will be triggered by the underlying socket's 'timeout' event.
