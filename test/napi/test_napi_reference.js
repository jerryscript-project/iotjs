'use strict';
// Flags: --expose-gc

var assert = require('assert');
var common = require('common.js');

var test_reference = require('./build/' + common.buildTypePath +
                             '/test_napi_reference.node');

// This test script uses external values with finalizer callbacks
// in order to track when values get garbage-collected. Each invocation
// of a finalizer callback increments the finalizeCount property.
assert.strictEqual(test_reference.finalizeCount, 0);

// Run each test function in sequence,
// with an async delay and GC call between each.
function runTests(i, title, tests) {
  if (tests[i]) {
    if (typeof tests[i] === 'string') {
      title = tests[i];
      runTests(i + 1, title, tests);
    } else {
      try {
        tests[i]();
      } catch (e) {
        console.error('Test failed: ' + title);
        throw e;
      }
      setImmediate(function() {
        process.gc();
        runTests(i + 1, title, tests);
      });
    }
  }
}
runTests(0, undefined, [

  'External value without a finalizer',
  function() {
    var value = test_reference.createExternal();
    assert.strictEqual(test_reference.finalizeCount, 0);
    assert.strictEqual(typeof value, 'object');
    test_reference.checkExternal(value);
  },
  function() {
    assert.strictEqual(test_reference.finalizeCount, 0);
  },

  'External value with a finalizer',
  function() {
    var value = test_reference.createExternalWithFinalize();
    assert.strictEqual(test_reference.finalizeCount, 0);
    assert.strictEqual(typeof value, 'object');
    test_reference.checkExternal(value);
  },
  function() {
    assert.strictEqual(test_reference.finalizeCount, 1);
  },

  'Weak reference',
  function() {
    var value = test_reference.createExternalWithFinalize();
    assert.strictEqual(test_reference.finalizeCount, 0);
    test_reference.createReference(value, 0);
    assert.strictEqual(test_reference.referenceValue, value);
  },
  function() {
    // Value should be GC'd because there is only a weak ref
    assert.strictEqual(test_reference.referenceValue, undefined);
    assert.strictEqual(test_reference.finalizeCount, 1);
    test_reference.deleteReference();
  },

  'Strong reference',
  function() {
    var value = test_reference.createExternalWithFinalize();
    assert.strictEqual(test_reference.finalizeCount, 0);
    test_reference.createReference(value, 1);
    assert.strictEqual(test_reference.referenceValue, value);
  },
  function() {
    // Value should NOT be GC'd because there is a strong ref
    assert.strictEqual(test_reference.finalizeCount, 0);
    test_reference.deleteReference();
  },
  function() {
    // Value should be GC'd because the strong ref was deleted
    assert.strictEqual(test_reference.finalizeCount, 1);
  },

  'Strong reference, increment then decrement to weak reference',
  function() {
    var value = test_reference.createExternalWithFinalize();
    assert.strictEqual(test_reference.finalizeCount, 0);
    test_reference.createReference(value, 1);
  },
  function() {
    // Value should NOT be GC'd because there is a strong ref
    assert.strictEqual(test_reference.finalizeCount, 0);
    assert.strictEqual(test_reference.incrementRefcount(), 2);
  },
  function() {
    // Value should NOT be GC'd because there is a strong ref
    assert.strictEqual(test_reference.finalizeCount, 0);
    assert.strictEqual(test_reference.decrementRefcount(), 1);
  },
  function() {
    // Value should NOT be GC'd because there is a strong ref
    assert.strictEqual(test_reference.finalizeCount, 0);
    assert.strictEqual(test_reference.decrementRefcount(), 0);
  },
  function() {
    // Value should be GC'd because the ref is now weak!
    assert.strictEqual(test_reference.finalizeCount, 1);
    test_reference.deleteReference();
  },
  function() {
    // Value was already GC'd
    assert.strictEqual(test_reference.finalizeCount, 1);
  },
]);
