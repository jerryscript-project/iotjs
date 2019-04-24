'use strict';
var assert = require('assert');
var common = require('common.js');

// Testing api calls for arrays
var test_array = require('./build/' + common.buildTypePath +
                         '/test_napi_array.node');

var array = [
  1,
  9,
  48,
  13493,
  9459324,
  { name: 'hello' },
  [
    'world',
    'node',
    'abi'
  ]
];

assert.throws(
  function() {
    test_array.TestGetElement(array, array.length + 1);
  }
);

assert.throws(
  function() {
    test_array.TestGetElement(array, -2);
  }
);

array.forEach(function(element, index) {
  assert.strictEqual(test_array.TestGetElement(array, index), element);
});


// assert.deepStrictEqual(test_array.New(array), array);

assert(test_array.TestHasElement(array, 0));
assert.strictEqual(test_array.TestHasElement(array, array.length + 1), false);

assert(test_array.NewWithLength(0) instanceof Array);
assert(test_array.NewWithLength(1) instanceof Array);
// check max allowed length for an array 2^32 -1
assert(test_array.NewWithLength(4294967295) instanceof Array);

{
  // Verify that array elements can be deleted.
  var arr = ['a', 'b', 'c', 'd'];

  assert.strictEqual(arr.length, 4);
  assert.strictEqual(2 in arr, true);
  assert.strictEqual(test_array.TestDeleteElement(arr, 2), true);
  assert.strictEqual(arr.length, 4);
  assert.strictEqual(2 in arr, false);
}
