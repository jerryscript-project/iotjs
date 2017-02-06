IoT.js provides Buffer to compensate the lack of pure JavaScript's capability of manipulating binary data, JavaScript string is only suitable for Unicode string. Buffer allows you to handle sequence of binary data in JavaScript world.

## Class: Buffer
Buffer class is a global type. you can create a buffer object in several way.


### Constructor


#### new Buffer(size)
* `size: Number`

Allocates a new buffer of `size` bytes.


#### new Buffer(buffer)
* `buffer: Buffer`

Allocates a new buffer copied from given `buffer`.


#### new Buffer(str)
* `str: String`

Allocates a new buffer containing given `str`.


#### new Buffer(str[, encoding]))
* `str: String`
* `encoding: String`

Allocates a new buffer containing given `str`.
If provided, the encoding parameter identifies the character encoding of string.
It only supports `hex`.


#### new Buffer(array)
* `array: Array`

Allocates a new Buffer using an array of octets.


### Class Methods


#### Buffer.byteLength(string[, encoding])
* `string: String`
* `encoding: String`
* Return: `Number`

Returns actual byte length of `string` encoded by `encoding`. UTF-8 encoding is by default.
Note that it may be different from `String.prototype.length` since that returns the number of characters in a string.


#### Buffer.concat(list)
* `list: Array of Buffer`
* Return: `Buffer`

Returns a `Buffer` concatenating all the Buffers in the `list`.


#### Buffer.isBuffer(obj)
* `obj`: Object

Test if `obj` is a `Buffer`.


### Properties


#### buf.length
* `Number`

The capacity of buffer in bytes.
Note that the value of `length` is not necessarily the same to the size of contents.


### Methods


#### buf.compare(otherBuffer)
* `otherBuffer: Buffer`
* Return: `Number`


#### buf.copy(targetBuffer[,targetStart[,sourceStart[,sourceEnd]]])
* `targetBuffer: Buffer`
* `targetStart: Number`, Default: `0`
* `sourceStart: Number`, Default: `0`
* `sourceEnd: Number`, Default: `buf.length`

Returns a number indicating which buffer comes first between `this` and `otherBuffer` when lexicographically sorted.


#### buf.equals(otherBuffer)
* `otherBuffer: Buffer`
* Return: `Bool`

Returns whether `this` and `otherBuffer` have the same bytes.


#### buf.slice([start[,end]])
* `start: Number`, Default: `0
* `end: Number`, Default: `buf.length`
* Return: `Buffer`

Returns new buffer containing the same bytes of original buffer cropped by the given indices.

Copies data from `buf[sourceStart..sourceEnd-1]` to `targetBuffer[targetStart..]`.


#### buf.toString([,start[,end]])
* `start: Number`, Default: `0`
* `end: Number`, Default: `buffer.length`
* Return: `String`

Returns a string from buffer.


#### buf.write(string[,offset[,length]])
* `string: String` - data to be written to buffer
* `offset: Number`, Default: `0` - start position of buffer for writing.
* `length: Number`, Default: `buffer.length - offset` - total length to be written.
* Return: `Number` - total number of bytes written.

Writes `string` to buffer starting from `offset`. Returns total number of bytes written to the buffer.


#### buf.writeUInt8(value, offset[, noAssert])
* `value: Number` - number to be written to buffer
* `offset: Number`, - start position of buffer for writing.
* `noAssert: Boolean`, Default: false - skip value and offset validation.
* Return: `Number` offset plus the number of bytes written.

Writes `value` to buffer starting from `offset`. `value` should be a valid unsigned 8-bit integer.


#### buf.writeUInt16LE(value, offset[, noAssert])
* `value: Number` - number to be written to buffer
* `offset: Number`, - start position of buffer for writing.
* `noAssert: Boolean`, Default: false - skip value and offset validation.
* Return: `Number` offset plus the number of bytes written.

Writes `value` to buffer starting from `offset` writes little endian format. `value` should be a valid unsigned 16-bit integer.


#### buf.writeUInt32LE(value, offset[, noAssert])
* `value: Number` - number to be written to buffer
* `offset: Number` - start position of buffer for writing.
* `noAssert: Boolean`, Default: false - skip value and offset validation.
* Return: `Number` offset plus the number of bytes written.

Writes `value` to buffer starting from `offset` writes little endian format. `value` should be a valid unsigned 32-bit integer.


#### buf.readUInt8(offset[, noAssert])
* `offset: Number` - start position of buffer for reading.
* `noAssert: Boolean`, Default: false - skip offset validation.
* Return: `Number`

Reads an unsigned 8-bit integer from buf at the specified offset.


#### buf.readInt8(offset[, noAssert])
* `offset: Number` - start position of buffer for reading.
* `noAssert: Boolean`, Default: false - skip offset validation.
* Return: `Number`

Reads an signed 8-bit integer from buf at the specified offset.


#### buf.readUInt16LE(offset[, noAssert])
* `offset: Number` - start position of buffer for reading.
* `noAssert: Boolean`, Default: false - skip offset validation.
* Return: `Number`

Reads an unsigned 16-bit integer from buf at the specified offset with little endian format.
