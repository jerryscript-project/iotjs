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


var assert = require('assert');

// Timeout without callback function.
assert.throws(function() {
  setTimeout(1000);
}, TypeError);

// Timeout with invalid callback function parameter.
assert.throws(function() {
  setTimeout({function: null}, 1000);
}, TypeError);

assert.throws(function() {
  setTimeout({function: null, value: function() {}}, 1000);
}, TypeError);

assert.throws(function() {
  setTimeout(null, 1000);
}, TypeError);

assert.throws(function() {
  setTimeout(undefined, 1000);
}, TypeError);

assert.throws(function() {
  setTimeout([10, 100, 1000], 1000);
}, TypeError);

assert.throws(function() {
  setTimeout('callback', 1000);
}, TypeError);

// ClearTimeout without timeout parameter.
assert.doesNotThrow(function() {
  clearTimeout();
});

assert.throws(function() {
  clearTimeout('timeout');
}, Error);

assert.throws(function() {
  clearTimeout(1000);
}, Error);

assert.doesNotThrow(function() {
  clearTimeout(null);
});

assert.doesNotThrow(function() {
  clearTimeout(undefined);
});

assert.throws(function() {
  clearTimeout({timeout: 1000});
}, Error);

// SetInterval without callback function.
assert.throws(function() {
  setInterval(10);
}, TypeError);

// SetInterval with invalid callback function parameter.
assert.throws(function() {
  setInterval(1000, 10);
}, TypeError);

assert.throws(function() {
  setInterval('callback', 10);
}, TypeError);

assert.throws(function() {
  setInterval({function: null, value: function() {}}, 10);
}, TypeError);

assert.throws(function() {
  setInterval(null, 10);
}, TypeError);

assert.throws(function() {
  setInterval(undefined, 10);
}, TypeError);

assert.throws(function() {
  setInterval([10, 100, 1000], 10);
}, TypeError);

// ClearInterval without interval parameter.
assert.doesNotThrow(function() {
  clearInterval();
});

// ClearInterval with invalid interval parameter.
assert.doesNotThrow(function() {
  clearInterval(null);
});

assert.doesNotThrow(function() {
  clearInterval(undefined);
});

assert.throws(function() {
  clearInterval('interval');
}, Error);

assert.throws(function() {
  clearInterval(1000);
}, Error);

assert.throws(function() {
  clearInterval([interval, 1000]);
}, Error);

assert.throws(function() {
  clearInterval({interval: function() {}, value: 1000});
}, Error);
