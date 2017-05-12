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


var readable1 = new Readable();
var d = "";
var e = "";


readable1.on('error', function(err) {
  e += ".";
});

readable1.on('data', function(data) {
  d += data.toString();
});

readable1.on('end', function() {
  e += 'e';
});


readable1.pause();
readable1.push('abcde');
readable1.push('12345');
assert.equal(d, '');
assert.equal(e, '');

readable1.resume();
assert.equal(d, 'abcde12345');
assert.equal(e, '');

readable1.push('a');
readable1.push('1');
readable1.push('b');
readable1.push('2');
assert.equal(d, 'abcde12345a1b2');
assert.equal(e, '');

assert.equal(readable1.isPaused(), false);
readable1.pause();
assert.equal(d, 'abcde12345a1b2');
assert.equal(e, '');
assert.equal(readable1.isPaused(), true);

// Pause the readable again. This should do nothing.
readable1.pause();
assert.equal(readable1.isPaused(), true);

readable1.push('c');
readable1.push('3');
readable1.push('d');
readable1.push('4');
assert.equal(d, 'abcde12345a1b2');
assert.equal(e, '');

readable1.resume();
assert.equal(d, 'abcde12345a1b2c3d4');
assert.equal(e, '');

readable1.push(null);
assert.equal(d, 'abcde12345a1b2c3d4');
assert.equal(e, 'e');

readable1.push('push after eof');
assert.equal(d, 'abcde12345a1b2c3d4');
assert.equal(e, 'e.');


// Create a readable stream without the new keyword.
var readable2 = Readable({encoding: 'utf8'});

// Push an invalid chunk into it.
assert.throws(function() {
  readable2.push(undefined);
}, TypeError);

assert.throws(function() {
  readable2.push(5001);
}, TypeError);

assert.throws(function() {
  readable2.push(5001.5);
}, TypeError);

assert.throws(function() {
  readable2.push([]);
}, TypeError);

assert.throws(function() {
  readable2.push([5001, 'string']);
}, TypeError);

assert.throws(function() {
  readable2.push({});
}, TypeError);

assert.throws(function() {
  readable2.push({obj: 'string', second: 5001});
}, TypeError);

// Read with irregular parameters from an empty stream.
assert.equal(readable2.read(-2), null);
assert.equal(readable2.read(0), null);

readable2.push('qwerty');
assert.equal(readable2.read(6), 'qwerty');

// Throw not implemented Error when we trying to read less length data.
readable2.push('new-data');
assert.throws(function() {
  readable2.read(1);
}, Error);

var readable3 = new Readable();
var readable3End = false;
var paused = false;
var str = 'test';

readable3.on('data', function(data) {
  assert.equal(paused, true);
  assert.equal(data, str);
});

readable3.on('end', function() {
  readable3End = true;
});

readable3.pause();
readable3.push(str);
readable3.push(null);

setTimeout(function() {
  paused = true;
  readable3.resume();
}, 1000);

// End on non-EOF stream
var readable4 = new Readable();
readable4.length = 10;
assert.throws(function() {
  readable4.push(null);
}, Error);


process.on('exit', function() {
  assert.equal(readable2 instanceof Readable, true);
  assert.equal(readable3End, true);
});
