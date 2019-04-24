var common = require('common.js');
var addon = require('./build/' + common.buildTypePath +
                    '/test_napi_error_handling');
var assert = require('assert');

var ERROR_CODE = "ErrorCode";
var ERROR_MSG = "ErrorMSG"

var error = addon.CreateError(ERROR_CODE, ERROR_MSG);

assert(error.code == ERROR_CODE);
assert(error.message == ERROR_MSG);

assert(error instanceof Error);

var typeError = addon.CreateTypeError(ERROR_CODE, ERROR_MSG);
assert(typeError.code == ERROR_CODE);
assert(typeError.message == ERROR_MSG);

assert(typeError instanceof TypeError);

var rangeError = addon.CreateRangeError(ERROR_CODE, ERROR_MSG);
assert(rangeError.code == ERROR_CODE);
assert(rangeError.message == ERROR_MSG);

assert(rangeError instanceof RangeError);
