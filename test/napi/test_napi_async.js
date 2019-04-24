'use strict';

var common = require('common.js');
var assert = require('assert');
var test_async = require('./build/' + common.buildTypePath +
                         '/test_napi_async.node');

// Successful async execution and completion callback.
test_async.Test(5, {}, common.mustCall(function(err, val) {
  console.log(err, val);
  assert.strictEqual(err, null);
  assert.strictEqual(val, 10);
  process.nextTick(common.mustCall());
}));

// Async work item cancellation with callback.
test_async.TestCancel(common.mustCall());
