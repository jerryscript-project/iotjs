/* Copyright 2015-present Samsung Electronics Co., Ltd. and other contributors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

var assert = require('assert');

function decodeError(string, encode)
{
  try {
    new Buffer(string, encode);
  } catch (e) {
    assert.assert(e instanceof TypeError);
  }
}

function testWrite(input, string, offset, length,
                   encoding, expected, expectedLength)
{
  var buf = new Buffer(input);

  assert.equal(buf.write(string, offset, length, encoding), expectedLength);
  assert.equal(buf.toString(), expected);
}

/* Base64 tests. */

assert.equal((new Buffer('YnVmZg==', 'base64')).toString(), 'buff');
assert.equal((new Buffer('YnVmZmU=', 'base64')).toString(), 'buffe');
assert.equal((new Buffer('YnVmZmVy', 'base64')).toString(), 'buffer');
assert.equal((new Buffer('KiAxMjM0ICo=', 'base64')).toString(), '* 1234 *');
assert.equal((new Buffer('f39/fw==', 'base64')).toString(),
             '\u007f\u007f\u007f\u007f');
assert.equal((new Buffer('fn5+fg==', 'base64')).toString(),
             '\u007e\u007e\u007e\u007e');

decodeError('Yn=mZg==', 'base64');
decodeError('YnVmZ===', 'base64');
decodeError('YnVmZm', 'base64');
decodeError('KiAx*jM0ICo=', 'base64');

testWrite('xxxxxxxx', 'MTIzNA==', 2, 16, 'base64', 'xx1234xx', 4);
testWrite('xxxxxxxx', 'MTIzNA==', 2, 0, 'base64', 'xxxxxxxx', 0);
testWrite('xxxxxxxx', 'MTIzNA==', 3, 2, 'base64', 'xxx12xxx', 2);
testWrite('xxxx', 'MTIzNA==', 2, 16, 'base64', 'xx12', 2);

assert.throws(function () {
  /* Malformed string must throw error regardless of the buffer length. */
  testWrite('xxxxxxxx', 'MTIzNA=!', 2, 2, 'base64', 'xx12xxxx');
});


assert.equal((new Buffer('buff')).toString('base64'), 'YnVmZg==');
assert.equal((new Buffer('buffe')).toString('base64'), 'YnVmZmU=');
assert.equal((new Buffer('buffer')).toString('base64'), 'YnVmZmVy');
assert.equal((new Buffer('\u007f\u007f\u007f\u007f')).toString('base64'),
             'f39/fw==');
assert.equal((new Buffer('\u007e\u007e\u007e\u007e')).toString('base64'),
             'fn5+fg==');

assert.equal((new Buffer('**buffer**')).toString('base64', 2, 7), 'YnVmZmU=');


/* Hex tests. */

assert.equal((new Buffer('6768696A6b6c6D6e6f70', 'hex')).toString(),
             'ghijklmnop');
assert.equal((new Buffer('2a20427546663352202a', 'hex')).toString(),
             '* BuFf3R *');
assert.equal((new Buffer('eFbfBf', 'hex')).toString(), '\uffff');

decodeError('0*12', 'hex');
decodeError('0fe', 'hex');
decodeError('0g', 'hex');

testWrite('xxxxxxxx', '31323334', 2, 16, 'hex', 'xx1234xx', 4);
testWrite('xxxxxxxx', '31323334', 2, 0, 'hex', 'xxxxxxxx', 0);
testWrite('xxxxxxxx', '31323334', 3, 2, 'hex', 'xxx12xxx', 2);
testWrite('xxxx', '31323334', 2, 16, 'hex', 'xx12', 2);

assert.throws(function () {
  /* Malformed string must throw error regardless of the buffer length. */
  testWrite('xxxxxxxx', '3132333g', 2, 2, 'base64', 'xx12xxxx');
});

assert.equal((new Buffer('ghijklmnop')).toString('hex'),
             '6768696a6b6c6d6e6f70');

assert.equal((new Buffer('ghijklmnop')).toString('hex', 2, 8),
             '696a6b6c6d6e');
