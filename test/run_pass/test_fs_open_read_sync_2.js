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

var fs = require('fs');
var assert = require('assert');

var fileName = process.cwd() + "/resources/greeting.txt";
var expectedContents = "Hello IoT.js!!";
var flags = "r";
var mode = 438;


// test sync open & read
try {
  var fd = fs.openSync(fileName, flags, mode);
  var buffer = new Buffer(64);
  fs.readSync(fd, buffer, 0, buffer.length, 0);
  assert.equal(buffer.toString(), expectedContents);
} catch (err) {
  throw err;
}
