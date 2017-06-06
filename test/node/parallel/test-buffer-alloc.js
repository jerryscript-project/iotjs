/* Copyright 2017-present Samsung Electronics Co., Ltd. and other contributors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// Copyright Joyent, Inc. and other Node contributors.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to permit
// persons to whom the Software is furnished to do so, subject to the
// following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
// NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
// OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
// USE OR OTHER DEALINGS IN THE SOFTWARE.

'use strict';
var common = require('node/common');
var assert = require('assert');

var Buffer = require('buffer').Buffer;

var b = new Buffer(1024);
assert.strictEqual(1024, b.length);

var c = new Buffer(512);
assert.strictEqual(512, c.length);

var d = new Buffer([]);
assert.strictEqual(0, d.length);

// try to create 0-length buffers
assert.doesNotThrow(function() { return new Buffer('') });
assert.doesNotThrow(function() { return new Buffer('', 'ascii') });
assert.doesNotThrow(function() { return new Buffer('', 'latin1') });
assert.doesNotThrow(function() { return new Buffer('', 'binary') });
assert.doesNotThrow(function() { return Buffer(0) });

// try to write a 0-length string beyond the end of b
//assert.throws(function() { return b.write('', 2048) }, RangeError);

// throw when writing to negative offset
assert.throws(function() { return b.write('a', -1) }, RangeError);

// throw when writing past bounds from the pool
assert.throws(function() { return b.write('a', 2048) }, RangeError);

// throw when writing to negative offset
assert.throws(function() { return b.write('a', -1) }, RangeError);

// try to copy 0 bytes worth of data into an empty buffer
b.copy(new Buffer(0), 0, 0, 0);

// try to copy 0 bytes past the end of the target buffer
b.copy(new Buffer(0), 1, 1, 1);
b.copy(new Buffer(1), 1, 1, 1);

// try to copy 0 bytes from past the end of the source buffer
b.copy(new Buffer(1), 0, 2048, 2048);

// Offset points to the end of the buffer
// (see https://github.com/nodejs/node/issues/8127).
// assert.doesNotThrow(function() { return (new Buffer(1)).write('', 1, 0) });

{
  var asciiString = 'hello world';
  var offset = 100;

  var sliceA = b.slice(offset, offset + asciiString.length);
  var sliceB = b.slice(offset, offset + asciiString.length);
  for (var i = 0; i < asciiString.length; i++) {
    assert.strictEqual(sliceA[i], sliceB[i]);
  }
}

{
  var slice = b.slice(100, 150);
  assert.strictEqual(50, slice.length);
  for (var i = 0; i < 50; i++) {
    assert.strictEqual(b[100 + i], slice[i]);
  }
}

{
  // make sure only top level parent propagates from allocPool
  var b = new Buffer(5);
  var c = b.slice(0, 4);
  var d = c.slice(0, 2);
  assert.strictEqual(b.parent, c.parent);
  assert.strictEqual(b.parent, d.parent);
}

{
  // also from a non-pooled instance
  var b = new Buffer(5);
  var c = b.slice(0, 4);
  var d = c.slice(0, 2);
  assert.strictEqual(c.parent, d.parent);
}

{
  // test hex toString
  var hexb = new Buffer(256);
  for (var i = 0; i < 256; i++) {
    hexb.writeUInt8(i, i);
  }
  var hexStr = hexb.toString('hex');
  assert.strictEqual(hexStr,
                     '000102030405060708090a0b0c0d0e0f' +
                     '101112131415161718191a1b1c1d1e1f' +
                     '202122232425262728292a2b2c2d2e2f' +
                     '303132333435363738393a3b3c3d3e3f' +
                     '404142434445464748494a4b4c4d4e4f' +
                     '505152535455565758595a5b5c5d5e5f' +
                     '606162636465666768696a6b6c6d6e6f' +
                     '707172737475767778797a7b7c7d7e7f' +
                     '808182838485868788898a8b8c8d8e8f' +
                     '909192939495969798999a9b9c9d9e9f' +
                     'a0a1a2a3a4a5a6a7a8a9aaabacadaeaf' +
                     'b0b1b2b3b4b5b6b7b8b9babbbcbdbebf' +
                     'c0c1c2c3c4c5c6c7c8c9cacbcccdcecf' +
                     'd0d1d2d3d4d5d6d7d8d9dadbdcdddedf' +
                     'e0e1e2e3e4e5e6e7e8e9eaebecedeeef' +
                     'f0f1f2f3f4f5f6f7f8f9fafbfcfdfeff');

  var hexb2 = new Buffer(hexStr, 'hex');
  for (var i = 0; i < 256; i++) {
    assert.strictEqual(hexb2.readUInt8(i), hexb.readUInt8(i));
  }
}

// Test single hex character throws TypeError
// - https://github.com/nodejs/node/issues/6770
//assert.throws(function() { return new Buffer('A', 'hex') }, TypeError);

function buildBuffer(data) {
  if (Array.isArray(data)) {
    var buffer = new Buffer(data.length);
    for (var i = 0; i < data.length; i++)
      buffer.writeUInt8(data[i], i);
    return buffer;
  }
  return null;
}

var x = buildBuffer([0x81, 0xa3, 0x66, 0x6f, 0x6f, 0xa3, 0x62, 0x61, 0x72]);

{
  var z = x.slice(4);
  assert.strictEqual(5, z.length);
  assert.strictEqual(0x6f, z.readUInt8(0));
  assert.strictEqual(0xa3, z.readUInt8(1));
  assert.strictEqual(0x62, z.readUInt8(2));
  assert.strictEqual(0x61, z.readUInt8(3));
  assert.strictEqual(0x72, z.readUInt8(4));
}

{
  var z = x.slice(0);
  assert.strictEqual(z.length, x.length);
}

{
  var z = x.slice(0, 4);
  assert.strictEqual(4, z.length);
  assert.strictEqual(0x81, z.readUInt8(0));
  assert.strictEqual(0xa3, z.readUInt8(1));
}

{
  var z = x.slice(0, 9);
  assert.strictEqual(9, z.length);
}

{
  var z = x.slice(1, 4);
  assert.strictEqual(3, z.length);
  assert.strictEqual(0xa3, z.readUInt8(0));
}

{
  var z = x.slice(2, 4);
  assert.strictEqual(2, z.length);
  assert.strictEqual(0x66, z.readUInt8(0));
  assert.strictEqual(0x6f, z.readUInt8(1));
}

{
  // #1210 Test UTF-8 string includes null character
  var buf = new Buffer('\x00');
  assert.strictEqual(buf.length, 1);
  buf = new Buffer('\x00\x00');
  assert.strictEqual(buf.length, 2);
}

{
  var buf = new Buffer(2);
  assert.strictEqual(buf.write(''), 0); //0bytes
  assert.strictEqual(buf.write('\x00'), 1); // 1byte (v8 adds null terminator)
  assert.strictEqual(buf.write('a\x00'), 2); // 1byte * 2
//  assert.strictEqual(buf.write('あ'), 0); // 3bytes
//  assert.strictEqual(buf.write('\x00あ'), 1); // 1byte + 3bytes
//  assert.strictEqual(buf.write('\x00\x00あ'), 2); // 1byte * 2 + 3bytes
}

{
  // #243 Test write() with maxLength
  var buf = new Buffer(4);
  buf.fill(0xFF);
  assert.strictEqual(buf.write('abcd', 1, 2, 'utf8'), 2);
  assert.strictEqual(buf.readUInt8(0), 0xFF);
  assert.strictEqual(buf.readUInt8(1), 0x61);
  assert.strictEqual(buf.readUInt8(2), 0x62);
  assert.strictEqual(buf.readUInt8(3), 0xFF);

  buf.fill(0xFF);
  assert.strictEqual(buf.write('abcd', 1, 4), 3);
  assert.strictEqual(buf.readUInt8(0), 0xFF);
  assert.strictEqual(buf.readUInt8(1), 0x61);
  assert.strictEqual(buf.readUInt8(2), 0x62);
  assert.strictEqual(buf.readUInt8(3), 0x63);

  buf.fill(0xFF);
  assert.strictEqual(buf.write('abcd', 1, 2, 'utf8'), 2);
  assert.strictEqual(buf.readUInt8(0), 0xFF);
  assert.strictEqual(buf.readUInt8(1), 0x61);
  assert.strictEqual(buf.readUInt8(2), 0x62);
  assert.strictEqual(buf.readUInt8(3), 0xFF);
}

{
  // test offset returns are correct
  var b = new Buffer(16);
  assert.strictEqual(4, b.writeUInt32LE(0, 0));
  assert.strictEqual(6, b.writeUInt16LE(0, 4));
  assert.strictEqual(7, b.writeUInt8(0, 6));
}

{
  // test unmatched surrogates not producing invalid utf8 output
  // ef bf bd = utf-8 representation of unicode replacement character
  // see https://codereview.chromium.org/121173009/
//  var buf = new Buffer('ab\ud800cd', 'utf8');
//  assert.strictEqual(buf.readUInt8(0), 0x61);
//  assert.strictEqual(buf.readUInt8(1), 0x62);
//  assert.strictEqual(buf.readUInt8(2), 0xef);
//  assert.strictEqual(buf.readUInt8(3), 0xbf);
//  assert.strictEqual(buf.readUInt8(4), 0xbd);
//  assert.strictEqual(buf.readUInt8(5), 0x63);
//  assert.strictEqual(buf.readUInt8(6), 0x64);
}

assert.strictEqual(new Buffer(-1).length, 0);
assert.strictEqual(new Buffer(NaN).length, 0);
assert.strictEqual(new Buffer(3.3).length, 3);

// Make sure that strings are not coerced to numbers.
assert.strictEqual(new Buffer('99').length, 2);
assert.strictEqual(new Buffer('13.37').length, 5);

//assert.throws(function() { return Buffer.from('', 'buffer') }, TypeError);
//assert.throws(function() { return Buffer((-1 >>> 0) + 1) }, RangeError);
//assert.throws(function() { return Buffer((-1 >>> 0) + 1) }, RangeError);
