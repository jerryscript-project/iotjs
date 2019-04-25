'use strict';
var assert = require('assert');
var common = require('common.js');

// Testing api calls for arrays
var test_dataview = require('./build/' + common.buildTypePath +
                            '/test_napi_dataview.node');

// Test for creating dataview
{
  var buffer = new ArrayBuffer(128);
  var template = new DataView(buffer);

  var theDataview = test_dataview.CreateDataViewFromJSDataView(template);
  assert(theDataview instanceof DataView,
         'Expect ' + theDataview + ' to be a DataView');
}

// Test for creating dataview with invalid range
{
  var buffer = new ArrayBuffer(128);
  assert.throws(function() {
    test_dataview.CreateDataView(buffer, 10, 200);
  }, RangeError);
}
