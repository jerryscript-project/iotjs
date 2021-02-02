'use strict';
var assert = require('assert');

var common = require('common.js');

var err;
try {
  var binding = require('./build/' + common.buildTypePath +
    '/test_napi_throw_in_init.node');
  console.log(binding);
} catch (error) {
  err = error;
}

assert.equal(err.message,
  'Directly raise exception when import napi native module');
console.log((err.stack || []).join('\n'));
