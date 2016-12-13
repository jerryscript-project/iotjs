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



var fs = require('fs');
var assert = require('assert');


var srcFilePath = "../resources/test1.txt";
var dstFilePath = "../tmp/test_fs1.txt";

try {
  var fd1 = fs.openSync(srcFilePath, 'r');
  var buffer = new Buffer(128);
  var bytes1 = fs.readSync(fd1, buffer, 0, buffer.length, 0);
  fs.closeSync(fd1);

  var fd2 = fs.openSync(dstFilePath, 'w');
  var bytes2 = fs.writeSync(fd2, buffer, 0, bytes1, 0);
  fs.closeSync(fd2);

  assert.equal(bytes1, bytes2);

  var fd3 = fs.openSync(srcFilePath, 'r');
  var bytes3 = fs.readSync(fd3, buffer, 0, buffer.length, 0);
  fs.closeSync(fd3);

  assert.equal(bytes1, bytes3);

  var result = 'TEST File Read & Write\n';

  assert.equal(buffer.toString(), result);
} catch (err) {
  throw err;
}
