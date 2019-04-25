var common = require('common.js');
var addon = require('./build/' + common.buildTypePath +
                    '/test_napi_error_handling');
var assert = require('assert');

var ERROR_MSG = "ErrorMSG";

process.on("uncaughtException", function (e) {
  assert(e.message === ERROR_MSG);
});

assert(addon.GetandClearLastException() === undefined);

var err = new Error(ERROR_MSG);

addon.FatalException(err);
