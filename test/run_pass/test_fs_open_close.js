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


// test sync open & close.

// test normal sequence.
try {
  var fd = fs.openSync(fileName, 'r');
  fs.closeSync(fd);
} catch (err) {
  throw err;
}


// test trying to open not exist file - expecting exception.
try {
  var fd = fs.openSync('not_exist_file', 'r');
  assert.fail('none', 'exception');
} catch (err) {
}


// test trying to close with bad fd - expecting exception.
try {
  fs.closeSync(-1);
  assert.fail('none', 'exception');
} catch (err) {
}


// test async open & close.

// test normal sequence.
var fs_async_normal_ok = false;
fs.open(fileName, 'r', function(err, fd) {
  if (err) {
    throw err;
  } else {
    fs.close(fd, function(err) {
      if (err) {
        throw err;
      } else {
        fs_async_normal_ok = true;
      }
    });
  }
});


// test not exist file - expecting exception.
var fs_async_open_not_exist_ok = false;
fs.open('not_exist_file', 'r', function(err, fd) {
  if (err) {
    fs_async_open_not_exist_ok = true;
  } else {
    assert.fail('none', 'exception');
  }
});


// test trying to close with bad fd - expecting exception.
var fs_async_close_bad_fd_ok = false;
fs.close(-1, function(err) {
  if (err) {
    fs_async_close_bad_fd_ok = true;
  } else {
    assert.fail('none', 'exception');
  }
});

var buffer = new Buffer(10);
// expect length out of bound
assert.throws(function () { fs.readSync(5, buffer, 0, 20); }, RangeError);
// expect offset out of bound
assert.throws(function () { fs.readSync(5, buffer, -1, 20); }, RangeError);

var fs_async_normal_ok2 = false;
fs.open(fileName, 'r', '4' ,function(err, fd) {
  if (err) {
    throw err;
  } else {
    fs.close(fd, function(err) {
      if (err) {
        throw err;
      } else {
        fs_async_normal_ok2 = true;
      }
    });
  }
});

process.on('exit', function() {
  assert.equal(fs_async_normal_ok, true);
  assert.equal(fs_async_normal_ok2, true);
  assert.equal(fs_async_open_not_exist_ok, true);
  assert.equal(fs_async_close_bad_fd_ok, true);
});


assert.throws (function() {
   var fd = fs.openSync(null, 123);
 }, TypeError);
assert.throws (function() {
  var fd = fs.openSync(process.cwd() + '/run_pass/test_fs_stat.js', 'k');
}, TypeError);
assert.throws (function() {
  var fd = fs.openSync(process.cwd() + '/resources/test2.txt', null);
}, TypeError);

('rs sr r+ rs+ sr+ a a+')
  .split(' ').forEach(function (flag){
    assert.doesNotThrow(function (){
      var fd = fs.openSync(process.cwd() + '/resources/test2.txt', flag);
      fs.closeSync(fd);
    }, 'file could not be opened with ' + flag);
  });

('wx xw w+ wx+ xw+ ax+ xa+ ax xa')
  .split(' ').forEach(function (flag){
    assert.doesNotThrow(function(){
      var file = process.cwd() + '/resources/TEMP' + flag + '.txt';
      var fd = fs.openSync(file, flag);
      fs.unlinkSync(file);
      fs.closeSync(fd);
    }, 'file could not be opened with ' + flag);
  });
