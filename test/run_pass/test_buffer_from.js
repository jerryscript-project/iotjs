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

var array_src = [65, 66, 67];
var buff = Buffer.from(array_src, 0, 3);
assert.equal(buff.toString(), "ABC");
var buff2 = Buffer.from(array_src, 0, -3);
// Buffer.from(string, encoding)
var string_utf = "ABC";
var buff3 = Buffer.from(string_utf, 'utf8');
assert.equal(buff3.toString(), "ABC");

var string_hex = "414243";
var buff4 = Buffer.from(string_hex, 'hex');
assert.equal(buff4.toString(), "ABC");

// Buffer.from(Buffer)
var buffer_src = new Buffer([0x41, 0x42, 0x43]);
var buff5 = Buffer.from(buffer_src);
assert.equal(buff5.toString(), "ABC");

var buff_undef = new Buffer(10);
var buff6 = Buffer.from(buff_undef);
assert.equal(buff6.toString(), buff_undef.toString());

// Corner case tests
var obj = {};
var num = 5;
assert.throws(function() { var buffer = Buffer.from(obj); },
              TypeError);
assert.throws(function() { var buffer = Buffer.from(num); },
              TypeError);
