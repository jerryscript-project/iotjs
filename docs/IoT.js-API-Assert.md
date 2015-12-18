Assert module is for writing test for applications.

## AssertionError
Assert module will produce `AssertionError` in case of assertion failure. `AssertionError` inherits standard `Error` thus it has properties provided by `Error` object including additional properties.

### Properties

#### name
The name of `AssertionError` is 'AssertionError` this is fixed.

#### actual
This property contains the value what actually be.

#### expected
This property contains the value what expected is.

#### operator
This property contains the operator used for evaluating `actual` with `expected`.

#### message
The error message.

## Assert
You can get assert module by `require('assert')` the module provide.

#### assert.fail(actual, expected, message, operator)
Throw an `AssertionError` exception.

#### assert(value, message)
Test if `value` is true, throws an `AssertionError` exception if `value` evaluates `false`.

#### assert.equal(actual, expected, message)
Test if `actual == expected` evaluates `true`, if not, throws an exception.

#### assert.notEqual(actual, expected, message)
Test if `actual != expected` evaluates `true`, if not, throws an exception.

#### assert.strictEqual(actual, expected, message)
Test if `actual === expected` evaluates `true`, if not, throws an exception.

#### assert.notStrictEqual(actual, expected, message)
Test if `actual !== expected` evaluates `true`, if not, throws an exception.

#### assert.throws(block, expected, message)
Test if the given `block` throws `expected` error, if not, throws an exception.

#### assert.doesNotThrows(block, message)
Test if the given `block` does not throws any exception, if not, throws an exception.
