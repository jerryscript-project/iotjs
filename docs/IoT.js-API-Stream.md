## Class: Stream.Readable

Readable stream is abstraction for data source from which data will be read.

At any time a readable stream is in one of two state: **flowing** and **paused**. In paused state, readable stream emits `'readable'` event for you that indicates there are data ready for read. and then you can explicitly call `stream.read()` to get the data form the stream. In flowing state, readable stream emits `'data'` event with actual data that you can receive data as fast as possible. All streams start out in paused mode.

### Events


#### 'close'
* `callback: Function()`

Emitted when the underlying resource has been closed.


#### 'data'
* `callback: Function(chunk)`
 * `chunk: Buffer | String`

Readable stream emits this event when a chunk of data prepared to be read.
This event only emitted when the stream is in flowing state.

Attaching a `data` event listener to a stream makes the stream to be switched to flowing mode.


#### 'end'
* `callback: Function()`

This event is only fired when there will be no more data to read.


#### 'readable'
* `callback: Function()`

Readable stream emits this event when a chunk of data prepared to be read.
This event only emitted when the stream is in paused state.


### Methods


#### readable.isPaused()
Returns `true` if the readable stream is in paused mode.


#### readable.pause()
Pause emitting `data` events if the stream was in flowing mode


#### readable.read([size])
* `size | Number` - specifies how much data will be read

Reads data from readable stream, and returns the data as `String | Buffer`


#### readable.resume()
Resume emitting `data` events if the readable was paused.


***

## Class: stream.Writable

Writable stream is abstraction for target that you can write data to.


### Events

#### 'drain'
* `callback: Function()`

If `writable.write()` returns false, `drain` events will indicate you that the stream is ready to be written.


#### 'error'
* `callback: Function(err)`
 * `err: Error`

Emitted if there were something wrong writing data.


#### 'finish'
* `callback: Function()`

After `writable.end()` has been called, and all data has been flushed, this event will be fired.


### Methods


#### writable.end([chunk][, callback])
* `chunk: String | Buffer` - The data to write
* `callback: Function()` - Function callback for when the chunk of data is flushed.


#### writable.write(chunk[, callback])
* `chunk: String | Buffer` - The data to write
* `callback: Function()` - Function callback for when the chunk of data is flushed.
* Returns: `Boolean`

This method writes `chunk` of data to the underlying system, when the data is flush it calls back the `callback` function.
If you can write right after calling this method, it will return `true`, otherwise, return `false`.

***

## Class: Stream.Duplex
Duplex streams are streams that implement both the Readable and Writable interfaces.
