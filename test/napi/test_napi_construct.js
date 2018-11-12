'use strict';
var assert = require('assert');
var test = require('./build/Release/test_napi_construct.node');

var val = test.Constructor(123);
assert.strictEqual(val.value, 123);
