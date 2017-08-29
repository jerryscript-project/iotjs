### Platform Support

The following shows Assert module APIs available for each platform.

|  | Linux<br/>(Ubuntu) | Raspbian<br/>(Raspberry Pi) | NuttX<br/>(STM32F4-Discovery) | TizenRT<br/>(Artik053) |
| :---: | :---: | :---: | :---: | :---: |
| assert.assert | O | O | O | - |
| assert.doesNotThrow | O | O | O | - |
| assert.equal | O | O | O | - |
| assert.fail | O | O | O | - |
| assert.notEqual | O | O | O | - |
| assert.notStrictEqual | O | O | O | - |
| assert.strictEqual | O | O | O | - |
| assert.throws | O | O | O | - |

# Assert

Assert module is designed for writing tests for applications.

You can access the functions of the module by adding `require('assert')` to your file.


## Class: AssertionError

Assert module will produce `AssertionError` in case of an assertion failure. `AssertionError` inherits standard `Error` thus it has properties provided by `Error` object including additional properties.


* `actual` {any} This property contains the actual value.
* `expected` {any} This property contains the expected value.
* `message` {any} The error message, default value is the error itself.
* `name` {string} The name is `AssertionError` string.
* `operator` {string} This property contains the operator used for comparing `actual` with `expected`.


### assert(value[, message])
* `value` {any} Value to test.
* `message` {any} Message displayed in the thrown error.

Checks if the `value` is truthy. If it is not, throws an AssertionError, with the given optional `message`.

**Example**

```js
var assert = require('assert');

assert.assert(1);
// OK

assert.assert(true);
// OK

assert.assert(false);
// throws "AssertionError: false == true"

assert.assert(0);
// throws "AssertionError: 0 == true"

assert.assert(false, "it's false");
// throws "AssertionError: it's false"
```


### doesNotThrow(block[, message])
* `block` {Function}
* `message` {any} Message to be displayed.

Tests if the given `block` does not throw any exception. Otherwise throws an
exception with the given optional `message`.

**Example**

```js
var assert = require('assert');

assert.doesNotThrow(
  function() {
    assert.assert(1);
  }
);
// OK

assert.doesNotThrow(
  function() {
    assert.assert(0);
  }
)
// throws "AssertionError: Got unwanted exception."
```


### equal(actual, expected[, message])
* `actual` {any} The actual value.
* `expected` {any} The expected value.
* `message` {any} Message to be displayed.

Tests if `actual == expected` is evaluated to `true`. Otherwise throws an
exception with the given optional `message`.

**Example**

```js
var assert = require('assert');

assert.equal(1, 1);
assert.equal(1, '1');
```


### fail(actual, expected, message, operator)
* `actual` {any} The actual value.
* `expected` {any} The expected value.
* `message` {any} Message to be displayed.
* `operator` {string} The operator.

Throws an `AssertionError` exception with the given `message`.

**Example**

```js
var assert = require('assert');

assert.fail(1, 2, undefined, '>');
// AssertionError: 1 > 2
```


### notEqual(actual, expected[, message])
* `actual` {any} The actual value.
* `expected` {any} The expected value.
* `message` {any} Message to be displayed.

Tests if `actual != expected` is evaluated to `true`. Otherwise throws an
exception with the given optional `message`.

**Example**

```js
var assert = require('assert');

assert.notEqual(1, 2);
```


### notStrictEqual(actual, expected[, message])
* `actual` {any} The actual value.
* `expected` {any} The expected value.
* `message` {any} Message to be displayed.

Tests if `actual !== expected` is evaluated to `true`. Otherwise throws an exception
with the given optional `message`.

**Example**

```js
var assert = require('assert');

assert.notStrictEqual(1, 2);
// OK

assert.notStrictEqual(1, 1);
// AssertionError: 1 !== 1

assert.notStrictEqual(1, '1');
// OK
```


### strictEqual(actual, expected[, message])
* `actual` {any} The actual value.
* `expected` {any} The expected value.
* `message` {any} Message to be displayed.

Tests if `actual === expected` is evaluated to `true`. Otherwise throws an exception
with the given optional `message`.

**Example**

```js
var assert = require('assert');

assert.strictEqual(1, 1);
// OK

assert.strictEqual(1, 2);
// AssertionError: 1 === 2

assert.strictEqual(1, '1');
// AssertionError: 1 === '1'
```


### throws(block[, expected, message])
* `block` {Function} The function that throws an error.
* `expected` {Function} The expected error type.
* `message` {any} Message to be displayed.

Tests if the given `block` throws an `expected` error. Otherwise throws an exception
with the given optional `message`.

**Example**

```js
var assert = require('assert');

assert.throws(
  function() {
    assert.equal(1, 2);
  },
  assert.AssertionError
);
// OK

assert.throws(
  function() {
    assert.equal(1, 1);
  },
  assert.AssertionError
);
// Uncaught error: Missing exception

assert.throws(
  function() {
    assert.equal(1, 2);
  },
  TypeError
);
// AssertionError
```
