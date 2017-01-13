## Module: Assert

Assert module is for writing test for applications.

## AssertionError
Assert module will produce `AssertionError` in case of assertion failure. `AssertionError` inherits standard `Error` thus it has properties provided by `Error` object including additional properties.

### Properties


#### actual
This property contains the actual value.


#### message
The error message.


#### expected
This property contains the expected value.


#### name
The name of `AssertionError` is 'AssertionError` this is fixed.


#### operator
This property contains the operator used for evaluating `actual` with `expected`.


## Assert
You can get assert module by `require('assert')` the module provide.


#### assert(value, message)
Test if `value` is true, it throws an `AssertionError` exception if `value` evaluates `false`.


#### assert.doesNotThrows(block, message)
Test if the given `block` does not throw any exception, if not, it throws an exception.


#### assert.equal(actual, expected, message)
Test if `actual == expected` is evaluated to `true`, if not, it throws an exception.


#### assert.fail(actual, expected, message, operator)
Throw an `AssertionError` exception.


#### assert.notEqual(actual, expected, message)
Test if `actual != expected` is evaluated to `true`, if not, it throws an exception.


#### assert.notStrictEqual(actual, expected, message)
Test if `actual !== expected` is evaluated to `true`, if not, it throws an exception.


#### assert.strictEqual(actual, expected, message)
Test if `actual === expected` is evaluated to `true`, if not, it throws an exception.


#### assert.throws(block, expected, message)
Test if the given `block` throws an `expected` error, if not, it throws an exception.
