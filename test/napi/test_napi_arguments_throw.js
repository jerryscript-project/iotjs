'use strict';
var assert = require('assert');
var common = require('common.js');
var test = require('./build/' + common.buildTypePath +
                   '/test_napi_arguments.node');

try {
  test.Throw(new Error('foo'));
  assert.fail('fail path');
} catch (err) {
  assert(err != null);
  assert.strictEqual(err.message, 'foo');
}
