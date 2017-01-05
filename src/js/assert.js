// Originally from narwhal.js (http://narwhaljs.org)
// Copyright (c) 2009 Thomas Robinson <280north.com>
// Copyright (c) 2015 Samsung Electronics Co., Ltd.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the 'Software'), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
// ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

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

var util = require('util');


function AssertionError(options) {
  this.name = 'AssertionError';
  this.actual = options.actual;
  this.expected = options.expected;
  this.operator = options.operator;
  if (options.message) {
    this.message = options.message;
  } else {
    this.message = getMessage(this);
  }
}

util.inherits(AssertionError, Error);


function getMessage(assertion) {
  return JSON.stringify(assertion, ['actual', 'expected', 'operator']);
}


function assert(value, message) {
  if (!value) {
    fail(value, true, message, '==');
  }
}


function fail(actual, expected, message, operator) {
  throw new AssertionError({
    message: message,
    actual: actual,
    expected: expected,
    operator: operator
  });
}


function equal(actual, expected, message) {
  if (actual != expected) {
    fail(actual, expected, message, '==');
  }
}


function notEqual(actual, expected, message) {
  if (actual == expected) {
    fail(actual, expected, message, '!=');
  }
}


function strictEqual(actual, expected, message) {
  if (actual !== expected) {
    fail(actual, expected, message, '===');
  }
}


function notStrictEqual(actual, expected, message) {
  if (actual === expected) {
    fail(actual, expected, message, '!==');
  }
}


function throws(block, expected, message) {
  var actual;

  try {
    block();
  } catch (e) {
    actual = e;
  }

  message = (expected && expected.name ? '(' + expected.name + ').' : '.') +
            (message ? ' ' + message : '.');

  if (!actual) {
    fail(actual, expected, 'Missing expected exception' + message);
  }
  if (!(actual instanceof expected)) {
    throw actual;
  }
}


function doesNotThrow(block, message) {
  var actual;

  try {
    block();
  } catch (e) {
    actual = e;
  }

  message = (message ? ' ' + message : '');

  if (actual) {
    fail(actual, null, 'Got unwanted exception.' + message);
  }
}


assert.AssertionError = AssertionError;
assert.assert = assert;
assert.fail = fail;
assert.equal = equal;
assert.notEqual = notEqual;
assert.strictEqual = strictEqual;
assert.notStrictEqual = notStrictEqual;
assert.throws = throws;
assert.doesNotThrow = doesNotThrow;

module.exports = assert;
