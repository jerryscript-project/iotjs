/* Copyright 2017-present Samsung Electronics Co., Ltd. and other contributors
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

// Copyright Joyent, Inc. and other Node contributors.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to permit
// persons to whom the Software is furnished to do so, subject to the
// following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
// NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
// OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
// USE OR OTHER DEALINGS IN THE SOFTWARE.

'use strict';

var common = require('node/common');
var assert = require('assert');
var http = require('http');

// Verify that ServerResponse.writeHead() works as setHeader.
// Issue 5036 on github.

var s = http.createServer(common.mustCall(function (req, res) {
  res.setHeader('test', '1');

  // toLowerCase() is used on the name argument, so it must be a string.
  var threw = false;
  try {
    res.setHeader(0xf00, 'bar');
  } catch (e) {
    assert.ok(e instanceof TypeError);
    threw = true;
  }
  assert.ok(threw, 'Non-string names should throw');

  // undefined value should throw, via 979d0ca8
  threw = false;
  try {
    res.setHeader('foo', undefined);
  } catch (e) {
    assert.ok(e instanceof Error);
    assert.strictEqual(e.message, 'value required in setHeader(foo, value)');
    threw = true;
  }
  assert.ok(threw, 'Undefined value should throw');

  res.writeHead(200, { Test: '2' });

  // assert.throws(function () {
  assert.doesNotThrow(function () {
    res.writeHead(100, {});
  });

  res.end();
}));

s.listen(0, common.mustCall(runTest));

function runTest() {
  http.get({ port: this.address().port }, common.mustCall(function (response) {
    response.on('end', common.mustCall(function () {
      assert.strictEqual(response.headers['test'], '1' /*'2'*/);
      // assert.notStrictEqual(response.rawHeaders.indexOf('Test'), -1);
      s.close();
    }));
    response.resume();
  }));
}
