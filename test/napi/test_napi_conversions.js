'use strict';
var assert = require('assert');
var common = require('common.js');
var test = require('./build/' + common.buildTypePath +
                   '/test_napi_conversions.node');

assert.strictEqual(false, test.asBool(false));
assert.strictEqual(true, test.asBool(true));
assert.throws(function() { test.asBool(undefined) }, Error);
assert.throws(function() { test.asBool(null) }, Error);
assert.throws(function() { test.asBool(Number.NaN) }, Error);
assert.throws(function() { test.asBool(0) }, Error);
assert.throws(function() { test.asBool('') }, Error);
assert.throws(function() { test.asBool('0') }, Error);
assert.throws(function() { test.asBool(1) }, Error);
assert.throws(function() { test.asBool('1') }, Error);
assert.throws(function() { test.asBool('true') }, Error);
assert.throws(function() { test.asBool({}) }, Error);
assert.throws(function() { test.asBool([]) }, Error);

[test.asInt32, test.asUInt32, test.asInt64].forEach(function (asInt) {
  assert.strictEqual(0, asInt(0));
  assert.strictEqual(1, asInt(1));
  assert.strictEqual(1, asInt(1.0));
  assert.strictEqual(1, asInt(1.1));
  assert.strictEqual(1, asInt(1.9));
  assert.strictEqual(0, asInt(0.9));
  assert.strictEqual(999, asInt(999.9));
  assert.strictEqual(0, asInt(Number.NaN));
  assert.throws(function() { asInt(undefined) }, Error);
  assert.throws(function() { asInt(null) }, Error);
  assert.throws(function() { asInt(false) }, Error);
  assert.throws(function() { asInt('') }, Error);
  assert.throws(function() { asInt('1') }, Error);
  assert.throws(function() { asInt({}) }, Error);
  assert.throws(function() { asInt([]) }, Error);
});

assert.strictEqual(-1, test.asInt32(-1));
assert.strictEqual(-1, test.asInt64(-1));
assert.strictEqual(Math.pow(2, 32) - 1, test.asUInt32(-1));
assert.strictEqual(0, test.asDouble(0));
assert.strictEqual(1, test.asDouble(1));
assert.strictEqual(1.0, test.asDouble(1.0));

assert.strictEqual(1.1, test.asDouble(1.1));
assert.strictEqual(1.9, test.asDouble(1.9));
assert.strictEqual(0.9, test.asDouble(0.9));
assert.strictEqual(999.9, test.asDouble(999.9));
assert.strictEqual(-1, test.asDouble(-1));
assert.throws(function() { test.asDouble(undefined) }, Error);
assert.throws(function() { test.asDouble(null) }, Error);
assert.throws(function() { test.asDouble(false) }, Error);
assert.throws(function() { test.asDouble('') }, Error);
assert.throws(function() { test.asDouble('1') }, Error);
assert.throws(function() { test.asDouble({}) }, Error);
assert.throws(function() { test.asDouble([]) }, Error);

assert.strictEqual('', test.asString(''));
assert.strictEqual('test', test.asString('test'));
assert.throws(function() { test.asString(undefined) }, Error);
assert.throws(function() { test.asString(null) }, Error);
assert.throws(function() { test.asString(false) }, Error);
assert.throws(function() { test.asString(1) }, Error);
assert.throws(function() { test.asString(1.1) }, Error);
assert.throws(function() { test.asString(Number.NaN) }, Error);
assert.throws(function() { test.asString({}) }, Error);
assert.throws(function() { test.asString([]) }, Error);

assert.strictEqual(true, test.toBool(true));
assert.strictEqual(true, test.toBool(1));
assert.strictEqual(true, test.toBool(-1));
assert.strictEqual(true, test.toBool('true'));
assert.strictEqual(true, test.toBool('false'));
assert.strictEqual(true, test.toBool({}));
assert.strictEqual(true, test.toBool([]));
assert.strictEqual(false, test.toBool(false));
assert.strictEqual(false, test.toBool(undefined));
assert.strictEqual(false, test.toBool(null));
assert.strictEqual(false, test.toBool(0));
assert.strictEqual(false, test.toBool(Number.NaN));
assert.strictEqual(false, test.toBool(''));

assert.strictEqual(0, test.toNumber(0));
assert.strictEqual(1, test.toNumber(1));
assert.strictEqual(1.1, test.toNumber(1.1));
assert.strictEqual(-1, test.toNumber(-1));
assert.strictEqual(0, test.toNumber('0'));
assert.strictEqual(1, test.toNumber('1'));
assert.strictEqual(1.1, test.toNumber('1.1'));
assert.strictEqual(0, test.toNumber([]));
assert.strictEqual(0, test.toNumber(false));
assert.strictEqual(0, test.toNumber(null));
assert.strictEqual(0, test.toNumber(''));

Number.isNaN = Number.isNaN || function(value) {
  return value !== value;
}

assert(Number.isNaN(test.asDouble(Number.NaN)));
assert(Number.isNaN(test.toNumber(Number.NaN)));
assert(Number.isNaN(test.toNumber({})));
assert(Number.isNaN(test.toNumber(undefined)));

assert.deepStrictEqual = assert.deepStrictEqual || function(expected, value) {
  var keys = Object.keys(value);
  if ((typeof expected !== typeof value)
      || (keys.length !== Object.keys(expected).length)) {
    assert(false);
  }

  if (keys.length > 0) {
    for (var key in keys) {
      if (typeof value[keys[key]] !== 'object') {
        assert.strictEqual(expected[keys[key]], value[keys[key]]);
      } else {
        assert.deepStrictEqual(expected[keys[key]], value[keys[key]]);
      }
    }
  } else if (typeof value.valueOf() !== 'object') {
    assert.strictEqual(expected.valueOf(), value.valueOf());
  }
}

assert.notDeepStrictEqual = assert.notDeepStrictEqual ||
function(expected, value) {
  if ((typeof expected !== typeof value)
      || (Object.keys(value).length !== Object.keys(expected).length)) {
    return;
  }

  var keys = Object.keys(value);
  if (keys.length > 0) {
    for (var key in keys) {
      if (typeof value[keys[key]] !== 'object') {
        assert.notStrictEqual(expected[keys[key]], value[keys[key]]);
      } else {
        assert.notDeepStrictEqual(expected[keys[key]], value[keys[key]]);
      }
    }
  } else if (typeof value.valueOf() !== 'object') {
    assert.notStrictEqual(expected.valueOf(), value.valueOf());
  }
}

assert.deepStrictEqual({}, test.toObject({}));
assert.deepStrictEqual({ 'test': 1 }, test.toObject({ 'test': 1 }));
assert.deepStrictEqual([], test.toObject([]));
assert.deepStrictEqual([ 1, 2, 3 ], test.toObject([ 1, 2, 3 ]));
assert.deepStrictEqual(new Boolean(false), test.toObject(false));
assert.deepStrictEqual(new Boolean(true), test.toObject(true));
assert.deepStrictEqual(new String(''), test.toObject(''));
assert.deepStrictEqual(new Number(0), test.toObject(0));
assert.notDeepStrictEqual(new Number(Number.NaN), test.toObject(Number.NaN));
assert.notDeepStrictEqual(new Boolean(true), test.toObject(false));
assert.notDeepStrictEqual(new Boolean(false), test.toObject(true));
assert.notDeepStrictEqual(test.toObject(false), false);
assert.notDeepStrictEqual(test.toObject(true), true);
assert.notDeepStrictEqual(test.toObject(''), '');
assert.notDeepStrictEqual(test.toObject(0), 0);

assert(!Number.isNaN(test.toObject(Number.NaN)));

assert.strictEqual('', test.toString(''));
assert.strictEqual('test', test.toString('test'));
assert.strictEqual('undefined', test.toString(undefined));
assert.strictEqual('null', test.toString(null));
assert.strictEqual('false', test.toString(false));
assert.strictEqual('true', test.toString(true));
assert.strictEqual('0', test.toString(0));
assert.strictEqual('1.1', test.toString(1.1));
assert.strictEqual('NaN', test.toString(Number.NaN));
assert.strictEqual('[object Object]', test.toString({}));
assert.strictEqual('test', test.toString({
  toString: function() { return 'test' }
}));
assert.strictEqual('', test.toString([]));
assert.strictEqual('1,2,3', test.toString([ 1, 2, 3 ]));
