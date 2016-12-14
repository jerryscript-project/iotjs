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


var Readable = require('stream').Readable;
var assert = require('assert');


var readable = new Readable();
var d = "";
var e = "";


readable.on('error', function(err) {
  e += ".";
});

readable.on('data', function(data) {
  d += data.toString();
});

readable.on('end', function() {
  e += 'e';
});


readable.pause();
readable.push('abcde');
readable.push('12345');
assert.equal(d, '');
assert.equal(e, '');

readable.resume();
assert.equal(d, 'abcde12345');
assert.equal(e, '');

readable.push('a');
readable.push('1');
readable.push('b');
readable.push('2');
assert.equal(d, 'abcde12345a1b2');
assert.equal(e, '');

readable.pause();
assert.equal(d, 'abcde12345a1b2');
assert.equal(e, '');

readable.push('c');
readable.push('3');
readable.push('d');
readable.push('4');
assert.equal(d, 'abcde12345a1b2');
assert.equal(e, '');

readable.resume();
assert.equal(d, 'abcde12345a1b2c3d4');
assert.equal(e, '');

readable.push(null);
assert.equal(d, 'abcde12345a1b2c3d4');
assert.equal(e, 'e');

readable.push('push after eof');
assert.equal(d, 'abcde12345a1b2c3d4');
assert.equal(e, 'e.');
