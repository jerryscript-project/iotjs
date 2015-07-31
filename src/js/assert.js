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
  // FIXME: use `JSON.stringify` to generate assertion message.
  var msg  = '{ actual: '
           + assertion.actual
           + ', expected: '
           + assertion.expected
           + ', operator: '
           + assertion.operator
           + ' }';
  return msg;
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
