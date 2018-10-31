/* Copyright 2018-present Samsung Electronics Co., Ltd. and other contributors
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

var source = new ArrayBuffer(10);

//creation tests
var buff1 = Buffer.from(source, 0, 10);
assert.equal(buff1.length, 10);
var buff2 = Buffer.from(source, 3, 7);
assert.throws(function() { var buffer = buffer.from(source, 3, 10) })
assert.equal(buff2.length, 7);
var buff3 = Buffer.from(source, 10, 0);
assert.equal(buff3.length, 0);
var buff4 = Buffer.from(source, 0, 0);
assert.equal(buff4.length, 0);
assert.throws(function() { var buffer = Buffer.from(source, 0, 1000); },
              RangeError);
assert.throws(function() { var buffer = Buffer.from(source, 1000, 9); },
              RangeError);
assert.throws(function() { var buffer = Buffer.from(source, 1000, 1000); },
              RangeError);

var buff5 = Buffer.from(source, undefined, 10);
assert.equal(buff5.length, 10);
var buff6 = Buffer.from(source, undefined, 0);
assert.equal(buff6.length, 0);

var buff7 = Buffer.from(source, undefined, -10);
assert.equal(buff7.length, 0);
var buff8 = Buffer.from(source, 0, undefined);
assert.equal(buff8.length, 10);
var buff9 = Buffer.from(source, 10, undefined);
assert.equal(buff9.length, 0);
assert.throws(function() {var buffer = Buffer.from(source, -10, undefined); },
              RangeError);
var buff10 = Buffer.from(source, undefined, undefined);
assert.equal(buff10.length, 10);

var buff11 = Buffer.from(source, NaN, 10);
assert.equal(buff11.length, 10);
var buff12 = Buffer.from(source, NaN, 0);
assert.equal(buff12.length, 0);
var buff13 = Buffer.from(source, NaN, -10);
assert.equal(buff13.length, 0);

var buff14 = Buffer.from(source, 0, NaN);
assert.equal(buff14.length, 0);

var buff15 = Buffer.from(source, 10, NaN);
assert.equal(buff15.length, 0);
assert.throws(function() { var buffer = Buffer.from(source, -10, NaN); },
              RangeError);

//value checks
var typed_source1 = new Uint8Array([65, 66]);
var arr_buff = Buffer.from(typed_source1.buffer, 0, 2);
assert.equal(arr_buff.toString('utf-8'), 'AB');

var typed_source2 = new Uint16Array([65, 66]);
var arr_buff2 = Buffer.from(typed_source2.buffer, 0, 1);
var arr_buff3 = Buffer.from(typed_source2.buffer, 2, 1);
assert.equal(arr_buff2.toString(), 'A');
assert.equal(arr_buff3.toString(), 'B');

var typed_source3 = new Uint8Array([42, 43]);
var arr_buff4 = Buffer.from(typed_source3.buffer, 0, 2);
assert.equal(arr_buff4.toString('hex'), '2a2b');
