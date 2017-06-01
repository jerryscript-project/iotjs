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
require('node/common');

var assert = require('assert');
var a = require('assert');

// Test starts here

function makeBlock(f) {
  var args = Array.prototype.slice.call(arguments, 1);
  return function() {
    return f.apply(this, args);
  };
}

assert.ok(a.AssertionError.prototype instanceof Error,
          'a.AssertionError instanceof Error');

assert.throws(makeBlock(a, false), a.AssertionError, 'ok(false)');

assert.doesNotThrow(makeBlock(a, true), a.AssertionError, 'ok(true)');

assert.doesNotThrow(makeBlock(a, 'test', 'ok(\'test\')'));

assert.throws(makeBlock(a.ok, false),
              a.AssertionError, 'ok(false)');

assert.doesNotThrow(makeBlock(a.ok, true),
                    a.AssertionError, 'ok(true)');

assert.doesNotThrow(makeBlock(a.ok, 'test'), 'ok(\'test\')');

assert.throws(makeBlock(a.equal, true, false),
              a.AssertionError, 'equal(true, false)');

assert.doesNotThrow(makeBlock(a.equal, null, null),
                    'equal(null, null)');

assert.doesNotThrow(makeBlock(a.equal, undefined, undefined),
                    'equal(undefined, undefined)');

assert.doesNotThrow(makeBlock(a.equal, null, undefined),
                    'equal(null, undefined)');

assert.doesNotThrow(makeBlock(a.equal, true, true), 'equal(true, true)');

assert.doesNotThrow(makeBlock(a.equal, 2, '2'), 'equal(2, \'2\')');

assert.doesNotThrow(makeBlock(a.notEqual, true, false),
                    'notEqual(true, false)');

assert.throws(makeBlock(a.notEqual, true, true),
              a.AssertionError, 'notEqual(true, true)');

assert.throws(makeBlock(a.strictEqual, 2, '2'),
              a.AssertionError, 'strictEqual(2, \'2\')');

assert.throws(makeBlock(a.strictEqual, null, undefined),
              a.AssertionError, 'strictEqual(null, undefined)');

assert.throws(makeBlock(a.notStrictEqual, 2, 2),
              a.AssertionError, 'notStrictEqual(2, 2)');

assert.doesNotThrow(makeBlock(a.notStrictEqual, 2, '2'),
                    'notStrictEqual(2, \'2\')');

// Testing the throwing
function thrower(errorConstructor) {
  throw new errorConstructor('test');
}

// the basic calls work
assert.throws(makeBlock(thrower, a.AssertionError),
              a.AssertionError, 'message');
assert.throws(makeBlock(thrower, a.AssertionError), a.AssertionError);
assert.throws(makeBlock(thrower, a.AssertionError));

// if not passing an error, catch all.
assert.throws(makeBlock(thrower, TypeError));

// when passing a type, only catch errors of the appropriate type
var threw = false;
try {
  a.throws(makeBlock(thrower, TypeError), a.AssertionError);
} catch (e) {
  threw = true;
  assert.ok(e instanceof TypeError, 'type');
}
assert.strictEqual(true, threw,
                   'a.throws with an explicit error is eating extra errors',
                   a.AssertionError);
threw = false;

// key difference is that throwing our correct error makes an assertion error
try {
  a.doesNotThrow(makeBlock(thrower, TypeError), TypeError);
} catch (e) {
  threw = true;
  assert.ok(e instanceof a.AssertionError);
}
assert.strictEqual(true, threw,
                   'a.doesNotThrow is not catching type matching errors');

var circular = {y: 1};
circular.x = circular;

function testAssertionMessage(actual, expected) {
  try {
    assert.strictEqual(actual, '');
  } catch (e) {
    assert.ok(e.toString());
  }
}

testAssertionMessage(undefined, 'undefined');
testAssertionMessage(null, 'null');
testAssertionMessage(true, 'true');
testAssertionMessage(false, 'false');
testAssertionMessage(0, '0');
testAssertionMessage(100, '100');
testAssertionMessage(NaN, 'NaN');
testAssertionMessage(Infinity, 'Infinity');
testAssertionMessage(-Infinity, '-Infinity');
testAssertionMessage('', '""');
testAssertionMessage('foo', '\'foo\'');
testAssertionMessage([], '[]');
testAssertionMessage([1, 2, 3], '[ 1, 2, 3 ]');
testAssertionMessage(/a/, '/a/');
testAssertionMessage(/abc/gim, '/abc/gim');
testAssertionMessage(function f() {}, '[Function: f]');
testAssertionMessage(function() {}, '[Function]');
testAssertionMessage({}, '{}');
testAssertionMessage(circular, '{ y: 1, x: [Circular] }');
testAssertionMessage({a: undefined, b: null}, '{ a: undefined, b: null }');
testAssertionMessage({a: NaN, b: Infinity, c: -Infinity},
                     '{ a: NaN, b: Infinity, c: -Infinity }');

// #2893
try {
  assert.throws(function() {
    assert.ifError(null);
  });
} catch (e) {
  threw = true;
  assert.strictEqual(e.message, 'Missing expected exception..');
}
assert.ok(threw);

console.log('All OK');
