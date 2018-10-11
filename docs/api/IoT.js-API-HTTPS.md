### Platform Support

 The following shows Https module APIs available for each platform.

 |  | Linux<br/>(Ubuntu) | Tizen<br/>(Raspberry Pi) | Raspbian<br/>(Raspberry Pi) | NuttX<br/>(STM32F4-Discovery) | TizenRT<br/>(Artik053) |
 | :---: | :---: | :---: | :---: | :---: | :---: |
 | https.createServer | O | O | O | △ ¹ | △ ¹ |
 | https.request | O | O | O | △ ¹ | △ ¹ |
 | https.get | O | O | O | △ ¹ | △ ¹ |

1. On NuttX/STM32F4-Discovery and TizenRT/Artik053, even a couple of sockets/server/requests might not work properly.


# Https

IoT.js provides HTTPS to support HTTPS clients enabling users to send HTTPS requests easily.

### https.createServer([options][, requestListener])
* `options` {Object} Accepts the same `options` as [tls.createServer](IoT.js-API-TLS.md#tlscreateserveroptions-secureconnectionlistener) and [http.createServer](IoT.js-API-HTTP.md#httpcreateserverrequestlistener) methods.
* `requestListener` {Function}
  * request {http.IncomingMessage}
  * response {http.ServerResponse}
* Returns: {https.Server}

To create a server the certificates should be specified via the `options` object.

The `requestListener` is a function which is automatically added to the `'request'` event.

**Example**

```js
var options = {
  key: fs.readFileSync('server.key'),
  cert: fs.readFileSync('server.cert')
};
var server = https.createServer(options, function(request, response) {
  ...
});
```


### https.request(options[, callback])
* `options` {Object}
  * `host` {string} A domain name or IP address of the server to issue the request to. **Default:** 'localhost'.
  * `hostname` {string} Alias for host.
  * `port` {number} Port of remote server. **Default:** 443.
  * `method` {string} A string specifying the HTTPS request method. **Default:** 'GET'.
  * `path` {string} Request path. **Default:** '/'. Should include query string if any. E.G. '/index.html?page=12'. An exception is thrown when the request path contains illegal characters. Currently, only spaces are rejected but that may change in the future.
  * `headers` {Object} An object containing request headers.
  * `auth` {string} Optional Basic Authentication in the form `username:password`. Used to compute HTTPS Basic Authentication header.
  * `ca` {string} Optional file path to CA certificate. Allows to override system trusted CA certificates.
  * `cert` {string} Optional file path to client authentication certificate in PEM format.
  * `key` {string} Optional file path to private keys for client cert in PEM format.
  * `rejectUnauthorized` {boolean} Optional Specify whether to verify the Server's certificate against CA certificates. WARNING - Making this `false` may be a security risk. **Default:** `true`
* `callback` {Function}
  * `response` {http.IncomingMessage}
* Returns: {http.ClientRequest}

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
  * `port` {number} Port of remote server. **Default:** 443.
  * `method` {string} A string specifying the HTTPS request method. **Default:** 'GET'.
  * `path` {string} Request path. **Default:** '/'. Should include query string if any. E.G. '/index.html?page=12'. An exception is thrown when the request path contains illegal characters. Currently, only spaces are rejected but that may change in the future.
  * `headers` {Object} An object containing request headers.
  * `auth` {string} Optional Basic Authentication in the form `username:password`. Used to compute HTTPS Basic Authentication header.
  * `ca` {string} Optional file path to CA certificate. Allows to override system trusted CA certificates.
  * `cert` {string} Optional file path to client authentication certificate in PEM format.
  * `key` {string} Optional file path to private keys for client cert in PEM format.
  * `rejectUnauthorized` {boolean} Optional Specify whether to verify the Server's certificate against CA certificates. WARNING - Making this `false` may be a security risk. **Default:** `true`
* `callback` {Function}
  * `response` {http.IncomingMessage}
* Returns: {http.ClientRequest}

Same as `https.request` except that `https.get` automatically call `req.end()` at the end.

Example:
```javascript
var https = require('https');

https.get({
  port: 443,
}, function(res) {
...
});
```
