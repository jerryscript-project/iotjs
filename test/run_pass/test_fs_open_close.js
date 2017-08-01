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

var fileName = process.cwd() + '/resources/greeting.txt';

// test sync open & close.

// test normal sequence.
assert.doesNotThrow(function() {
  var fd = fs.openSync(fileName, 'r');
  assert.notEqual(fs, null, 'Error opening file: ' + fileName);
  fs.closeSync(fd);
}, 'Test normal sequence. Open file for reading');


// test trying to open not exist file - expecting exception.
assert.throws(function() {
  var fd = fs.openSync('not_exist_file', 'r');
}, Error, 'Test trying to openSync not exist file - expecting exception');


// test trying to close with bad fd - expecting exception.
assert.throws(function() {
  fs.closeSync(-1);
}, Error, 'Test trying to close with bad fd - expecting exception');

// test async open & close.

// test normal sequence.
fs.open(fileName, 'r', function(err, fd) {
  assert.equal(err, null,
               'Async Normal: Error opening ' + fileName + ': ' + err);

  fs.close(fd, function(err) {
    assert.equal(err, null,
                 'Async Normal: Error closing ' + fileName + ': ' + err);
  });
});


// test not exist file - expecting exception.
fs.open('not_exist_file', 'r', function(err, fd) {
  assert.notEqual(err, null, 'Test not exist file - expecting exception');
});


// test trying to close with bad fd - expecting exception.
fs.close(-1, function(err) {
  assert.notEqual(err, null,
                  'Test trying to close with bad fd - expecting exception');
});

{
  var buffer = new Buffer(10);
  // expect length out of bound
  assert.throws(function () { fs.readSync(5, buffer, 0, 20); }, RangeError,
                'Expect length out of bound 0, 20');
  // expect offset out of bound
  assert.throws(function () { fs.readSync(5, buffer, -1, 20); }, RangeError,
                'Expect length out of bound -1, 20');
}

fs.open(fileName, 'r', '4' , function(err, fd) {
  assert.equal(err, null,
               'Async Normal2: Error opening ' + fileName + ': ' + err);

  fs.close(fd, function(err) {
    assert.equal(err, null,
                 'Async Normal2: Error closing ' + fileName + ': ' + err);
    });
});

assert.throws(function() {
   var fd = fs.openSync(null, 123);
}, TypeError, 'Calling fs.openSync with null file path');
assert.throws(function() {
  var fd = fs.openSync(process.cwd() + '/run_pass/test_fs_stat.js', 'k');
}, TypeError, 'Calling fs.openSync with invalid flag');
assert.throws(function() {
  var fd = fs.openSync(process.cwd() + '/resources/test2.txt', null);
}, TypeError, 'Calling fs.openSync with null flag');


var testRoot = process.cwd() + '/resources';
var readFile = testRoot + '/test2.txt';

// TizenRT tests are performed from ROM
// Files should be stored in other path
if (process.platform === 'tizenrt') {
  testRoot = '/mnt';
  readFile = testRoot + '/test2.txt';

  // Create file for read test
  fs.closeSync(fs.openSync(readFile, 'w'));
}

('rs sr r+ rs+ sr+ a a+')
  .split(' ').forEach(function (flag) {
    assert.doesNotThrow(function () {
      var fd = fs.openSync(readFile, flag);
      fs.closeSync(fd);
    }, 'File could not be opened with flag: ' + flag);
  });

// Cleanup after test
if (process.platform === 'tizenrt') {
  fs.unlinkSync(readFile);
}

('wx xw w+ wx+ xw+ ax+ xa+ ax xa')
  .split(' ').forEach(function (flag) {
    assert.doesNotThrow(function() {
      var file = testRoot + '/TEMP' + flag + '.txt';
      var fd = fs.openSync(file, flag);
      fs.closeSync(fd);
      fs.unlinkSync(file);
    }, 'File could not be opened with flag: ' + flag);
  });
