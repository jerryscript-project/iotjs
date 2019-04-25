var common = require('common.js');
var addon = require('./build/' + common.buildTypePath +
                    '/test_napi_error_handling');
var assert = require('assert');

var error;
var ERROR_CODE = "ErrorCODE";
var ERROR_MSG = "ErrorMSG";

// Error
try {
  addon.ThrowError();
} catch (e) {
  error = e;
}

assert(error instanceof Error);
assert(error.code === "ErrorCODE");
assert(error.message === "ErrorMSG");

//TypeError
try {
  addon.ThrowTypeError();
} catch (e) {
  error = e;
}

assert(error instanceof TypeError);
assert(error.code === "ErrorCODE");
assert(error.message === "ErrorMSG");

//RangeError
try {
  addon.ThrowRangeError();
} catch (e) {
  error = e;
}
assert(error instanceof RangeError);
assert(error.code === "ErrorCODE");
assert(error.message === "ErrorMSG");
