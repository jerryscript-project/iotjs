### Platform Support

The following shows stream module APIs available for each platform.

|  | Linux<br/>(Ubuntu) | Raspbian<br/>(Raspberry Pi) | Nuttx<br/>(STM32F4-Discovery) |
| :---: | :---: | :---: | :---: |
| readable.isPaused | O | O | O |
| readable.pause | O | O | O |
| readable.read | O | O | O |
| readable.resume | O | O | O |
| writable.end | O | O | O |
| writable.write | O | O | O |

### Contents

- [Stream](#stream)
- [Class: Stream](#class-stream)
    - [Constructor](#constructor)
        - [`new Stream()`](#new-stream)
- [Class: Stream.Readable](#class-streamreadable)
    - [Readable Constructor](#readable-constructor)
        - [`new Stream.Readable()`](#new-streamreadable)
    - [Prototype Functions](#prototype-functions)
        - [`readable.isPaused()`](#readableispaused)
        - [`readable.pause()`](#readablepause)
        - [`readable.read([size])`](#readablereadsize)
        - [`readable.resume()`](#readableresume)
        - [`readable.push(chunk)`](#readablepushchunk)
    - [Events](#events)
        - [`'close'`](#close)
        - [`'data'`](#data)
        - [`'end'`](#end)
        - [`'error'`](#error)
        - [`'readable'`](#readable)
- [Class: Stream.Writable](#class-streamwritable)
    - [Writable Constructor](#Writable-constructor)
        - [`new Stream.Writable([options])`](#new-streamWritableoptions)
    - [Prototype Functions](#prototype-functions-1)
        - [`writable.end([chunk][, callback])`](#writableendchunk-callback)
        - [`writable.write(chunk[, callback])`](#writablewritechunk-callback)
        - [`writable._readyToWrite()`](#writable_readytowrite)
        - [`writable._write(chunk, callback, onwrite)`](#writable_writechunk-callback-onwrite)
    - [Events](#events-1)
        - [`'drain'`](#drain)
        - [`'error'`](#error-1)
        - [`'finish'`](#finish)
- [Class: Stream.Duplex](#class-streamduplex)

# Stream

The stream module provides a base API that makes it easy to build objects that implement the stream interface.

# Class: Stream

A stream is an abstract interface for working with streaming data in IoT.js.

## Constructor

### `new Stream()`

Returns a new Stream object.

# Class: Stream.Readable

Readable stream is abstraction for data source from which data will be read. At any time a readable stream is in one of two state: **flowing** and **paused**. In paused state, readable stream emits `'readable'` event for you that indicates there are data ready for read. And then, you can explicitly call `stream.read()` to get the data form the stream. In flowing state, readable stream emits `'data'` event with actual data that you can receive data as fast as possible. All streams start out in paused mode.

## Readable Constructor

### `new Stream.Readable()`

**This method is only for implementing a readable stream.**
Returns a new Readable Stream object.

## Prototype Functions

### `readable.isPaused()`
* Returns: `<Boolean>`

Returns `true` if the readable stream is in paused mode.

### `readable.pause()`
* Returns: `<Readable>`

Pauses emitting `data` events if the stream is in flowing mode.

### `readable.read([size])`
* `size <Number>` - Specify how much data will be read.
* Returns: `<String> | <Buffer> | <Null>`

The readable.read() method pulls some data out of the internal buffer and returns it. If no data available to be read, null is returned. By default, the data will be returned as a Buffer object.

### `readable.resume()`
* Returns: `<Readable>`

The readable.resume() method causes an explicitly paused Readable stream to resume emitting 'data' events, switching the stream into flowing mode.

### `readable.push(chunk)`
* `chunk <Buffer> | <String>`

**This method is only for implementing a readable stream.**
Use this method to push chunk of data to the underlying Buffer of this stream.
This can then be read by a consumer using either `readable.read` or `'data'` event of this stream.

## Events

### `'close'`
* `callback <Function()>`

Emitted when the underlying resource has been closed.

### `'data'`
* `callback <Function(chunk)>`
* `chunk <Buffer> | <String>`

Readable stream emits this event when a chunk of data prepared to be read. This event is only emitted when the stream is in flowing state. Attaching a `data` event listener to a stream makes the stream to be switched to flowing mode.

### `'end'`
* `callback <Function()>`

This event is only fired when there will be no more data to read.

### `'error'`
* `callback Function(err)`
* `err <Error>`

Emitted if there were something wrong reading data.

### `'readable'`
* `callback <Function()>`

The 'readable' event is emitted when there is data available to be read from the stream. This event will also be emitted once the end of the stream data has been reached but before the 'end' event is emitted.

# Class: Stream.Writable

Writable stream is abstraction for target that you can write data to.

## Writable Constructor

### `new Stream.Writable([options])`
* `options <Object>`

**This method is only for implementing a Writable stream.**
**Some functions of the class Stream.Writable are abstract interfaces. Concrete implementations of this class MUST call the above constructor and MUST implement the method `writable._write()`**
Returns a new Writable Stream object. Set `options.highWaterMark` for the number of characters after which `write()` starts returning false.

## Prototype Functions

### `writable.end([chunk][, callback])`
* `chunk <String> | <Buffer>` - The data to write
* `callback <Function()>` - Callback for for when the stream is finished.

### `writable.write(chunk[, callback])`
* `chunk <String> | <Buffer>` - The data to write
* `callback <Function()>` - Callback for when the chunk of data is flushed.
* Returns: `<Boolean>`

This method writes `chunk` of data to the underlying system, when the data is flush it calls back the `callback` function.
If you can write right after calling this method, it will return `true`, otherwise, return `false`.

### `writable._readyToWrite()`
**This method is only for implementing a Writable stream.**
**Concrete implementations of this class MUST call the above method.**
Concrete stream implementations should call this method to inform the stream when it is ready to be written to.

### `writable._write(chunk, callback, onwrite)`
* `chunk <String> | <Buffer>` - The data to be written by the stream
* `callback <Function()>` - Callback to be called when the chunk of data is flushed.
* `onwrite <Function()>` - Internal Callback to be called for when the chunk of data is flushed.

**This method is only for implementing a Writable stream.**
**This function of the class Stream.Writable is an abstract interface. Concrete implementations of this class MUST implement the above method.**
Concrete stream implementations should override this method. This method will be called when the stream neeeds to write a `chunk` of data. After the data is flushed, call `callback` and `onwrite` callbacks. This function will only be called after `writable._readyToWrite()` is called.

## Events

### `'drain'`
* `callback <Function()>`

If `writable.write()` returns false, `drain` events will indicate you that the stream is ready to be written.

### `'error'`
* `callback Function(err)`
* `err <Error>`

Emitted if there were something wrong writing data.

### `'finish'`
* `callback <Function()>`

After `writable.end()` has been called, and all data has been flushed, this event will be fired.


# Class: Stream.Duplex
Duplex streams are streams that implement both the Readable and Writable interfaces.
