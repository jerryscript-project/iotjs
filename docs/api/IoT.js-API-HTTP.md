### Platform Support


<a id="markdown-contents" name="contents"></a>
### Contents

- [Http](#http)
    - [Module Functions](#module-functions)
        - [`http.createServer(requestListener)`](#httpcreateserverrequestlistener)
        - [`http.request(options[, callback])`](#httprequestoptions-callback)
        - [`http.get(options[, callback])`](#httpgetoptions-callback)
    - [Module Properties](#module-properties)
        - [`http.METHODS`](#httpmethods)
- [Class: http.Server](#class-httpserver)
    - [Properties](#properties)
        - [`timeout`](#timeout)
    - [Prototype Functions](#prototype-functions)
        - [`listen(port[, hostname][, backlog][, callback])`](#listenport-hostname-backlog-callback)
        - [`close([callback])`](#closecallback)
        - [`setTimeout(ms, cb)`](#settimeoutms-cb)
    - [Events](#events)
        - [`'request'`](#request)
        - [`'connection'`](#connection)
        - [`'close'`](#close)
- [Class: http.ClientRequest](#class-httpclientrequest)
    - [Events](#events-1)
        - [`'close'`](#close-3)
        - [`'error'`](#error)
        - [`'finish'`](#finish-1)
        - [`'response'`](#response)
        - [`'socket'`](#socket)
    - [Prototype Functions](#prototype-functions-1)
        - [`write(data[, callback])`](#writedata-callback)
        - [end([data][, callback])](#enddata-callback)
        - [`setTimeout(ms, cb)`](#settimeoutms-cb-1)
- [Class: http.ServerResponse](#class-httpserverresponse)
    - [Prototype Functions](#prototype-functions-2)
        - [`writeHead(statusCode[, statusMessage][, headers])`](#writeheadstatuscode-statusmessage-headers)
        - [`setHeader(name, value)`](#setheadername-value)
        - [`getHeader(name)`](#getheadername)
        - [`removeHeader(name)`](#removeheadername)
        - [`write(data[, callback])`](#writedata-callback-1)
        - [`end([data][, callback])`](#enddata-callback)
        - [`setTimeout(ms, cb)`](#settimeoutms-cb-2)
    - [Events](#events-2)
        - [`'end'`](#end)
        - [`'close'`](#close-1)
        - [`'finish'`](#finish)
- [Class: http.IncomingMessage](#class-httpincomingmessage)
    - [Properties](#properties-1)
        - [`headers`](#headers)
        - [`method`](#method)
        - [`url`](#url)
        - [`statusCode`](#statuscode)
        - [`statusMessage`](#statusmessage)
        - [`socket`](#socket)
    - [Prototype Functions](#prototype-functions-3)
        - [`setTimeout(ms, cb)`](#settimeoutms-cb-3)
    - [Events](#events-3)
        - [`'end'`](#end-1)
        - [`'close'`](#close-2)

<a id="markdown-http" name="http"></a>
# Http

IoT.js provides HTTP to support HTTP server and client enabling users to receive/send HTTP request easily.

<a id="markdown-module-functions" name="module-functions"></a>
## Module Functions
<a id="markdown-httpcreateserverrequestlistener" name="httpcreateserverrequestlistener"></a>
### `http.createServer(requestListener)`
* `requestListener <Function()>`
* Returns: `http.Server`


<a id="markdown-httprequestoptions-callback" name="httprequestoptions-callback"></a>
### `http.request(options[, callback])`
* `options <Object>`
* `callback <Function()>`
* Returns: `http.ClientRequest`

<a id="markdown-httpgetoptions-callback" name="httpgetoptions-callback"></a>
### `http.get(options[, callback])`
* `options <Object>`
* `callback <Function()>`
* Returns: `http.ClientRequest`

Same as http.request except that `http.get` automatically call `req.end()` at the end.

<a id="markdown-properties" name="module-properties"></a>
## Module Properties

<a id="markdown-httpmethods" name="httpmethods"></a>
### `http.METHODS`
A list of HTTP methods supported by the parser as `String` properties of an `Object`.

<a id="markdown-class-httpserver" name="class-httpserver"></a>
# Class: http.Server

<a id="markdown-properties" name="properties"></a>
## Properties

<a id="markdown-timeout" name="timeout"></a>
### `timeout`
Server's timeout value. Default value is 2 minutes.

<a id="markdown-prototype-functions" name="prototype-functions"></a>
## Prototype Functions

<a id="markdown-listenport-hostname-backlog-callback" name="listenport-hostname-backlog-callback"></a>
### `listen(port[, hostname][, backlog][, callback])`
* `port <Number>`
* `host <String>`
* `backlog <Number>`
* `callback <Function()>`

Wait for new TCP connection with specified port and hostname. If no hostname is provided, server accepts any IP address.
`backlog` is maximum pending connections. Default backlog length is 511.
`callback` will be called when server has been bound.

<a id="markdown-closecallback" name="closecallback"></a>
### `close([callback])`
* `callback <Function()>`

Stop accepting new connection to this server. However, the existing connections are preserved. When server is finally closed after all connections are closed, a callback is called.

<a id="markdown-settimeoutms-cb" name="settimeoutms-cb"></a>
### `setTimeout(ms, cb)`

* `ms <Number>`
* `cb <Function()>`

Registers cb for 'timeout' event and sets socket's timeout value to ms. This event will be triggered by the underlying socket's 'timeout' event.

If cb is not provided, the socket will be destroyed automatically after timeout.
If you provide cb, you should handle the socket's timeout.

Default timeout for server is 2 minutes.

<a id="markdown-events" name="events"></a>
## Events

<a id="markdown-request" name="request"></a>
### `'request'`
* `request <http.IncomingMessage>`
* `response <http.ServerResponse>`

After request header is parsed, this event will be fired.

<a id="markdown-connection" name="connection"></a>
### `'connection'`
This event is emitted when new TCP connection is established.

<a id="markdown-close" name="close"></a>
### `'close'`
This event is emitted when server is closed.



<a id="markdown-class-httpclientrequest" name="class-httpclientrequest"></a>
# Class: http.ClientRequest

<a id="markdown-events-1" name="events-1"></a>
## Events


<a id="markdown-close-3" name="close-3"></a>
### `'close'`

This event is fired when the underlying socket is closed.

<a id="markdown-error" name="error"></a>
### `'error'`
* `callback Function(err)`
* `err <Error>`

Emitted if something went wrong with making or parsing the request.

<a id="markdown-finish-1" name="finish-1"></a>
### `'finish'`

This event is emitted after all the data was sent, meaning header and body all finished sending.

<a id="markdown-response" name="response"></a>
### `'response'`
* `response <http.IncomingMessage>`

This event is emitted when server's response header is parsed. ` http.IncomingMessage` object is passed as argument to handler.


<a id="markdown-socket" name="socket"></a>
### `'socket'`
* `socket <net.Socket>`

This event is emitted when a socket is assigned to this request. `net.Socket` object is passed as argument to handler.

After response header is parsed, this event will be fired.

<a id="markdown-prototype-functions-1" name="prototype-functions-1"></a>
## Prototype Functions

<a id="markdown-writedata-callback" name="writedata-callback"></a>
### `write(data[, callback])`
* `data <Buffer> | <String>`
* `callback <Function()>`

Sends `data` as a request body. `callback` will be called when data is flushed.


<a id="markdown-enddata-callback" name="enddata-callback"></a>
### end([data][, callback])
* `data <Buffer> | <String>`
* `callback <Function()>`

Finishes sending the request.

If `data` is provided, it sends `data` first, and finishes.
If `callback` is specified, it is called when the request stream is finished.


<a id="markdown-settimeoutms-cb-1" name="settimeoutms-cb-1"></a>
### `setTimeout(ms, cb)`

* `ms <Number>`
* `cb <Function()>`

Registers cb for 'timeout' event and set socket's timeout value to ms. This event will be triggered by the underlying socket's 'timeout' event.

If cb is not provided, the socket will be destroyed automatically after timeout.
If you provides cb, you should handle the socket's timeout.


<a id="markdown-class-httpserverresponse" name="class-httpserverresponse"></a>
# Class: http.ServerResponse

<a id="markdown-prototype-functions-2" name="prototype-functions-2"></a>
## Prototype Functions

<a id="markdown-writeheadstatuscode-statusmessage-headers" name="writeheadstatuscode-statusmessage-headers"></a>
### `writeHead(statusCode[, statusMessage][, headers])`
* `statusCode <Number>`
* `statusMessage <String>`
* `headers <Object>`

Sets response's header. `headers` is a map between field and value in header.

<a id="markdown-setheadername-value" name="setheadername-value"></a>
### `setHeader(name, value)`
* `name <String>`
* `value <String>`

Sets response's header field(`name`) to `value`. If the field exists, it overwrites the existing value.

<a id="markdown-getheadername" name="getheadername"></a>
### `getHeader(name)`
* `name <String>`

Returns `name` field of response's header

<a id="markdown-removeheadername" name="removeheadername"></a>
### `removeHeader(name)`
* `name <String>`

Removes `name` field from response's header

<a id="markdown-writedata-callback-1" name="writedata-callback-1"></a>
### `write(data[, callback])`
* `data <Buffer> | <String>`
* `callback <Function()>`

Sends `data` as a response body. `callback` will be called when data is flushed.

<a id="markdown-enddata-callback" name="enddata-callback"></a>
### `end([data][, callback])`
* `data <Buffer> | <String>`
* `callback <Function()>`

Finishes sending the response.

If `data` is provided, it sends `data` first, and finishes.
If `callback` is specified, it is called when the response stream is finished.

<a id="markdown-settimeoutms-cb-2" name="settimeoutms-cb-2"></a>
### `setTimeout(ms, cb)`

* `ms <Number>`
* `cb <Function()>`

Registers cb for 'timeout' event and set socket's timeout value to ms. This event will be triggered by the underlying socket's 'timeout' event.

<a id="markdown-events-2" name="events-2"></a>
## Events
<a id="markdown-end" name="end"></a>
### `'end'`
This event is fired when no more data to be sent.

<a id="markdown-close-1" name="close-1"></a>
### `'close'`
When underlying connection is closed, 'close' event is emitted.

<a id="markdown-finish" name="finish"></a>
### `'finish'`
This event is emitted when the response has been sent. It does not guarantee that client has received data yet.


<a id="markdown-class-httpincomingmessage" name="class-httpincomingmessage"></a>
# Class: http.IncomingMessage

http.IncomingMessage inherits [`Stream.readable`](IoT.js-API-Stream.md)

<a id="markdown-properties-1" name="properties-1"></a>
## Properties

<a id="markdown-headers" name="headers"></a>
### `headers`
HTTP header object.

<a id="markdown-method" name="method"></a>
### `method`
Requests method as `String`

<a id="markdown-url" name="url"></a>
### `url`
Requests URL as `String`

<a id="markdown-statuscode" name="statuscode"></a>
### `statusCode`
HTTP response status code as `Number` of 3-digit.

<a id="markdown-statusmessage" name="statusmessage"></a>
### `statusMessage`
HTTP response status message as `String`

<a id="markdown-socket" name="socket"></a>
### `socket`
Underlying socket

<a id="markdown-prototype-functions-3" name="prototype-functions-3"></a>
## Prototype Functions
<a id="markdown-settimeoutms-cb-3" name="settimeoutms-cb-3"></a>
### `setTimeout(ms, cb)`

* `ms <Number>`
* `cb <Function()>`

Registers cb for 'timeout' event set socket's timeout value to ms. This event will be triggered by the underlying socket's 'timeout' event.

<a id="markdown-events-3" name="events-3"></a>
## Events
<a id="markdown-end-1" name="end-1"></a>
### `'end'`
This event is fired when no more data to be received.

<a id="markdown-close-2" name="close-2"></a>
### `'close'`
When underlying connection is closed, 'close' event is emitted.
