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
var dstFilePath = "../tmp/test_fs2.txt";

var data;

function onWrite(err, written, buffer) {
  if (err) {
    throw err;
  } else {
    var fd = fs.openSync(dstFilePath, 'r');
    var buffer = new Buffer(128);
    fs.readSync(fd, buffer, 0, buffer.length, 0);

    var result = 'TEST File Read & Write\n';

    assert.equal(buffer.toString(), result);
  }
}

function onOpenForWrite(err, fd) {
  if (err) {
    throw err;
  } else {
    fs.write(fd, data, 0, data.length, onWrite);
  }
}

function onRead(err, bytesRead, buffer) {
  if (err) {
    throw err;
  } else {
    data = new Buffer(buffer.toString());
    fs.open(dstFilePath, 'w', onOpenForWrite);
  }
}

function onOpenForRead(err, fd) {
  if (err) {
    throw err;
  } else {
    var buffer = new Buffer(128);
    fs.read(fd, buffer, 0, buffer.length, 0, onRead);
  }
}

fs.open(srcFilePath, 'r', onOpenForRead);
