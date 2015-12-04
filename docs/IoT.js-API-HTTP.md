# Module: http

IoT.js provides HTTP to support HTTP server and client enabling users to receive/send HTTP request easily.

## Methods
### http.createServer(requestListener)
* `requestListener: Function`
* Return: `http.Server` instance


### http.request(options[, callback])
* `options: Object`
* `callback: Function`
* Return: `http.ClientRequest` instance

### http.get(options[, callback])
* `options: Object`
* `callback: Function`
* Return: `http.ClientRequest` instance

Same as http.request except that `http.get` automatically call `req.end()` at the end.


## Class: http.Server

### Event

#### 'request'
* `request: http.IncomingMessage` instance.
* `response: http.ServerResponse` instance.
After request header is parsed, this event will be fired.

#### 'connection'
This event is emitted when new TCP connection is established.

#### 'close'
This event is emitted when server is closed.

### Method

#### listen(port[, hostname][, backlog][, callback])
* `port: Number`
* `host: String`
* `backlog: Number`
* `callback: Function`

Wait for new TCP connection with specified port and hostname. If no hostname is provided, server accepts any IP address.
```backlog``` is maximum pending connections. Default backlog length is 511.
```callback``` will be called when server has been bound.

#### close([callback])
* `callback: Function`

Stop accecpting new connection to this server. However, the existing connections is preserved. When server is finally closed after all connection closed, callback is called.

#### setTimeout(ms, cb)

* `ms: Number`
* `cb: Function`

Register cb for 'timeout' event and set socket's timeout value to ms. This event will be triggered by the underlying socket's 'timeout' event.

If cb is not provided, the socket will be destroyed automatically after timeout.
If you provides cb, you should handle the socket's timeout.

Default timeout for server is 2 minutes.

### Member Variable
#### timeout
Server's timeout value. Default value is 2 minutes.


## Class: http.ClientRequest
### Event

#### 'response'
* `response: http.IncomingMessage` instance

This event is emitted when server's response header is parsed. ` http.IncomingMessage` object is passed as argument to handler.


#### 'socket'
* `Net.socket` instance

This event is emitted when a socket is assigned to this request. `Net.socket` object is passed as argument to handler.

After response header is parsed, this event will be fired.

### Method

#### write(data[, callback])
* `data: Buffer|String`
* `callback: Function`

Sends `data` as a request body. `callback` will be called when data is flushed.


#### end([data][, callback])
* `data: Buffer|String`
* `callback: Function`

Finishes sending the request.

If `data` is provided, it sends `data` first, and finishes.

If `callback` is specified, it is called when the request stream is finished.



#### setTimeout(ms, cb)

* `ms: Number`
* `cb: Function`

Registers cb for 'timeout' event and set socket's timeout value to ms. This event will be triggered by the underlying socket's 'timeout' event.

If cb is not provided, the socket will be destroyed automatically after timeout.
If you provides cb, you should handle the socket's timeout.


## Class: http.ServerResponse
### Event
#### 'end'
Thit event is fired when no more data to be sent.

#### 'close'
When underlying connection is closed, 'close' event is emitted.

#### 'finish'
Thit event is emitted when the response has been sent. It does not guarantee that client has received data yet.

### Method
#### writeHead(statusCode[, statusMessage][, headers])
* `statusCode: Number`
* `statusMessage: String`
* `headers: Object`

Sets response's header. `headers` is a map between field and value in header.

#### setHeader(name, value)
* `name: String`
* `value: `

Sets response's header field(`name`) to `value`. If the field exists, it overwrites the existing value.

#### getHeader(name)
* `name: String`

Returns `name` field of response's header

#### removeHeader(name)
* `name: String`

Removes `name` field from response's header

#### write(data[, callback])
* `data: Buffer|String`
* `callback: Function`

Sends `data` as a response body. `callback` will be called when data is flushed.

#### end([data][, callback])
* `data: Buffer|String`
* `callback: Function`

Finishes sending the response.

If `data` is provided, it sends `data` first, and finishes.

If `callback` is specified, it is called when the response stream is finished.

#### setTimeout(ms, cb)

* `ms: Number`
* `cb: Function`

Register cb for 'timeout' event and set socket's timeout value to ms. This event will be triggered by the underlying socket's 'timeout' event.



## Class: http.IncomingMessage
### Event
#### 'end'
This event is fired when no more data to be received.

#### 'close'
When underlying connection is closed, 'close' event is emitted.

### Method
#### setTimeout(ms, cb)

* `ms: Number`
* `cb: Function`

Register cb for 'timeout' event set socket's timeout value to ms. This event will be triggered by the underlying socket's 'timeout' event.



### Member Varible
#### headers
HTTP header object.

#### method
Request method as `String`

#### url
Request URL as `String`

#### statusCode
HTTP response status code as `Number` of 3-digit.

#### statusMessage
HTTP response status message as `String`

#### socket
Underlying socket
