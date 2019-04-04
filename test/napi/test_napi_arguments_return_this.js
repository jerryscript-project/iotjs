'use strict';
var assert = require('assert');
var test = require('./build/Release/test_napi_arguments.node');


var obj = {};
assert.strictEqual(test.ReturnThis.call(obj), obj);
