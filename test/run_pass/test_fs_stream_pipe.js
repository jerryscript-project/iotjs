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

var inputFileName = process.cwd() + '/resources/tobeornottobe.txt';
var outputFileName = process.cwd() + '/tmp/test_fs4.txt';

var buff1 = new Buffer(2048);
var buff2 = new Buffer(2048);

// Get the correct content of the input for later checking.
fs.open(inputFileName, 'r', 0666, function(err, fd) {
  if (err) {
    throw err;
  }

  fs.read(fd, buff1, 0, buff1.length, 0, function(err, bytesRead, buffer) {
    if (err) {
      throw err;
    }

    fs.close(fd, onclose);
  });
});

function onclose(err) {
  if (err) {
    throw err;
  }

  var readableFileStream = fs.createReadStream(inputFileName);
  var writableFileStream = fs.createWriteStream(outputFileName);

  writableFileStream.on('ready', function() {
    readableFileStream.pipe(writableFileStream);
  });

  writableFileStream.on('close', check_output);
}

function check_output() {
  // Read the output for checking.
  fs.open(outputFileName, 'r', function(err, fd) {
    if (err) {
      throw err;
    }

    fs.read(fd, buff2, 0, buff2.length, 0, function(err, bytesRead, buffer) {
      if (err) {
        throw err;
      }

      fs.close(fd, function(err) {
        if (err) {
          throw err;
        }
      })
    });
  });
}

process.on('exit', function() {
  assert.equal(buff1.toString(), buff2.toString(),
               'File contents do not match');
});
