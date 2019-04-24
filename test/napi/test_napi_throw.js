var common = require('common.js');
var addon = require('./build/' + common.buildTypePath +
                    '/test_napi_error_handling');
var assert = require('assert');

var ERROR_MSG = "ErrorMSG";
var error = new Error(ERROR_MSG);
var catched;

try {
  addon.Throw(error)
} catch (e) {
  catched = e;
}

assert(catched instanceof Error)
assert(catched.message === ERROR_MSG)

try {
  addon.Throw(ERROR_MSG)
} catch (e) {
  catched = e;
}

assert(typeof catched === 'string')
assert(catched === ERROR_MSG)
