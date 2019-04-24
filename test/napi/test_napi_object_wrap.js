
'use strict';
var assert = require('assert');
var common = require('common.js');
var test = require('./build/' + common.buildTypePath +
                   '/test_napi_object_wrap.node');

function context() {
  var obj = {};
  assert.strictEqual(test.Wrap(obj), obj);
}

assert.strictEqual(test.GetNativeCounter(), 0);
context();
process.gc();
assert.strictEqual(test.GetNativeCounter(), 1);
