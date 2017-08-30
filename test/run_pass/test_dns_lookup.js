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

var dns = require('dns');
var assert = require('assert');

var options = {
  family: 4,
  hints: dns.ADDRCONFIG | dns.V4MAPPED,
};

function isIPv4(ip) {
  var IPv4Regex = new RegExp(
    '^([0-9]{1,3}\.){3}[0-9]{1,3}(\/([0-9]|[1-2][0-9]|3[0-2]))?$'
  );

  return IPv4Regex.test(ip);
}

dns.lookup('localhost', 4, function(err, ip, family) {
  assert.equal(err, null);
  assert.equal(isIPv4(ip), true);
  assert.equal(ip, '127.0.0.1');
  assert.strictEqual(family, 4);
});

// Test with IPv4 option.
dns.lookup('localhost', 4, function(err, ip, family) {
  assert.equal(err, null);
  assert.equal(isIPv4(ip), true);
  assert.equal(ip, '127.0.0.1');
  assert.strictEqual(family, 4);
});

// Test without explicit options parameter.
dns.lookup('localhost', function(err, ip, family) {
  assert.equal(err, null);
  assert.equal(isIPv4(ip), true);
  assert.equal(ip, '127.0.0.1');
});

// Test with invalid hostname.
dns.lookup('invalid', 4, function(err, ip, family) {
  assert.notEqual(err, null);
});

// Test with empty hostname.
dns.lookup('', 4, function(err, ip, family) {
  assert.notEqual(err, null);
});

// Test with non string hostname.
assert.throws(function() {
  dns.lookup(5000, options, function(err, ip, family) {});
}, TypeError);

// Test without callback function.
assert.throws(function() {
  dns.lookup('localhost', options);
}, TypeError);

// Test with invalid callback parameter.
assert.throws(function() {
  dns.lookup('localhost', options, 5000);
}, TypeError);

assert.throws(function() {
  dns.lookup('localhost', options, 'callback');
}, TypeError);

// Test with invalid hints option.
options.hints = -1;
assert.throws(function() {
  dns.lookup('localhost', options, function(err, ip, family) {});
}, TypeError);

options.hints = 5000;
assert.throws(function() {
  dns.lookup('localhost', options, function(err, ip, family) {});
}, TypeError);

// Test with invalid options parameter.
assert.throws(function() {
  dns.lookup('localhost', 'options', function(err, ip, family) {});
}, TypeError);

assert.throws(function() {
  dns.lookup('localhost', '', function(err, ip, family) {});
}, TypeError);

// Test with invalid family option.
assert.throws(function() {
  dns.lookup('localhost', 10, function(err, ip, family) {});
}, TypeError);

assert.throws(function() {
  dns.lookup('localhost', -5, function(err, ip, family) {});
}, TypeError);
