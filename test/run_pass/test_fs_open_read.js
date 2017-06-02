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


var fileName = process.cwd() + "/resources/greeting.txt";
var expectedContents = "Hello IoT.js!!";
var flags = "r";
var mode = 438;


// test async open & read
fs.open(fileName, flags, mode, function(err, fd) {
  if (err) {
    throw err;
  } else {
    var buffer = new Buffer(64);
    fs.read(fd, buffer, 0, buffer.length, 0, function(err, bytesRead, buffer) {
      if (err) {
        throw err;
      } else {
        assert.equal(buffer.toString(), expectedContents);
      }
    });
  }
});


// error test
assert.throws(
  function() {
    fs.openSync("non_exist_file", flags, mode);
  },
  Error
);

// test the position argument of fs.read()
fs.open(fileName, flags, function(err, fd) {
  if (err) {
    throw err;
  }
  var buffer = new Buffer(64);
  fs.read(fd, buffer, 0, 7, null, function(err, bytesRead, buffer) {
    if (err) {
      throw err;
    }
    fs.read(fd, buffer, 7, 7, null, function(err, bytesRead, buffer) {
      if (err) {
        throw err;
      }
      assert.equal(buffer.toString(), expectedContents);
      fs.closeSync(fd);
    });
  });
});
