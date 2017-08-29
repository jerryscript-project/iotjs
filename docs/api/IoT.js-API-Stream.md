### Platform Support

The following shows stream module APIs available for each platform.

|  | Linux<br/>(Ubuntu) | Raspbian<br/>(Raspberry Pi) | NuttX<br/>(STM32F4-Discovery) | TizenRT<br/>(Artik053) |
| :---: | :---: | :---: | :---: | :---: |
| readable.isPaused | O | O | O | - |
| readable.pause | O | O | O | - |
| readable.read | O | O | O | - |
| readable.resume | O | O | O | - |
| writable.end | O | O | O | - |
| writable.write | O | O | O | - |

# Stream

A stream is an abstract interface for working with streaming data.
Streams can be readable, writable, or both (duplex). All streams
are instances of EventEmitter.

## Class: Stream

### new Stream()

**This method is only for implementing a new stream type.**

This function registers the common properties of all streams.

**Example**

```js
var Stream = require('stream').Stream;

var stream = new Stream();

stream.read = function () {
  // A custom read function.
}
```


## Class: Stream.Readable

Readable stream is an abstraction for a *data source* from
which data will be read. At any time a readable stream operates
in one of two modes: *flowing* and *paused*. All streams begin
in paused mode.

In paused mode the stream emits [`'readable'`](#event-readable)
event every time a new data is available for read. This data can
be read by calling the [`readable.read()`](#readablereadsize)
function. A stream can be switched to paused mode by calling
the [`readable.pause()`](#readablepause) function.

In flowing mode the stream emits [`'data'`](#event-data) events
every time when data is received. The data buffer is passed to
the callback function. A stream can be switched to flowing mode
by calling the [`readable.resume()`](#readableresume) function
or by adding a [`'data'`](#event-data) event handler.

Supported events:

### Event: 'close'

Emitted when the underlying resource has been closed and no more
events will be emitted. Not all [`Readable`](#class-streamreadable)
streams will emit the `'close'` event.


### Event: 'data'
* `chunk` {Buffer|string}

Emitted when the stream passes the ownership of the data to a
consumer. Only streams in flowing mode emit this event.

A stream can be switched to flowing mode by calling the
[`readable.resume()`](#readableresume) function or by adding
a `'data'` event handler.

**Example**

```js
var Readable = require('stream').Readable;

var readable = new Readable();

readable.on('data', function (chunk) {
  // prints: data received: message
  console.log('data received: ' + chunk);
})

readable.push("message");
```


### Event: 'end'

Emitted when there is no more data to be consumed from the stream.

**Example**

```js
var Readable = require('stream').Readable;

var readable = new Readable();

readable.on('end', function () {
  // prints: no more data
  console.log('no more data');
})

readable.push(null);
```


### Event: 'error'
* `error` {Object}

Emitted when an error is detected by the readable stream. This event
may occure any time.

### Event: 'readable'

Emitted when there is data available to be read from the stream.
This event will also be emitted once the end of the stream data
has been reached but before the ['end'](#event-end) event is emitted.

**Example**

```js
var Readable = require('stream').Readable;

var readable = new Readable();

readable.on('readable', function () {
  // prints:.
  console.log('data received: ' + this.read());
})

readable.push('message');
```


### new Readable(options)
* `options` {Object}
  * `defaultEncoding` {string} **Default:** `utf8`

**This method is only for implementing a new
[`Readable`](#class-streamreadable) stream type.**

This function registers the common properties
of [`Readable`](#class-streamreadable) streams.

The `options` object allows passing configuration
arguments. The `Readable` constructor only supports
a few members of the `options` object but descendants
may supports more of them.

**Example**

```js
var Readable = require('stream').Readable;

var readable = new Readable({ defaultEncoding: 'utf8' });
```


### readable.isPaused()
* Returns: {boolean}

Returns `true` if the [`Readable`](#class-streamreadable)
stream is in paused mode. Otherwise the stream is in
flowing mode. By default the stream starts in paused mode.

**Example**

```js
var Readable = require('stream').Readable;

var readable = new Readable();

// prints: true
console.log(readable.isPaused());
```

### readable.pause()
* Returns: {Readable}

Stops emitting [`'data'`](#event-data) events if the
stream is in flowing mode and sets paused mode. No
effect otherwise.

**Example**

```js
var Readable = require('stream').Readable;

var readable = new Readable();

readable.pause();
```


### readable.read([size])
* `size` {number} Specify how much data will be read.
* Returns: {Buffer|null}

The `readable.read()` method pulls some data out of the
internal buffer and returns it. If no data is available
`null` is returned instead.

Note: currently all data must be read.

**Example**

```js
var Readable = require('stream').Readable;

var readable = new Readable();

// prints: null
console.log(readable.read());

readable.push('message');

// prints: message
console.log(readable.read());
```


### readable.resume()
* Returns: {Readable}

Starts emitting [`'data'`](#events-data) events if the
stream is in paused mode and sets flowing mode. No effect
otherwise.

**Example**

```js
var Readable = require('stream').Readable;

var readable = new Readable();

readable.resume();
```


### readable.push(chunk)
* `chunk` {Buffer|string}

**This method is only for implementing a new
[`Readable`](#class-streamreadable) stream type.**

Push a chunk of data to the underlying `Buffer` of
this stream. The data can be read by using
either [`readable.read()`](#readablereadsize) method
or [`'data'`](#event-data) event of this stream.

**Example**

```js
var Readable = require('stream').Readable;

var readable = new Readable();

readable.push('message');

// prints: message
console.log(readable.read());
```


# Class: Stream.Writable

Writable stream is an abstraction for a *destination*
to which data is written. Several functions of the
Stream.Writable class are abstract interfaces and
descendants can override them. The
[`new Stream.Writable()`](#new-streamwritableoptions)
constructor must be used for initializing a
writable stream and
[`writable._write()`](#writable_writechunk-callback-onwrite)
method are required to be implemented.


Supported events:

### Event: 'drain'

If [`writable.write()`](#writablewritechunk-callback)
returns `false` the stream emits a `drain` event when
it is appropriate to resume writing data into the stream.

**Example**

```js
var Writable = require('stream').Writable;

// Buffer is full after 1 byte of data.
var writable = new Writable({ highWaterMark:1 });

writable._write = function(chunk, callback, onwrite) {
  onwrite();
}

// Writes 1 byte data into the stream.
writable.write("X");

writable.on('drain', function() {
  // prints: can continue writing
  console.log('can continue writing');
});

writable._readyToWrite();
```


### Event: 'error'
* `err` {Error}

Emitted if an error occures during writing the data.


### Event: 'finish'

Emitted after [`writable.end()`](#writableendchunk-callback)
has been called and all pending data has been flushed.

**Example**

```js
var Writable = require('stream').Writable;

var writable = new Writable();

writable.on('finish', function() {
  // prints: end of writing
  console.log('end of writing');
});

writable.end();
```


### new Stream.Writable([options])
* `options` {Object}
  * `highWaterMark` {number}

**This method is only for implementing a new
[`Writable`](#class-streamwritable) stream type.**

This function registers the common properties
of [`Writable`](#class-streamwritable) streams.

When the size of the internal buffer reaches
`options.highWaterMark` the
[`writable.write()`](#writablewritechunk-callback)
method starts returning with `false` and the data
should be drained before further data is written.

**Example**

```js
var Writable = require('stream').Writable;

var writable = new Writable({ highWaterMark:256 });
```


### writable.end([chunk[, callback]])
* `chunk` {Buffer|string} Final data to write.
* `callback` {Function}

Calling this function signals that no more data will
be written to the Writable. The optional `chunk`
argument allows writing a final chunk of data.
If `chunk` is `null` no data is written.

The optional `callback` function is attached
as a listener for the ['finish'](#event-finish) event.

**Example**

```js
var Writable = require('stream').Writable;

var writable = new Writable();

writable.end();
```


### writable.write(chunk[, callback])
* `chunk` {Buffer|string} Data to write.
* `callback` {Function} Called when this chunk of data is flushed.
* Returns: {boolean}

Converts `chunk` into a sequence of bytes and writes this data
to the stream. An optional `callback` function is called when
the data is flushed.

The returned value is `true` if writing can be continued
right after calling this method. Otherwise the returned
value is `false` and no data should be written until the
[`'drain'`](#event-drain) event is received.

**Example**

```js
var Writable = require('stream').Writable;

var writable = new Writable();

writable.write("String");
```


### writable._readyToWrite()

**This method is only for implementing a new
[`Writable`](#class-streamwritable) stream type.**

This method informs the [`Writable`](#class-streamwritable)
stream that the implementation is ready for receiving data.

**Example**

```js
var Writable = require('stream').Writable;

var writable = new Writable();

writable._readyToWrite();
```


### writable._write(chunk, callback, onwrite)
* `chunk` {Buffer|string} The data to be written by the stream.
* `callback` {Function} Callback to be called when the chunk of data is flushed.
* `onwrite` {Function} Internal Callback to be called for when the chunk of data is flushed.

**This method is only for implementing a new
[`Writable`](#class-streamwritable) stream type.**

This internal method is called by the
[`Writable`](#class-streamwritable) stream and the implementation
**must** override it. The data to be written is passed as the
`chunk` argument. After the operation is completed, both
`callback` and `onwrite` function should be called.

**Example**

```js
var Writable = require('stream').Writable;

var writable = new Writable();

writable._write = function(chunk, callback, onwrite) {
  // prints: message
  console.log(chunk);

  onwrite();
  if (callback)
    callback();
}

writable._readyToWrite();

writable.write('message');
```


# Class: Stream.Duplex

Duplex streams are streams that implement both the
Readable and Writable interfaces.
