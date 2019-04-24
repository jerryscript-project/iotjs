'use strict';
var assert = require('assert');
var common = require('common.js');

// testing api calls for symbol
var test_symbol = require('./build/' + common.buildTypePath +
                          '/test_napi_symbol.node');

var sym = test_symbol.CreateSymbol('test');
assert.strictEqual(sym.toString(), 'Symbol(test)');

{
  var myObj = {};
  var fooSym = test_symbol.CreateSymbol('foo');
  var otherSym = test_symbol.CreateSymbol('bar');
  myObj.foo = 'bar';
  myObj[fooSym] = 'baz';
  myObj[otherSym] = 'bing';
  assert.strictEqual(myObj.foo, 'bar');
  assert.strictEqual(myObj[fooSym], 'baz');
  assert.strictEqual(myObj[otherSym], 'bing');
}

{
  var fooSym = test_symbol.CreateSymbol('foo');
  var myObj = {};
  myObj.foo = 'bar';
  myObj[fooSym] = 'baz';
  Object.keys(myObj); // -> [ 'foo' ]
  Object.getOwnPropertyNames(myObj); // -> [ 'foo' ]
  Object.getOwnPropertySymbols(myObj); // -> [ Symbol(foo) ]
  assert.strictEqual(Object.getOwnPropertySymbols(myObj)[0], fooSym);
}

{
  assert.notStrictEqual(test_symbol.CreateSymbol(), test_symbol.CreateSymbol());
  assert.notStrictEqual(test_symbol.CreateSymbol('foo'),
                        test_symbol.CreateSymbol('foo'));
  assert.notStrictEqual(test_symbol.CreateSymbol('foo'),
                        test_symbol.CreateSymbol('bar'));

  var foo1 = test_symbol.CreateSymbol('foo');
  var foo2 = test_symbol.CreateSymbol('foo');
  var object = {
    [foo1]: 1,
    [foo2]: 2,
  };
  assert.strictEqual(object[foo1], 1);
  assert.strictEqual(object[foo2], 2);
}
