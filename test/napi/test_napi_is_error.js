var common = require('common.js');
var addon = require('./build/' + common.buildTypePath +
                    '/test_napi_error_handling');
var assert = require('assert');

var err = new Error("ErrorMSG");
try {
  var c = true;
  throw c
} catch (e) {
  assert(addon.IsError(e) === false);
}
assert(addon.IsError(err));
