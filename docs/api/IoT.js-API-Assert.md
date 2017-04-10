### Platform Support

The following shows Assert module APIs available for each platform.

|  | Linux<br/>(Ubuntu) | Raspbian<br/>(Raspberry Pi) | Nuttx<br/>(STM32F4-Discovery) |
| :---: | :---: | :---: | :---: |
| assert.assert | O | O | O |
| assert.doesNotThrows | O | O | O |
| assert.equal | O | O | O |
| assert.fail | O | O | O |
| assert.notEqual | O | O | O |
| assert.notStrictEqual | O | O | O |
| assert.strictEqual | O | O | O |
| assert.throws | O | O | O |


### Contents

- [Assert](#assert)
    - [Module Functions](#module-functions)
        - [`assert.assert(value[, message])`](#assertassertvalue-message)
        - [`assert.doesNotThrows(block[, message])`](#assertdoesnotthrowsblock-message)
        - [`assert.equal(actual, expected[, message])`](#assertequalactual-expected-message)
        - [`assert.fail(actual, expected, message, operator)`](#assertfailactual-expected-message-operator)
        - [`assert.notEqual(actual, expected[, message])`](#assertnotequalactual-expected-message)
        - [`assert.notStrictEqual(actual, expected[, message])`](#assertnotstrictequalactual-expected-message)
        - [`assert.strictEqual(actual, expected[, message])`](#assertstrictequalactual-expected-message)
        - [`assert.throws(block[, expected, message])`](#assertthrowsblock-expected-message)
    - [Class: assert.AssertionError](#class-assertassertionerror)
        - [Properties](#properties)
            - [`actual`](#actual)
            - [`message`](#message)
            - [`expected`](#expected)
            - [`name`](#name)
            - [`operator`](#operator)


# Assert

Assert module is for writing test for applications.

You can get assert module by `require('assert')` the module provide.


## Module Functions


### `assert.assert(value[, message])`
* `value <any>`
* `message <any>`

Test if `value` is true, it throws an `AssertionError` exception if `value` evaluates `false`.

**Example**

```js
var assert = require('assert');
assert.assert(1 == 1);
```


### `assert.doesNotThrows(block[, message])`
* `value <Function()>`
* `message <any>`

Test if the given `block` does not throw any exception, if not, it throws an exception.

**Example**

```js
var assert = require('assert');
assert.doesNotThrow(
  function() {
    assert.assert(1 == 1);
  }
);
```


### `assert.equal(actual, expected[, message])`
* `actual <any>`
* `expected <any>`
* `message <any>`

Test if `actual == expected` is evaluated to `true`, if not, it throws an exception.

**Example**

```js
var assert = require('assert');
assert.equal(1, 1);
assert.equal(1, '1');
```


### `assert.fail(actual, expected, message, operator)`
* `actual <any>`
* `expected <any>`
* `message <any>`
* `operator <String>`

Throw an `AssertionError` exception.

**Example**

```js
var assert = require('assert');
assert.fail(1, 2, undefined, '>'); // AssertionError: 1 > 2
```


### `assert.notEqual(actual, expected[, message])`
* `actual <any>`
* `expected <any>`
* `message <any>`

Test if `actual != expected` is evaluated to `true`, if not, it throws an exception.

**Example**

```js
var assert = require('assert');
assert.notEqual(1, 2);
```


### `assert.notStrictEqual(actual, expected[, message])`
* `actual <any>`
* `expected <any>`
* `message <any>`

Test if `actual !== expected` is evaluated to `true`, if not, it throws an exception.

**Example**

```js
var assert = require('assert');
assert.notStrictEqual(0, false);
```


### `assert.strictEqual(actual, expected[, message])`
* `actual <any>`
* `expected <any>`
* `message <any>`

Test if `actual === expected` is evaluated to `true`, if not, it throws an exception.

**Example**

```js
var assert = require('assert');
assert.strictEqual(1, 1);
```


### `assert.throws(block[, expected, message])`
* `actual <any>`
* `expected <any>`
* `message <any>`

Test if the given `block` throws an `expected` error, if not, it throws an exception.

**Example**

```js
var assert = require('assert');
assert.throws(
  function() {
    assert.equal(1, 2);
  },
  assert.AssertionError
);
```


# Class: assert.AssertionError

Assert module will produce `AssertionError` in case of assertion failure. `AssertionError` inherits standard `Error` thus it has properties provided by `Error` object including additional properties.


## Properties


### `actual`
* `<any>`
This property contains the actual value.


### `message`
* `<any>`
The error message.


### `expected`
* `<any>`
This property contains the expected value.


### `name`
* `<String>`
The name of `AssertionError` is 'AssertionError` this is fixed.


### `operator`
* `<String>`
This property contains the operator used for evaluating `actual` with `expected`.