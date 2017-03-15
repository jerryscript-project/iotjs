## Contents

* [Buffer](#buffer)
  * [Constructor](#constructor)
    * [`new Buffer(size)`](#new-buffer-size)
    * [`new Buffer(buffer)`](#new-buffer-buffer)
    * [`new Buffer(str[, encoding]))`](#new-buffer-str)
    * [`new Buffer(array)`](#new-buffer-array)
  * [Class Methods](#class-methods)
    * [`Buffer.byteLength(str[, encoding])`](#buffer-byte-length)
    * [`Buffer.concat(list)`](#buffer-concat)
    * [`Buffer.isBuffer(obj)`](#buffer-isbuffer)
  * [Properties](#properties)
    * [`buf.length`](#buf-length)
  * [Prototype methods](#prototype-methods)
    * [`buf.compare(otherBuffer)`](#buf-compare)
    * [`buf.copy(targetBuffer[,targetStart[,sourceStart[,sourceEnd]]])`](#buf-copy)
    * [`buf.equals(otherBuffer)`](#buf-equals)
    * [`buf.fill(value)`](#buf-fill)
    * [`buf.slice([start[,end]])`](#buf-slice)
    * [`buf.toString([start[,end]])`](#buf-to-string)
    * [`buf.write(string[,offset[,length]])`](#buf-write)
    * [`buf.writeUInt8(value, offset[, noAssert])`](#buf-write-uint8)
    * [`buf.writeUInt16LE(value, offset[, noAssert])`](#buf-write-uint16le)
    * [`buf.writeUInt32LE(value, offset[, noAssert])`](#buf-write-uint32le)
    * [`buf.readInt8(offset[, noAssert])`](#buf-read-int8)
    * [`buf.readUInt8(offset[, noAssert])`](#buf-read-uint8)
    * [`buf.readUInt16LE(offset[, noAssert])`](#buf-read-uint16)


## Class: Buffer <a name="buffer"></a>

Buffer class is a global type with various constructors and accessors.

IoT.js provides Buffer to manipulate binary data. Currently buffer has
a pure ES5 compatible implementation, but this might be reworked to use
UInt8Array in the future.

## Constructor <a name="constructor"></a>


### `new Buffer(size)` <a name="new-buffer-size"></a>
* `size <Integer>`, size of the new buffer

Creates a new buffer of `size` bytes and initialize its data to zero.

**Example**

```js
var Buffer = require('buffer');

var buffer = new Buffer(5);
```

### `new Buffer(buffer)` <a name="new-buffer-buffer"></a>
* `buffer <Buffer>`, source buffer

Creates a copy of an existing buffer. The buffer data is not shared
between the two buffers.

**Example**

```js
var Buffer = require('buffer');

var buffer1 = new Buffer(5);
var buffer2 = new Buffer(buffer1);
```


### `new Buffer(str[, encoding]))` <a name="new-buffer-str"></a>
* `str <String>`, source string
* `encoding <String>`, encoding format

Creates a new buffer which contains the CESU-8 representation of
the `str` string argument. If `encoding` optional argument is
present its value must be `hex`. When this encoding is specified
the `str` argument must be a sequence of hexadecimal digit pairs,
and these pairs are converted to bytes.

**Example**

```js
var Buffer = require('buffer');

var buffer = new Buffer(String.fromCharCode(65))
console.log(buffer); // prints 1
var buffer = new Buffer(String.fromCharCode(128))
console.log(buffer); // prints 2
var buffer = new Buffer(String.fromCharCode(2048))
console.log(buffer); // prints 3

var buffer = new Buffer('4142', 'hex')
console.log(buffer); // prints AB
```


### `new Buffer(array)` <a name="new-buffer-array"></a>
* `array <Array>`, array of numbers

Creates a new Buffer from an array of numbers. The
numbers are converted to integers first and their modulo
256 remainder is used for constructing the buffer.

**Example**

```js
var buffer = new Buffer([65, 256 + 65, 65 - 256, 65.1])
console.log(buffer); // prints AAAA
```


## Class Methods <a name="class-methods"></a>


### `Buffer.byteLength(str[, encoding])` <a name="buffer-byte-length"></a>
* `str <String>`, source string
* `encoding <String>`, string encoding
* Returns: `<Integer>`, byte length of source string

Returns the byte length of a buffer representing the value
of the `string` argument encoded with `encoding`. The
effect is the same as:

```js
return new Buffer(str, encoding).length;
```

**Example**

```js
var Buffer = require('buffer');

// prints 1
console.log(Buffer.byteLength(String.fromCharCode(65)));

// prints 2
console.log(Buffer.byteLength(String.fromCharCode(128)));

// prints 3
console.log(Buffer.byteLength(String.fromCharCode(2048)));

// prints 2
console.log(Buffer.byteLength('4142', 'hex'));
```


### `Buffer.concat(list)` <a name="buffer-concat"></a>
* `list <Array>`, an array of `Buffer` objects
* Returns: `<Buffer>`, concatenated buffer

Returns the concatenation of the `Buffer` objects
provided in the `list` array.

**Example**

```js
var Buffer = require('buffer');

var buffer = Buffer.concat([ new Buffer('He'),
                             new Buffer('llo'),
                             new Buffer(' wo'),
                             new Buffer('rld') ])
// prints Hello world
console.log(buffer);
```


### `Buffer.isBuffer(obj)` <a name="buffer-isbuffer"></a>
* `obj <Object>`, a generic object
* Returns: `<Boolean>`

Returns `true` if `obj` is an instance of `Buffer`. Returns
`false` otherwise.

**Example**

```js
var Buffer = require('buffer');

console.log(Buffer.isBuffer(new Buffer(1))); // prints true
console.log(Buffer.isBuffer('str')); // prints false
```


## Properties <a name="properties"></a>


### `buf.length` <a name="buf-length"></a>
* `<Integer>`, length of the buffer

Returns the capacity of the buffer in bytes. Note: when
the buffer is converted to another type (e.g. String) the
length of the converted value might be different from
this value.

**Example**

```js
var Buffer = require('buffer');

var buffer = new Buffer([0xc8, 0x80])
console.log(buffer.length); // prints 2

var str = buffer.toString();
console.log(str.length); // prints 1
```


## Prototype methods <a name="prototype-methods"></a>


### `buf.compare(otherBuffer)` <a name="buf-compare"></a>
* `otherBuffer <Buffer>`, the right-hand side of the comparison
* Returns: `<Integer>`

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

console.log(buffer1.compare(buffer1)); // prints 0
console.log(buffer1.compare(buffer2)); // prints 1
console.log(buffer1.compare(buffer3)); // prints -1
```


### `buf.copy(targetBuffer[,targetStart[,sourceStart[,sourceEnd]]])` <a name="buf-copy"></a>
* `targetBuffer <Buffer>`, modified buffer
* `targetStart <Number>`, Default: `0`
* `sourceStart <Number>`, Default: `0`
* `sourceEnd <Number>`, Default: `buf.length`

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

console.log(buffer1); // prints Hello JS world!
```


### `buf.equals(otherBuffer)` <a name="buf-equals"></a>
* `otherBuffer <Buffer>`, the right-hand side of the comparison
* Return: `<Boolean>`

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

console.log(buffer1.equals(buffer2)); // prints true
console.log(buffer1.equals(buffer3)); // prints false
```


### `buf.fill(value)` <a name="buf-fill"></a>
* `value <Integer>`, all bytes are set to this value
* Return: `<Buffer>`, the original buffer

Set all bytes of the buffer to value. The value is converted to
integer first and its modulo 256 remainder is used for updating
the buffer. Returns with `buf`.

**Example**

```js
var Buffer = require('buffer');

var buffer = new Buffer('Hello');

buffer.fill(65);
console.log(buffer); // prints AAAAA
buffer.fill(66 - 256);
console.log(buffer); // prints BBBBB
```


### `buf.slice([start[,end]])` <a name="buf-slice"></a>
* `start <Integer>`, Default: `0`
* `end <Integer>`, Default: `buf.length`
* Return: `<Buffer>`, a newly created buffer

This function returns with a newly created buffer which
contains the bytes of the `buf` buffer between `start`
and `end`.

**Example**

```js
var Buffer = require('buffer');

var buffer = new Buffer('This is JavaScript!!!');

console.log(buffer.slice(8, 18)); // prints JavaScript
```


### `buf.toString([start[,end]])` <a name="buf-to-string"></a>
* `start <Integer>`, Default: `0`
* `end <Integer>`, Default: `buffer.length`
* Return: `<String>`

Returns a string created from the bytes stored in the buffer.
By passing `start` and `end` the conversion can be limited
to a subset of the `buf` buffer. If a single `hex` string is
passed to the function, the whole buffer is converted to
hexadecimal data.

**Example**

```js
var Buffer = require('buffer');

var buffer = new Buffer("DEFG");

console.log(buffer.toString(1, 3)); // prints EF
console.log(buffer.toString('hex')); // prints 44454647
```


### `buf.write(string[,offset[,length]])` <a name="buf-write"></a>
* `string <String>`, data to be written to buffer
* `offset <Integer>`, start position of buffer for writing. Default: `0`
* `length <Integer>`, maximum length to be written. Default: `buffer.length - offset`.
* Return: `<Integer>`, total number of bytes written.

Writes `string` into the `buf` buffer. The start position of the
writing can be specified by `offset` and the maximum number of
updated bytes can be limited by `length`. Returns total number
of bytes written to the buffer.

**Example**

```js
var Buffer = require('buffer');

var buffer = new Buffer("......");
buffer.write("AB");
buffer.write("XY", 3);

console.log(buffer); // prints AB.XY.

var buffer = new Buffer("......");
buffer.write("ABCDEF", 1, 3);

console.log(buffer); // prints .ABC..
```


### `buf.writeUInt8(value, offset[, noAssert])` <a name="buf-write-uint8"></a>
* `value <Integer>`, number to be written into the buffer
* `offset <Integer>`, start position of the writing
* `noAssert <Boolean>, skip argument validation. Default: `false`
* Return: `<Number>` offset plus the number of bytes written.

Writes `value` into the buffer starting from `offset` position.
The `value` must be a valid 8-bit unsigned integer.

**Example**

```js
var Buffer = require('buffer');

var buffer = new Buffer("....");

console.log(buffer.writeUInt8(65, 2)); // prints 3
console.log(buffer); // prints ..A.
```


### `buf.writeUInt16LE(value, offset[, noAssert])` <a name="buf-write-uint16le"></a>
* `value <Integer>`, number to be written into the buffer
* `offset <Integer>`, start position of the writing
* `noAssert <Boolean>`, skip argument validation. Default: `false`
* Return: `<Integer>` offset plus the number of bytes written

Writes `value` into the buffer starting from `offset` position
with little endian format. The `value` must be a valid 16-bit
unsigned integer.

**Example**

```js
var Buffer = require('buffer');

var buffer = new Buffer("......");

print(buffer.wrrteUInt16LE(0x4142, 1)); // prints 3
print(buffer); // prints .BA...
```


### `buf.writeUInt32LE(value, offset[, noAssert])` <a name="buf-write-uint32le"></a>
* `value <Integer>`, number to be written into the buffer
* `offset <Integer>`, start position of the writing
* `noAssert <Boolean>`, skip argument validation. Default: `false`
* Return: `<Integer>` offset plus the number of bytes written

Writes `value` into the buffer starting from `offset` position
with little endian format. The `value` must be a valid 32-bit
unsigned integer.

**Example**

```js
var Buffer = require('buffer');

var buffer = new Buffer("......");

print(buffer.wrrteUInt32LE(0x41424344, 1)); // prints 5
print(buffer); // prints .DCBA.
```


### `buf.readInt8(offset[, noAssert])` <a name="buf-read-int8"></a>
* `offset: Number` - start position of buffer for reading.
* `noAssert: Boolean`, Default: false - skip offset validation.
* Return: `Number`

Reads a signed 8-bit integer from `buf` buffer starting from
`offset` position.

**Example**

```js
var Buffer = require('buffer');

var buffer = new Buffer("ABCDEF");

print(buffer.readUInt8(1).toString(16)); // prints 42
```


### `buf.readUInt8(offset[, noAssert])` <a name="buf-read-uint8"></a>
* `offset <Integer>`, start position of the reading
* `noAssert <Boolean>`, skip argument validation. Default: `false`
* Return: `Number`

Reads an unsigned 8-bit integer from `buf` buffer starting from
`offset` position.

**Example**

```js
var Buffer = require('buffer');

var buffer = new Buffer("ABCDEF");

print(buffer.readUInt8(1).toString(16)); // prints 42
```


### `buf.readUInt16LE(offset[, noAssert])` <a name="buf-read-uint16"></a>
* `offset: Number` - start position of buffer for reading.
* `noAssert: Boolean`, Default: false - skip offset validation.
* Return: `Number`

Reads an unsigned 16-bit integer from `buf` buffer starting from
`offset` position with little endian format.

**Example**

```js
var Buffer = require('buffer');

var buffer = new Buffer("ABCDEF");

print(buffer.readUInt16LE(1).toString(16)); // prints 4342
```

