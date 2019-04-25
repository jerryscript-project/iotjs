'use strict';
var assert = require('assert');
var common = require('common.js');
var test = require('./build/' + common.buildTypePath +
                   '/test_napi_arguments.node');


var obj = {};
assert.strictEqual(test.Return(obj), obj);
