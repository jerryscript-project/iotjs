var common = require('common.js');
var assert = require('assert');
var testHandleScope = require('./build/' + common.buildTypePath +
                              '/test_napi_handle_scope.node');

testHandleScope.NewScope();

assert(testHandleScope.NewScopeEscape() instanceof Object);

testHandleScope.NewScopeEscapeTwice();

assert.throws(
  function() {
    testHandleScope.NewScopeWithException(function() {
      throw new RangeError();
    });
  },
  RangeError
);
