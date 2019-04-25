'use strict';

var common = require('common.js');
var storeEnv = require('./build/' + common.buildTypePath +
                       '/test_napi_env_store.node');
var compareEnv = require('./build/' + common.buildTypePath +
                         '/test_napi_env_compare.node');
var assert = require('assert');

// N-API environment pointers in two different modules must be different
assert.strictEqual(compareEnv(storeEnv), true);
