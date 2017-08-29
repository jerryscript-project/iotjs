### Platform Support

 The following shows Https module APIs available for each platform.

|  | Linux<br/>(Ubuntu) | Raspbian<br/>(Raspberry Pi) | Nuttx<br/>(STM32F4-Discovery) | TizenRT<br/>(Artik053) | Tizen<br/>(Artik 10) |
| :---: | :---: | :---: | :---: | :---: | :---: |
| https.request  | X | X | X | X | O |
| https.get  | X | X | X | X | O |


# Https

IoT.js provides HTTPS to support HTTPS clients enabling users to send HTTPS requests easily.

### https.request(options[, callback])
* `options` {Object}
  * `host` {string} A domain name or IP address of the server to issue the request to. **Default:** 'localhost'.
  * `hostname` {string} Alias for host.
  * `port` {number} Port of remote server. **Default:** 80.
  * `method` {string} A string specifying the HTTPS request method. **Default:** 'GET'.
  * `path` {string} Request path. **Default:** '/'. Should include query string if any. E.G. '/index.html?page=12'. An exception is thrown when the request path contains illegal characters. Currently, only spaces are rejected but that may change in the future.
  * `headers` {Object} An object containing request headers.
  * `auth` {string} Optional Basic Authentication in the form `username:password`. Used to compute HTTPS Basic Authentication header.
  * `ca` {string} Optional file path to CA certificate. Allows to override system trusted CA certificates.
  * `cert` {string} Optional file path to client authentication certificate in PEM format.
  * `key` {string} Optional file path to private keys for client cert in PEM format.
  * `rejectUnauthorized` {boolean} Optional Specify whether to verify the Server's certificate against CA certificates. WARNING - Making this `false` may be a security risk. **Default:** `true`
* `callback` {Function}
  * `response` {https.IncomingMessage}
* Returns: {https.ClientRequest}

Example:
```javascript
var https = require('https');

var request = https.request({
  method: 'POST',
  port: 443,
  headers: {'Content-Length': 3}
});

...

request.end();
```

Note that in the example `req.end()` was called. With `https.request()` one must always call `req.end()` to signify that you're done with the request - even if there is no data being written to the request body.

### https.get(options[, callback])
* `options` {Object}
  * `host` {string} A domain name or IP address of the server to issue the request to. **Default:** 'localhost'.
  * `hostname` {string} Alias for host.
  * `port` {number} Port of remote server. **Default:** 80.
  * `method` {string} A string specifying the HTTPS request method. **Default:** 'GET'.
  * `path` {string} Request path. **Default:** '/'. Should include query string if any. E.G. '/index.html?page=12'. An exception is thrown when the request path contains illegal characters. Currently, only spaces are rejected but that may change in the future.
  * `headers` {Object} An object containing request headers.
  * `auth` {string} Optional Basic Authentication in the form `username:password`. Used to compute HTTPS Basic Authentication header.
  * `ca` {string} Optional file path to CA certificate. Allows to override system trusted CA certificates.
  * `cert` {string} Optional file path to client authentication certificate in PEM format.
  * `key` {string} Optional file path to private keys for client cert in PEM format.
  * `rejectUnauthorized` {boolean} Optional Specify whether to verify the Server's certificate against CA certificates. WARNING - Making this `false` may be a security risk. **Default:** `true`
* `callback` {Function}
  * `response` {https.IncomingMessage}
* Returns: {https.ClientRequest}

Same as `https.request` except that `https.get` automatically call `req.end()` at the end.

Example:
```javascript
var https = require('https');

https.get({
  port: 80,
}, function(res) {
...
});
```


### https.METHODS
A list of HTTPS methods supported by the parser as `string` properties of an `Object`. Currently supported methods are `'DELETE'`, `'GET'`, `'HEAD'`, `'POST'`, `'PUT'`, `'CONNECT'`, `'OPTIONS'`, `'TRACE'`.


## Class: https.ClientRequest

This object is created internally and returned from https.request(). It represents an in-progress request whose header has already been queued.

https.ClientRequest inherits [`Stream.writable`](IoT.js-API-Stream.md). See it's documentation to write data to an outgoing HTTP request. Notable methods are `'writable.write()'` (to send data as request body), `'writable.end()'` (to signal an end and flush remaining request body), and the event `'finish'`.

### Event: 'close'
This event is fired when the underlying socket is closed.

### Event: 'response'
* `response` {https.IncomingMessage}

This event is emitted when server's response header is parsed. ` https.IncomingMessage` object is passed as argument to handler.

### Event: 'socket'
This event is emitted when a socket is assigned to this request.

### request.abort()
Will abort the outgoing request, dropping any data to be sent/received and destroying the underlying socket.

### request.aborted
If the request has been aborted, this contains the time at which the request was aborted in milliseconds since epoch as `Number`.

### request.setTimeout(ms, cb)
* `ms` {number}
* `cb` {Function}

Registers cb for 'timeout' event and set socket's timeout value to ms. This event will be triggered by the underlying socket's 'timeout' event.

If cb is not provided, the socket will be destroyed automatically after timeout.
If you provides cb, you should handle the socket's timeout.


## Class: https.IncomingMessage

This object is created internally and returned to the callback in https.request(). It represents the response sent by a server to a request.

https.IncomingMessage inherits [`Stream.readable`](IoT.js-API-Stream.md). See it's documentation to read incoming data from an HTTP request. Notable events are `'data'` (fired when there is data to read), `'close'`, `'end'` (Request has ended) and the method `readable.read()`.

### message.headers
HTTPS header object.

### message.method
Request's method as `string`

### message.statusCode
HTTPS response status code as `number` of 3-digit.

### message.statusMessage
HTTPS response status message as `string`

### message.url
Requests URL as `string`

### message.setTimeout(ms, cb)
* `ms` {number}
* `cb` {Function}

Registers cb for 'timeout' event set socket's timeout value to ms. This event will be triggered by the underlying socket's 'timeout' event.

If cb is not provided, the socket will be destroyed automatically after timeout.
If you provides cb, you should handle the socket's timeout.
