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


var srcFilePath = process.cwd() + "/resources/test1.txt";
var dstFilePath = process.cwd() + "/tmp/test_fs2.txt";

// TizenRT tests are performed from ROM
// Files should be stored in other path
if (process.platform === 'tizenrt') {
  dstFilePath = "/mnt/test_fs2.txt";

  // Prepare test environment
  if (fs.existsSync(dstFilePath)) {
    fs.unlinkSync(dstFilePath);
  }
}

function cleanup() {
  if (process.platform === 'tizenrt') {
    fs.unlinkSync(dstFilePath);
  }
}

var result = 'TEST File Read & Write\n';
var data;

function onOpenForWrite(err, fd) {
  assert.equal(err, null, 'Failed to open for write file:' + err);

  fs.write(fd, data, 0, data.length, function (err, written, buffer) {
    assert.equal(err, null, 'Failed to write file:' + err);
    fs.closeSync(fd);

    var fdr = fs.openSync(dstFilePath, 'r');
    var buffer = new Buffer(128);
    fs.readSync(fdr, buffer, 0, buffer.length, 0);
    fs.closeSync(fdr);

    cleanup();

    assert.equal(buffer.toString(), result,
                 'Read/write content does not match');
  });
}

function onRead(err, bytesRead, buffer) {
  assert.equal(err, null, 'Failed to read file:' + err);
  data = new Buffer(buffer.toString());
  fs.open(dstFilePath, 'w', onOpenForWrite);
}

function onOpenForRead(err, fd) {
  assert.equal(err, null, 'Failed to open for read file:' + err);
  var buffer = new Buffer(128);
  fs.read(fd, buffer, 0, buffer.length, 0, onRead);
}

fs.open(srcFilePath, 'r', onOpenForRead);
