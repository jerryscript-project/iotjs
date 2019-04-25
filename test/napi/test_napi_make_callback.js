'use strict';

var common = require('common.js');
var assert = require('assert');
var binding = require('./build/' + common.buildTypePath +
                      '/test_napi_make_callback.node');
var makeCallback = binding.makeCallback;

function myMultiArgFunc(arg1, arg2, arg3) {
  assert.strictEqual(arg1, 1);
  assert.strictEqual(arg2, 2);
  assert.strictEqual(arg3, 3);
  return 42;
}

assert.strictEqual(42, makeCallback(process, common.mustCall(function() {
  assert.strictEqual(0, arguments.length);
  assert.strictEqual(this, process);
  return 42;
})));

assert.strictEqual(42, makeCallback(process, common.mustCall(function(x) {
  assert.strictEqual(1, arguments.length);
  assert.strictEqual(this, process);
  assert.strictEqual(x, 1337);
  return 42;
}), 1337));

assert.strictEqual(42,
                   makeCallback(this,
                                common.mustCall(myMultiArgFunc), 1, 2, 3));
