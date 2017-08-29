### Platform Support

The following shows Buffer module APIs available for each platform.

|  | Linux<br/>(Ubuntu) | Raspbian<br/>(Raspberry Pi) | NuttX<br/>(STM32F4-Discovery) | TizenRT<br/>(Artik053) |
| :---: | :---: | :---: | :---: | :---: |
| buf.compare | O | O | O | - |
| buf.copy | O | O | O | - |
| buf.equals | O | O | O | - |
| buf.fill | O | O | O | - |
| buf.slice | O | O | O | - |
| buf.toString | O | O | O | - |
| buf.write | O | O | O | - |
| buf.writeUInt8 | O | O | O | - |
| buf.writeUInt16LE | O | O | O | - |
| buf.writeUInt32LE | O | O | O | - |
| buf.readInt8 | O | O | O | - |
| buf.readUInt8 | O | O | O | - |
| buf.readUInt16LE | O | O | O | - |


# Buffer

Buffer class is a global type with various constructors and accessors.

IoT.js provides Buffer to manipulate binary data. Currently buffer has
a pure ES5 compatible implementation, but this might be reworked to use
UInt8Array in the future.

**Example**

```js
var Buffer = require('buffer');

// Creates a zero-filled Buffer of length 10.
var buf1 = new Buffer(10);

// Creates a Buffer containing [0x1, 0x2, 0x3].
var buf2 = new Buffer([1, 2, 3]);

// Creates a Buffer containing UTF-8 bytes [0x74, 0xc3, 0xa9, 0x73, 0x74].
var buf3 = new Buffer('tést');
```

## Class: Buffer

### new Buffer(size)
* `size` {integer} Size of the new buffer.

Creates a new buffer of `size` bytes and initialize its data to zero.

**Example**

```js
var Buffer = require('buffer');

var buffer = new Buffer(5);
```

### new Buffer(buffer)
* `buffer` {Buffer} Source buffer.

Creates a copy of an existing buffer. The buffer data is not shared
between the two buffers.

**Example**

```js
var Buffer = require('buffer');

var buffer1 = new Buffer(5);
var buffer2 = new Buffer(buffer1);
```


### new Buffer(str[, encoding])
* `str` {string} Source string.
* `encoding` {string} Encoding format.

Creates a new buffer which contains the CESU-8 representation of
the `str` string argument. If `encoding` optional argument is
present its value must be `hex`. When this encoding is specified
the `str` argument must be a sequence of hexadecimal digit pairs,
and these pairs are converted to bytes.

**Example**

```js
var Buffer = require('buffer');

var buffer = new Buffer(String.fromCharCode(65));

// prints: 1
console.log(buffer);

var buffer = new Buffer(String.fromCharCode(128));

// prints: 2
console.log(buffer);

var buffer = new Buffer(String.fromCharCode(2048));

// prints: 3
console.log(buffer);

var buffer = new Buffer('4142', 'hex');

// prints: AB
console.log(buffer);
```


### new Buffer(array)
* `array` {Array} Array of numbers.

Creates a new Buffer from an array of numbers. The
numbers are converted to integers first and their modulo
256 remainder is used for constructing the buffer.

**Example**

```js
var buffer = new Buffer([65, 256 + 65, 65 - 256, 65.1]);

// prints: AAAA
console.log(buffer);
```


### Buffer.byteLength(str, encoding)

* `str` {string} Source string.
* `encoding` {string} String encoding.
* Returns: {integer} Byte length of source string.

Returns the byte length of a buffer representing the value
of the string argument encoded with encoding. The effect is
the same as:

```js
return new Buffer(str, encoding).length;
```

**Example**

```js
var Buffer = require('buffer');

// prints: 1
console.log(Buffer.byteLength(String.fromCharCode(65)));

// prints: 2
console.log(Buffer.byteLength(String.fromCharCode(128)));

// prints: 3
console.log(Buffer.byteLength(String.fromCharCode(2048)));

// prints: 2
console.log(Buffer.byteLength('4142', 'hex'));
```


### Buffer.concat(list)
* `list` {Array} An array of `Buffer` objects.
* Returns: {Buffer} Concatenated buffer.

Returns the concatenation of the `Buffer` objects
provided in the `list` array.

**Example**

```js
var Buffer = require('buffer');

var buffer = Buffer.concat([ new Buffer('He'),
                             new Buffer('llo'),
                             new Buffer(' wo'),
                             new Buffer('rld') ])

// prints: Hello world
console.log(buffer);
```


### Buffer.isBuffer(obj)
* `obj` {Object}
* Returns: {boolean}

Returns `true` if `obj` is an instance of `Buffer`. Returns
`false` otherwise.

**Example**

```js
var Buffer = require('buffer');

// prints: true
console.log(Buffer.isBuffer(new Buffer(1)));

// prints: false
console.log(Buffer.isBuffer('str'));
```


### buf.length
* {integer}

Returns the capacity of the buffer in bytes. Note: when
the buffer is converted to another type (e.g. String) the
length of the converted value might be different from
this value.

**Example**

```js
var Buffer = require('buffer');

var buffer = new Buffer([0xc8, 0x80]);

// prints: 2
console.log(buffer.length);

var str = buffer.toString();

// prints: 1
console.log(str.length);
```


### buf.compare(otherBuffer)
* `otherBuffer` {Buffer} The right-hand side of the comparison.
* Returns: {integer}

This function performs a lexicographic comparison between
two buffers.

It returns with `0` if the two buffers are the same. Otherwise
it returns with `-1` if the first different byte is lower for
`buf`, and `1` if the byte is higher. If the length of the two
buffers are different, the comparison is performed until the
lower length is reached. If all bytes are the same the function
returns with `-1` if `buf.length` is less than `otherBuffer.length`
and `1` otherwise.

**Example**

```js
var Buffer = require('buffer');

var buffer1 = new Buffer('AB');
var buffer2 = new Buffer('A');
var buffer3 = new Buffer('B');

// prints: 0
console.log(buffer1.compare(buffer1));

// prints: 1
console.log(buffer1.compare(buffer2));

// prints: -1
console.log(buffer1.compare(buffer3));
```


### buf.copy(targetBuffer[, targetStart[, sourceStart[, sourceEnd]]])
* `targetBuffer` {Buffer} The buffer to be modified.
* `targetStart` {Integer} **Default:** `0`
* `sourceStart` {integer} **Default:** `0`
* `sourceEnd` {integer} **Default:** `buf.length`
* Returns: {integer} The number of bytes copied.

Copy a sequence of bytes from `buf` buffer to `targetBuffer` buffer.
The source byte range is specified by `sourceStart` and `sourceEnd`
and the destination byte offset is specified by `targetStart`. Only
the `targetBuffer` is modified.

**Example**

```js
var Buffer = require('buffer');

var buffer1 = new Buffer('Hello XY world!');
var buffer2 = new Buffer('<JS>');

buffer2.copy(buffer1, 6, 1, 3);

// prints: Hello JS world!
console.log(buffer1);
```


### buf.equals(otherBuffer)
* `otherBuffer` {Buffer} The right-hand side of the comparison.
* Returns: {boolean}

Returns `true` if `this` and `otherBuffer` have exactly the
same bytes. Returns `false` otherwise. The effect is the same as:

```js
return buf.compare(otherBuffer) == 0;
```

**Example**

```js
var Buffer = require('buffer');

var buffer1 = new Buffer('AB');
var buffer2 = new Buffer('4142', 'hex');
var buffer3 = new Buffer('A');

// prints: true
console.log(buffer1.equals(buffer2));

// prints: false
console.log(buffer1.equals(buffer3));
```


### buf.fill(value)
* `value` {integer} All bytes are set to this value.
* Returns: {Buffer} The original buffer.

Set all bytes of the buffer to value. The value is converted to
integer first and its modulo 256 remainder is used for updating
the buffer. Returns with `buf`.

**Example**

```js
var Buffer = require('buffer');

var buffer = new Buffer('Hello');

buffer.fill(65);

// prints: AAAAA
console.log(buffer);

buffer.fill(66 - 256);

// prints: BBBBB
console.log(buffer);
```


### buf.slice([start[, end]])
* `start` {integer} **Default:** `0`
* `end` {integer} **Default:** `buf.length`
* Returns: {Buffer} A newly created buffer.

This function returns with a newly created buffer which
contains the bytes of the `buf` buffer between `start`
and `end`.

**Example**

```js
var Buffer = require('buffer');

var buffer = new Buffer('This is JavaScript!!!');

// prints: JavaScript
console.log(buffer.slice(8, 18));
```


### buf.toString([start[, end]])
* `start` {integer} **Default:** `0`
* `end` {integer} **Default:** `buffer.length`
* Returns: {string}

Returns a string created from the bytes stored in the buffer.
By passing `start` and `end` the conversion can be limited
to a subset of the `buf` buffer. If a single `hex` string is
passed to the function, the whole buffer is converted to
hexadecimal data.

**Example**

```js
var Buffer = require('buffer');

var buffer = new Buffer('DEFG');

// prints: EF
console.log(buffer.toString(1, 3));

// prints: 44454647
console.log(buffer.toString('hex'));
```


### buf.write(string[, offset[, length]])
* `string` {string} Data to be written into buffer.
* `offset` {integer} Start position of writing. **Default:** `0`
* `length` {integer} How many bytes to write. **Default:** `buffer.length - offset`.
* Returns: {integer} Total number of bytes written.

Writes `string` into the `buf` buffer. The start position of the
writing can be specified by `offset` and the maximum number of
updated bytes can be limited by `length`. Returns total number
of bytes written to the buffer.

**Example**

```js
var Buffer = require('buffer');

var buffer = new Buffer('......');
buffer.write('AB');
buffer.write('XY', 3);

// prints: AB.XY.
console.log(buffer);

var buffer = new Buffer('......');
buffer.write('ABCDEF', 1, 3);

// prints: .ABC..
console.log(buffer);
```


### buf.writeUInt8(value, offset[, noAssert])
* `value` {integer} Number to be written into the buffer.
* `offset` {integer} Start position of the writing.
* `noAssert` {boolean} Skip argument validation. **Default:** `false`
* Returns: {number} Offset plus the number of bytes written.

Writes `value` into the buffer starting from `offset` position.
The `value` must be a valid 8-bit unsigned integer.

If `noAssert` is set and the value is outside of the expected range
or the offset is higher than the size of the buffer the operation
is undefined.

**Example**

```js
var Buffer = require('buffer');

var buffer = new Buffer('....');

// prints: 3
console.log(buffer.writeUInt8(65, 2));

// prints: ..A.
console.log(buffer);
```


### buf.writeUInt16LE(value, offset[, noAssert])
* `value` {integer} Number to be written into the buffer.
* `offset` {integer} Start position of the writing.
* `noAssert` {boolean} Skip argument validation. **Default:** `false`
* Returns: {integer} Offset plus the number of bytes written.

Writes `value` into the buffer starting from `offset` position
with little endian format. The `value` must be a valid 16-bit
unsigned integer.

If `noAssert` is set and the value is outside of the expected range
or the offset is higher than the size of the buffer the operation
is undefined.

**Example**

```js
var Buffer = require('buffer');

var buffer = new Buffer('......');

// prints: 3
console.log(buffer.writeUInt16LE(0x4142, 1));

// prints .BA...
console.log(buffer);
```


### buf.writeUInt32LE(value, offset[, noAssert])
* `value` {integer} Number to be written into the buffer.
* `offset` {integer} Start position of the writing.
* `noAssert` {boolean} Skip argument validation. **Default:** `false`
* Returns: {integer} Offset plus the number of bytes written.

Writes `value` into the buffer starting from `offset` position
with little endian format. The `value` must be a valid 32-bit
unsigned integer.

If `noAssert` is set and the value is outside of the expected range
or the offset is higher than the size of the buffer the operation
is undefined.

**Example**

```js
var Buffer = require('buffer');

var buffer = new Buffer('......');

// prints: 5
console.log(buffer.writeUInt32LE(0x41424344, 1));

// prints: .DCBA.
console.log(buffer);
```


### buf.readInt8(offset[, noAssert])
* `offset` {number} Start position of buffer for reading.
* `noAssert` {boolean} Skip offset validation. **Default:** `false`
* Returns: {number}

Reads a signed 8-bit integer from `buf` buffer starting from
`offset` position.

If `noAssert` is set and the offset is higher than the size of
the buffer the result is undefined.

**Example**

```js
var Buffer = require('buffer');

var buffer = new Buffer('ABCDEF');

// prints: 42
console.log(buffer.readUInt8(1).toString(16));
```


### buf.readUInt8(offset[, noAssert])
* `offset` {integer} Start position of the reading.
* `noAssert` {boolean} Skip argument validation. **Default:** `false`
* Returns: {number}

Reads an unsigned 8-bit integer from `buf` buffer starting from
`offset` position.

If `noAssert` is set and the offset is higher than the size of
the buffer the result is undefined.

**Example**

```js
var Buffer = require('buffer');

var buffer = new Buffer('ABCDEF');

// prints: 42
console.log(buffer.readUInt8(1).toString(16));
```


### buf.readUInt16LE(offset[, noAssert])
* `offset` {number} Start position of buffer for reading.
* `noAssert` {boolean} Skip offset validation. **Default:** `false`
* Returns: {number}

Reads an unsigned 16-bit integer from `buf` buffer starting from
`offset` position with little endian format.

If `noAssert` is set and the offset is higher than the size of
the buffer the result is undefined.

**Example**

```js
var Buffer = require('buffer');

var buffer = new Buffer('ABCDEF');

// prints: 4342
console.log(buffer.readUInt16LE(1).toString(16));
```
