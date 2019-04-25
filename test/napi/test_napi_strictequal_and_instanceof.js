'use strict';
var assert = require('assert');
var common = require('common.js');
var napi_test =
  require('./build/' + common.buildTypePath +
          '/test_napi_strictequal_and_instanceof.node');

assert(napi_test !== null);
assert.strictEqual(typeof napi_test, 'object');
assert.strictEqual(napi_test.id, 321);

assert.strictEqual(typeof napi_test.sayHello, 'function');
assert.strictEqual(napi_test.sayHello(), 'Hello');

assert.strictEqual(typeof napi_test.sayError, 'function');

var error;
try {
  napi_test.sayError();
} catch (err) {
  error = err;
}
assert(error instanceof Error);
assert.strictEqual(error.code, 'foo');
assert.strictEqual(error.message, 'bar');

var lhs = {};
assert.strictEqual(napi_test.strictEquals(lhs, lhs), true);
assert.strictEqual(napi_test.instanceof(lhs, Object), lhs instanceof Object);
