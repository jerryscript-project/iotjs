/* Copyright 2015-present Samsung Electronics Co., Ltd. and other contributors
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



var assert = require('assert');

var buff1 = new Buffer("test");
assert.equal(buff1.toString(), "test");
assert.equal(buff1.toString(0, 0), "");
assert.equal(buff1.toString(0, 1), "t");
assert.equal(buff1.toString(0, 2), "te");
assert.equal(buff1.toString(0, 3), "tes");
assert.equal(buff1.toString(0, 4), "test");
assert.equal(buff1.toString(1, 4), "est");
assert.equal(buff1.toString(2, 4), "st");
assert.equal(buff1.toString(3, 4), "t");
assert.equal(buff1.toString(4, 4), "");
assert.equal(buff1.length, 4);

var buff2 = new Buffer(10);
buff2.write("abcde");
assert.equal(buff2.toString(), "abcde");
assert.equal(buff2.length ,10);

buff2.write("fgh", 5);
assert.equal(buff2.toString(), "abcdefgh");
assert.equal(buff2.length ,10);

assert.throws(function() { buff2.write("ijk", -1); }, RangeError);
assert.throws(function() { buff2.write("ijk", 10); }, RangeError);

var buff3 = Buffer.concat([buff1, buff2]);
assert.equal(buff3.toString(), "testabcdefgh");
assert.equal(buff3.length ,14);

var buff4 = new Buffer(10);
var buff5 = new Buffer('a1b2c3');
buff5.copy(buff4);
assert.equal(buff4.toString(), 'a1b2c3');
buff5.copy(buff4, 4, 2);
assert.equal(buff4.toString(), 'a1b2b2c3');
assert.throws(function() { buff5.copy(buff4, -1); }, RangeError);
assert.throws(function() { buff2.write(null, 0, 0);}, TypeError);
assert.throws(function() { buff5.copy(null); }, TypeError);
assert.throws(function() { buff5.compare(null); }, TypeError);
assert.throws(function() { buff5.equals(null); }, TypeError);
assert.throws(function() { Buffer.concat([buff1, null]);}, TypeError);
assert.throws(function() {Buffer.concat(null, null); }, TypeError);
assert.throws(function() {var buff_err = new Buffer(null); }, TypeError);

var buffer_err = new Buffer(-1);

var buff_1 = new Buffer("asd");
var buff_2 = new Buffer(2);
buff_1.copy(buff_2, 0, 0, 2);
buff_2.write("asd", 0, 3);

assert.throws(function() {var buff_3 = new Buffer('asd', 'hex'); }, TypeError);

var buff_3 = new Buffer(4);
assert.throws(function() {buff_3.writeUInt8(10000, 0); }, TypeError);
var buff_4 = new Buffer(4);
assert.throws(function() {buff_4.writeUInt8(0x11, 3000); }, RangeError);
var buff_5 = new Buffer(4);
assert.throws(function() {buff_5.readUInt8(3000); }, RangeError);
var buff_6 = buff_5.slice(undefined, 2);

buff_5.fill("asd");

buff_5.readInt8(3, true);
buff_5.writeUInt32LE(0, 0, true);
buff_5.writeUInt8(0, 0, true);
buff_5.writeUInt16LE(0, 0, true);


var buff6 = buff3.slice(1);
assert.equal(buff6.toString(), 'estabcdefgh');
assert.equal(buff6.length, 13);

var buff7 = buff6.slice(3, 5);
assert.equal(buff7.toString(), 'ab');
assert.equal(buff7.length, 2);

var buff8 = new Buffer(buff5);
assert.equal(buff8.toString(), 'a1b2c3');
assert.equal(buff8.equals(buff5), true);
assert.equal(buff8.equals(buff6), false);

var buff9 = new Buffer('abcabcabcd');
var buff10 = buff9.slice(0, 3);
var buff11 = buff9.slice(3, 6);
var buff12 = buff9.slice(6);
assert.equal(buff10.equals(buff11), true);
assert.equal(buff11.equals(buff10), true);
assert.equal(buff11.equals(buff12), false);
assert.equal(buff10.compare(buff11), 0);
assert.equal(buff11.compare(buff10), 0);
assert.equal(buff11.compare(buff12), -1);
assert.equal(buff12.compare(buff11), 1);

assert.equal(buff9.slice(-2).toString(), 'cd');
assert.equal(buff9.slice(-3, -2).toString(), 'b');
assert.equal(buff9.slice(0, -2).toString(), 'abcabcab');


assert.equal(Buffer.isBuffer(buff9), true);
assert.equal(Buffer.isBuffer(1), false);
assert.equal(Buffer.isBuffer({}), false);
assert.equal(Buffer.isBuffer('1'), false);
assert.equal(Buffer.isBuffer([1]), false);
assert.equal(Buffer.isBuffer([buff1]), false);
assert.equal(Buffer.isBuffer({obj:buff1}), false);


assert.equal(buff3.toString(), 'testabcdefgh');


var buff13 = new Buffer(4);
buff13.writeUInt8(0x11, 0);
assert.equal(buff13.readUInt8(0), 0x11);
buff13.writeUInt16LE(0x3456, 1);
assert.equal(buff13.readUInt8(1), 0x56);
assert.equal(buff13.readUInt8(2), 0x34);
assert.equal(buff13.readUInt16LE(1), 0x3456);
buff13.writeUInt32LE(0x89abcdef, 0);
assert.equal(buff13.readUInt8(0), 0xef);
assert.equal(buff13.readUInt8(1), 0xcd);
assert.equal(buff13.readUInt8(2), 0xab);
assert.equal(buff13.readUInt8(3), 0x89);
assert.equal(buff13.readUInt16LE(0), 0xcdef);
assert.equal(buff13.readUInt16LE(2), 0x89ab);


var buff14 = new Buffer([0x01, 0xa1, 0xfb]);
assert.equal(buff14.readUInt8(0), 0x01);
assert.equal(buff14.readUInt8(1), 0xa1);
assert.equal(buff14.readUInt8(2), 0xfb);
assert.equal(buff14.readInt8(2), -5);
assert.equal(buff14.toString('hex'), '01a1fb');

var buff15 = new Buffer('7456a9', 'hex');
assert.equal(buff15.length, 3);
assert.equal(buff15.readUInt8(0), 0x74);
assert.equal(buff15.readUInt8(1), 0x56);
assert.equal(buff15.readUInt8(2), 0xa9);
assert.equal(buff15.toString('hex'), '7456a9');

var buff16 = new Buffer(4);
var ret = buff16.fill(7);
assert.equal(buff16.readInt8(0), 7);
assert.equal(buff16.readInt8(1), 7);
assert.equal(buff16.readInt8(2), 7);
assert.equal(buff16.readInt8(3), 7);
assert.equal(buff16, ret);
buff16.fill(13+1024);
assert.equal(buff16.readInt8(0), 13);
assert.equal(buff16.readInt8(1), 13);
assert.equal(buff16.readInt8(2), 13);
assert.equal(buff16.readInt8(3), 13);

assert.equal(Buffer(new Array()).toString(), '');
assert.equal(new Buffer(1).readUInt8(1, true), 0);
assert.equal(new Buffer(1).readUInt16LE({}, true), 0);

var buff17 = new Buffer("a");
assert.throws(function() { buff17.fill(8071).toString(); }, TypeError);
