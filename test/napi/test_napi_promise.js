'use strict';

var common = require('common.js');

// This tests the promise-related n-api calls

var assert = require('assert');
var test_promise = require('./build/' + common.buildTypePath +
                           '/test_napi_promise.node');

// A resolution
{
  var expected_result = 42;
  var promise = test_promise.createPromise();
  promise.then(
    common.mustCall(function(result) {
      assert.strictEqual(result, expected_result);
    }));
  test_promise.concludeCurrentPromise(expected_result, true);
}

// A rejection
{
  var expected_result = 'It\'s not you, it\'s me.';
  var promise = test_promise.createPromise();
  promise.then(function(result) {
      // This should never be called
      assert.strictEqual(true, false);
    });
  test_promise.concludeCurrentPromise(expected_result, false);
}

// Chaining
{
  var expected_result = 'chained answer';
  var promise = test_promise.createPromise();
  promise.then(
    common.mustCall(function(result) {
      assert.strictEqual(result, expected_result);
    }));
  test_promise.concludeCurrentPromise(Promise.resolve('chained answer'), true);
}

var promiseTypeTestPromise = test_promise.createPromise();
assert.strictEqual(test_promise.isPromise(promiseTypeTestPromise), true);
test_promise.concludeCurrentPromise(undefined, true);

var rejectPromise = Promise.reject(-1);
var expected_reason = -1;
assert.strictEqual(test_promise.isPromise(rejectPromise), true);
rejectPromise.catch((reason) => {
  assert.strictEqual(reason, expected_reason);
});

assert.strictEqual(test_promise.isPromise(2.4), false);
assert.strictEqual(test_promise.isPromise('I promise!'), false);
assert.strictEqual(test_promise.isPromise(undefined), false);
assert.strictEqual(test_promise.isPromise(null), false);
assert.strictEqual(test_promise.isPromise({}), false);
