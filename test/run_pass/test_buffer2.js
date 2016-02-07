/* Copyright 2015 Samsung Electronics Co., Ltd.
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

var str1 = '\u0801\u00EE';
var len1 = Buffer.byteLength(str1,'cesu8');
var len2 = Buffer.byteLength(str1,'utf16le');
var len2_1 = Buffer.byteLength(str1,'utf8');
assert.equal(len1, 5);
assert.equal(len2, 4);
assert.equal(len2_1, 5);

assert.equal(Buffer.byteLength(str1), 5);

var str2 = '\uD83D\uDC4D'; // THUMBS UP SIGN (U+1F44D)
var len3 = Buffer.byteLength(str2, 'utf16le');
var len4 = Buffer.byteLength(str2, 'cesu8');
var len4_1 = Buffer.byteLength(str2, 'utf8');
assert.equal(len3, 4);
assert.equal(len4, 6);
assert.equal(len4_1, 4);


// CHECKME: this unicode for surrogate code,
// but it works in Node.js and IoT.js
var str3 = '\uD800';
var len5 = Buffer.byteLength(str3,'cesu8');
var len6 = Buffer.byteLength(str3,'utf16le');
assert.equal(len5, 3);
assert.equal(len6, 2);
