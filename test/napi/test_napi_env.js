'use strict';

var storeEnv = require('./build/Release/test_napi_env_store.node');
var compareEnv = require('./build/Release/test_napi_env_compare.node');
var assert = require('assert');

// N-API environment pointers in two different modules must be different
assert.strictEqual(compareEnv(storeEnv), true);
