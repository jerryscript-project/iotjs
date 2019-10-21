### Platform Support

 The following shows Http module APIs available for each platform.

 |  | Linux<br/>(Ubuntu) | Tizen<br/>(Raspberry Pi) | Raspbian<br/>(Raspberry Pi) | NuttX<br/>(STM32F4-Discovery) | TizenRT<br/>(Artik053) |
 | :--- | :---: | :---: | :---: | :---: | :---: |
 | http.createServer                    | O | O | O | △ ¹ | △ ¹ |
 | http.request                         | O | O | O | △ ¹ | △ ¹ |
 | http.get                             | O | O | O | △ ¹ | △ ¹ |
 | http.METHODS                         | O | O | O | O   | O   |
 | http.Server                          | O | O | O | △ ¹ | △ ¹ |
 | http.Server.close                    | O | O | O | △ ¹ | △ ¹ |
 | http.Server.listen                   | O | O | O | △ ¹ | △ ¹ |
 | http.Server.setTimeout               | O | O | O | △ ¹ | △ ¹ |
 | http.ClientRequest                   | O | O | O | △ ¹ | △ ¹ |
 | http.ClientRequest.abort             | O | O | O | △ ¹ | △ ¹ |
 | http.ClientRequest.end               | O | O | O | △ ¹ | △ ¹ |
 | http.ClientRequest.setTimeout        | O | O | O | △ ¹ | △ ¹ |
 | http.ClientRequest.write             | O | O | O | △ ¹ | △ ¹ |
 | http.IncomingMessage                 | O | O | O | △ ¹ | △ ¹ |
 | http.IncomingMessage.headers         | O | O | O | △ ¹ | △ ¹ |
 | http.IncomingMessage.method          | O | O | O | △ ¹ | △ ¹ |
 | http.IncomingMessage.httpVersion     | O | O | O | △ ¹ | △ ¹ |
 | http.IncomingMessage.socket          | O | O | O | △ ¹ | △ ¹ |
 | http.IncomingMessage.statusCode      | O | O | O | △ ¹ | △ ¹ |
 | http.IncomingMessage.url             | O | O | O | △ ¹ | △ ¹ |
 | http.IncomingMessage.statusMessage   | O | O | O | △ ¹ | △ ¹ |
 | http.IncomingMessage.setTimeout      | O | O | O | △ ¹ | △ ¹ |
 | http.ServerResponse                  | O | O | O | △ ¹ | △ ¹ |
 | http.ServerResponse.end              | O | O | O | △ ¹ | △ ¹ |
 | http.ServerResponse.getHeader        | O | O | O | △ ¹ | △ ¹ |
 | http.ServerResponse.setHeader        | O | O | O | △ ¹ | △ ¹ |
 | http.ServerResponse.setTimeout       | O | O | O | △ ¹ | △ ¹ |
 | http.ServerResponse.write            | O | O | O | △ ¹ | △ ¹ |
 | http.ServerResponse.writeHead        | O | O | O | △ ¹ | △ ¹ |

1. On NuttX/STM32F4-Discovery and TizenRT/Artik053, even a couple of sockets/server/requests might not work properly.

# Http

IoT.js provides HTTP to support HTTP server and client enabling users to receive/send HTTP request easily.

### http.createServer([options][, requestListener])
* `options` {Object}
  * `IncomingMessage` {Function} Specifies the `IncomingMessage` constructor to be used when creating an http incoming message object.
    Useful when extending the original {http.IncommingMessge}.
    Default: `http.IncommingMessage`.
  * `ServerResponse` {Function} Specifies the `ServerResponse` constructor to be used when creating the server response object.
    Useful when extending the original {http.ServerResponse}.
    Default: 'http.ServerResponse`.
* `requestListener` {Function}
  * `request` {http.IncomingMessage}
  * `response` {http.ServerResponse}
* Returns: {http.Server}

This call only creates the HTTP server instance and does not start the server.
To start the server and listen for connections use the `server.listen` method.

If a server is no longer needed, all request and response streams should be closed and the `server.close` method
should be used to stop the server listening for connections.

The `requestListener` is a function which is automatically added to the `'request'` event of the http server.

**Example**

```js
var console = require('console');
var http = require('http');

var server = http.createServer(function(request, response) {
  console.log('Request for path: ' + request.url);

  var message = '<h1>Hello</h1>';

  response.setHeader('Content-Type', 'text/html');
  response.setHeader('Content-Length', message.length);
  response.writeHead(200);
  response.write(message);
  response.end();
});

var port = 8081
server.listen(port, function() {
  console.log('HTTP server listening on port: ' + port);
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

The function creates a `http.ClientRequest` instance with the `options` defined.
This can be used to get data from a server or to send data for a server.

In case of data send the `'Content-Length'` header should be specifed so the server can properly handle the request.

**Example**

```js
var http = require('http');

var data_A = 'Data to upload..';
var data_B = 'more data';
var request = http.request({
  method: 'POST',
  port: 8081,
  headers: { 'Content-Length': data_A.length + data_B.length }
});

request.write(data_A);
request.write(data_B);

request.end();
```

Note that in the example `request.end()` was called. With `http.request()` one must always call
`request.end()` to signify that you're done with the request - even if there is no data being written to the request body.

### http.get(options[, callback])
* `options` {Object}
* `callback` {Function}
  * `response` {http.IncomingMessage}
* Returns: {http.ClientRequest}

Same as `http.request` except that `http.get` automatically calls `request.end()` before returning the `http.ClientRequest`
instance thus calling the `write` method on the return value is invalid.

This method is usefuly when there is no HTTP body to send.

**Example**

```js
var http = require('http');

http.get({
  port: 80,
}, function(response) {
  console.log('Got response');
  response.on('data', function(chunk) {
    console.log('Chunk: ');
    console.log(chunk.toString());
  });
});
```


### http.METHODS
* {string[]}

A list of HTTP methods supported by the parser as a `string` array.

## Class: http.Server

This class inherits from `net.Server` and represents a HTTP server.

### Event: 'clientError'
Event callback arguments:
* `exception` {Error} Describes what kind of error occured.
* `socket` {net.Socket} The socket which triggered the error.

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
This event is emitted when the HTTP server is closed.

**Example**

```js
var console = require('console');
var http = require('http');

var server = http.createServer();

server.on('close', function() {
  console.log('Server closed');
});

server.listen(8081, function() {
  console.log('HTTP server listening');

  server.close();
});
```

When the example is executed the following will text will be printed:

```
HTTP server listening
Server closed
```

### Event: 'connection'
Event callback argument:
* `socket` {net.Socket}

This event is emitted when new TCP connection is established. This event is triggered before
the `request` event. At this stage no HTTP header or data is processed.

Usually there is no need to listen for this event.


### Event: 'request'
* `request` {http.IncomingMessage} Represents the HTTP request sent by the client.
* `response` {http.ServerResponse} Represents the HTTP response which will be sent by the server.

After HTTP request headers are parsed, the `'request'` event will be fired.

**Example**

```js
var console = require('console');
var http = require('http');

var server = http.createServer();

server.on('request', function(request, response) {
  console.log('Request for path: ' + request.url);

  var message = '<h1>Hello</h1>';

  response.setHeader('Content-Type', 'text/html');
  response.setHeader('Content-Length', message.length);
  response.writeHead(200);
  response.write(message);
  response.end();
});

var port = 8081
server.listen(port, function() {
  console.log('HTTP server listening on port: ' + port);
});
```

### server.timeout
* {number}

The number of milliseconds of inactivity before a socket is presumed to have timed out. Default value is 120000 (2 minutes).

### server.listen(port[, hostname][, backlog][, callback])
* `port` {number} Port number to listen on.
* `host` {string} Host IP or name where the server should listen. Default: `'0.0.0.0'`.
* `backlog` {number} The number of maximum pending connections. Default backlog length is 511 (not 512).
* `callback` {Function} Callback called when the `'listening'` event is emitted by the underlying `net.Server`.
* Returns {http.Server} The same server instance which was used to call the `listen` method.

Wait for new TCP connections with specified port and hostname. If no hostname is provided, server listens on all available IP address.

**Example**

```js
var console = require('console');
var http = require('http');

var server = http.createServer(function(req, res) {
  res.end();
});
server.listen(8080, function() {
  console.log('Started listening');
});
```

### server.close([callback])
* `callback` {Function} Function which to be registered for the `'close'` event.
* Returns {http.Server} The same server instance which was used to call the `close` method.

Stop accepting new connections to this server. However, the existing connections are preserved.
When the server is finally closed after all connections was closed, the `'close'` event is triggered.

See the `'close`' event.

### server.setTimeout(ms[, callback])

* `ms` {number}
* `callback` {Function} The callback function registered for the `'timeout'` event.

Registers cb for `'timeout'` event and sets socket's timeout value to ms. This event will be triggered by the underlying socket's `'timeout'` event.

If `callback` is not provided, the socket will be destroyed automatically after timeout.
If the `callback` function is provided, that function should should handle the socket's timeout.

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


## Class: http.ClientRequest

This object is created internally and returned from `http.request()`. It represents an in-progress request whose headers have already been queued.

### Event: 'close'
This event is fired when the underlying socket is closed.

### Event: 'error'
Event callback arguments:
* `err` {Error}

Emitted if something went wrong with making or parsing the request.

### Event: 'finish'
This event is emitted after all the data was sent, meaning header and body all finished sending.

### Event: 'response'
Event callback arguments:
* `response` {http.IncomingMessage} The incoming HTTP response from the server.

This event is emitted when server's HTTP response header is parsed.
The event is called only once. The developer should attach at least one event handler for this event
to correctly process any data sent back by the target server.

**Example**

```js
var http = require('http');

var options = {
  host: 'localhost',
  port: 8081,
  method: 'GET',
};
var client_request = http.request(options);
client_request.on('response', function(response) {
  console.log('HTTP status: ' + response.statusCode);
  console.log('Headers:');
  console.log(response.headers);

  response.on('data', function(chunk) {
    console.log(chunk.toString());
  });
});
client_request.end();
```

### Event: 'socket'
Event callback arguments:
* `socket` {net.Socket}

This event is emitted when a socket is assigned to this request.

After response header is parsed, this event will be fired.


### request.abort()
Will abort the outgoing request, dropping any data to be sent/received and destroying the underlying socket.

### request.end([data][, callback])
* `data` {Buffer | string} Data to be sent.
* `callback` {Function} Callback function invoked when all data is processed.

Finishes sending the request.

If `data` is provided, it sends `data` first, and finishes.
If `callback` is specified, it is called when the request stream is finished.

This method must be called to close the request and to make sure all data is sent.

**Example**

```js
var http = require('http');

var message = 'HTTP Body POST Data';
var options = {
  host: 'localhost',
  port: 8081,
  method: 'POST',
  headers: {'Content-Length': message.length},
};
var client_request = http.request(options, function(response) {
  console.log('HTTP status: ' + response.statusCode);
});
client_request.end(message);
```


### request.setTimeout(ms[, callback])

* `ms` {number}
* `callback` {Function} The callback function registered for the `'timeout'` event.

Registers `callback` for 'timeout' event and set socket's timeout value to ms. This event will be triggered by the underlying socket's `'timeout'` event.

If `callback` is not provided, the socket will be destroyed automatically after timeout.
If `callback` is provied, the method should handle the socket's timeout.

### request.write(data[, callback])
* `data` {Buffer | string} Data to be sent.
* `callback` {Function}

Sends `data` as a request body. `callback` will be called when data is flushed.


**Example**

```js
var http = require('http');

var message = "This is the data";
var options = {
  method: 'POST',
  port: 8383,
  path: '/',
  headers: {'Content-Length': message.length},
};

var client_request = http.request(options);
client_request.write(message);
client_request.end();
```

## Class: http.IncomingMessage

This object is created internally and returned to the callback for the http.ClientRequest `'response'` event and
for the `'request'` event in the http.Server class.
In case of the `http.ClientRequest` class this `IncomingMessage` will represent the response sent by a server for the given request.
In case of the `http.Server` class this will represent the request sent by a client for the server.

http.IncomingMessage inherits [`Stream.readable`](IoT.js-API-Stream.md).
See it's documentation to read incoming data from an HTTP request.
Notable events are `'data'` (fired when there is data to read), `'close'`, `'end'` (Request has ended) and the method `readable.read()`.


### Event: 'close'
When underlying connection is closed, 'close' event is emitted.

### Event: 'data'
Event callback arguments:
* `chunk` {Buffer} the buffer containing the data.

Raised when there is data to be processed from the underlying socket.
It is highly possible that this chunk of data is not the whole data,
thus if the developer needs the whole data in one, each chunk must be
stored. (See the example for a naive approach.)

The HTTP headers are already parsed before this event is triggered.

Please note that the total size of the data could be bigger
than the memory available on the device where the code is running.


**Example**

```js
var console = require('console');
var http = require('http');

var options = {
  host: 'localhost',
  port: 8081,
  method: 'GET',
  path: '/'
};
var client_request = http.request(options, function(response) {
  var parts = [];
  response.on('data', function(chunk) {
    parts.push(chunk);
  });
  response.on('end', function() {
    var body = Buffer.concat(parts);
    console.log(body.toString());
  });
});
client_request.end();
```

### Event: 'end'
This event is fired when no more data to be received.
At this point it is safe to assume all data was received from the other end.

### message.headers
A JavaScript object containing all HTTP headers sent by the other end.

### message.method
Requests method as `string`

### message.httpVersion
The HTTP version sent by the client. One of the following value: `'1.1'` or `'1.0'`.

### message.socket
Underlying network socket (`net.Socket`).

### message.statusCode
HTTP response status code as `number` of 3-digit.

### message.url
Request URL as `string`. Only contains the URL present in the HTTP request.

Note: only valid if the `IncomingMessage` was constructed by a `http.Server`.

**Example**

If the HTTP request is the following:

```
GET /page/1?data=a HTTP/1.1 \r\n
Accept: text/html\r\n
\r\n
```

the `message.url` will be: `/page/1?data=a`.

### message.statusMessage
HTTP response status message as `string`.

### message.setTimeout(ms, cb)

* `ms` {number}
* `cb` {Function}

Registers cb for 'timeout' event set socket's timeout value to ms. This event will be triggered by the underlying socket's 'timeout' event.


## Class: http.ServerResponse

Created internally when the `'request'` event is triggered by the `http.Server` class and
represents the response sent by the server to a client.

### Event: 'close'
When underlying connection is closed, 'close' event is emitted.

### Event: 'end'
This event is fired when no more data to be sent.

### Event: 'finish'
This event is emitted when the response has been sent. It does not guarantee that client has received data yet.


### response.end([data][, callback])
* `data` {Buffer | string} Data which should be sent.
* `callback` {Function}

Finishes sending the response.

If `data` is provided, it sends `data` first, and finishes.
If `callback` is specified, it is called when the response stream is finished.

The method should be called to correctly finish up a response.
Any method which sets headers must be called before this method and before any `write` calls.

**Example**

```js
var console = require('console');
var http = require('http');

var server = http.createServer(function(request, response) {
  console.log('Request for path: ' + request.url);

  var message = '<h1>Hello</h1>';

  response.setHeader('Content-Type', 'text/html');
  response.setHeader('Content-Length', message.length);
  response.writeHead(200);
  response.end(message);
});

var port = 8081
server.listen(port, function() {
  console.log('HTTP server listening on port: ' + port);
});
```

### response.getHeader(name)
* `name` {string} Case-sensitive HTTP header field name.

Returns the value of the `name` HTTP header field.

### response.removeHeader(name)
* `name` {string} Case-sensitive HTTP header field name.

Remove the HTTP header which has the `name` field name.
HTTP headers can not be modified after the first `write`, `writeHead` or `end` method call.

### response.setHeader(name, value)
* `name` {string} The name of the HTTP header field to set.
* `value` {string} The value of the field.

Sets response's header field(`name`) to `value`. If the field exists, it overwrites the existing value.
HTTP headers can not be modified after the first `write`, `writeHead` or `end` method call.

### response.setTimeout(ms, cb)

* `ms` {number}
* `cb` {Function}

Registers cb for 'timeout' event and set socket's timeout value to ms. This event will be triggered by the underlying socket's 'timeout' event.

### response.write(data[, callback])
* `data` {Buffer | string}
* `callback` {Function}

Sends `data` as a response body. `callback` will be called when data is flushed.

It is advised to set at least the `Content-Length` HTTP header field correctly before
any `write` calls. This is so the client could properly handle the server response.

After a `write` method was called there is no possibility to change any headers.

**Example**

```js
var console = require('console');
var http = require('http');

var server = http.createServer(function(request, response) {
  console.log('Request for path: ' + request.url);

  var message = '<h1>Hello</h1>';

  response.setHeader('Content-Type', 'text/html');
  response.setHeader('Content-Length', message.length);
  response.writeHead(200);
  response.write(message);
  response.end();
});

var port = 8081
server.listen(port, function() {
  console.log('HTTP server listening on port: ' + port);
});
```

### response.writeHead(statusCode[, statusMessage][, headers])
* `statusCode` {number}
* `statusMessage` {string} Optional. If not set the HTTP status message will be inferred from the status code.
* `headers` {Object} Optional. An object containing HTTP header field names and values.

Sets response status code, the status message and configures a set of HTTP
header values.

**Example**

```js
var console = require('console');
var http = require('http');

var server = http.createServer(function(request, response) {
  console.log('Request for path: ' + request.url);

  var message = '<h1>Hello</h1>';

  response.writeHead(200, 'OK', {
    'Content-Type': 'text/html',
    'Content-Length': message.length,
  });
  response.write(message);
  response.end();
});

var port = 8081
server.listen(port, function() {
  console.log('HTTP server listening on port: ' + port);
});
```
