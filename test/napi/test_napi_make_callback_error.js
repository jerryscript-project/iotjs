'use strict';

var common = require('common.js');
var assert = require('assert');
var binding = require('./build/' + common.buildTypePath +
                      '/test_napi_make_callback.node');
var makeCallback = binding.makeCallback;

var first = true;
process.on('uncaughtException', function(err) {
  if (first) {
    assert.strictEqual(err.message, 'foobar');
    first = false;
    return;
  }
  assert.strictEqual(err.message, 'tick');
});

process.nextTick(common.mustCall(function() {
  throw new Error('tick');
}));
makeCallback(process, common.mustCall(function() {
  throw new Error('foobar');
}));
