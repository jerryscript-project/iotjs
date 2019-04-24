'use strict';
var assert = require('assert');
var common = require('common.js');
var test = require('./build/' + common.buildTypePath +
                   '/test_napi_construct.node');

var val = test.Constructor(123);
assert.strictEqual(val.value, 123);
