/* Copyright 2018-present Samsung Electronics Co., Ltd. and other contributors
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

if (fs.createReadStream === undefined || fs.createWriteStream === undefined) {
  process.exit(0);
}

var dir = (process.platform === 'tizenrt') ? '/mnt/' : process.cwd() + '/tmp/';
var outputFile = dir + 'test_fs5.txt';
var testData = 'WriteStream test';

var writableFileStream = fs.createWriteStream(outputFile);

writableFileStream.on('ready', function() {
  writableFileStream.write(testData);
});

var buff = new Buffer(64);
writableFileStream.on('close', function() {
  // Check output correctness
  fs.open(outputFile, 'r', 0666, function(err, fd) {
    if (err) {
      throw err;
    }

    fs.read(fd, buff, 0, buff.length, 0, function(err, bytesRead, buffer) {
      if (err) {
        throw err;
      }

      assert.equal(buff.toString(), testData, 'Incorrect data in output file');

      fs.close(fd, function(err) {
        if (err) {
          throw err;
        }
      });
    });
  })
});
