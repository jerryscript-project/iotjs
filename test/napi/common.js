'use strict';
var assert = require('assert');
var mustCallChecks = [];

var buildTypePath = process.debug ? 'Debug' : 'Release';

function mustCall(fn, criteria) {
  if (typeof fn === 'number') {
    criteria = fn;
    fn = noop;
  } else if (fn === undefined) {
    fn = noop;
  }
  if (criteria === undefined) {
    criteria = 1;
  }

  if (typeof criteria !== 'number')
    throw new TypeError('Invalid value: ' + criteria);

  var context = {
    expect: criteria,
    actual: 0,
    stack: (new Error()).stack,
    name: fn.name || '<anonymous>'
  };

  if (mustCallChecks.length === 0) process.on('exit', runCallChecks);

  mustCallChecks.push(context);

  return function() {
    ++context.actual;
    return fn.apply(this, arguments);
  };
}

function noop() {}

function runCallChecks() {
  mustCallChecks.forEach(function(it) {
    assert.strictEqual(
      it.actual,
      it.expect,
      'Expect function ' + it.name + ' been called '+ it.expect + ' times, got '
      + it.actual + ' ' + it.stack);
  });
}

function expectsError(fn, exact) {
  // TODO:rigorous assert need!
  function innerFn(error) {
    return error instanceof Error;
  }

  if (fn) {
    assert.throws(fn, innerFn, exact);
    return;
  }

  return mustCall(innerFn, exact);
}

module.exports = {
  mustCall: mustCall,
  expectsError: expectsError,
  buildTypePath: buildTypePath,
  // don't use port in a parallelized test
  PORT: process.env.NODE_COMMON_PORT || 12306
};
