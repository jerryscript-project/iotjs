/* Copyright 2016-present Samsung Electronics Co., Ltd. and other contributors
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

{
  var filePath = process.cwd() + '/resources/tobeornottobe.txt';

  fs.exists(filePath, function(exists) {
    assert.equal(exists, true, 'File should exist: ' + filePath);
  });
}

{
  var filePath = '';

  assert.doesNotThrow(function() {
    fs.exists(filePath, function (exists) {
      assert.equal(exists, false, 'File with empty name should not exist');
    });
  }, 'Checking fs.exists(\'\') without callback does not throw exception');

  assert.doesNotThrow(function() {
    fs.exists(filePath);
  }, 'Checking fs.exists(\'\') without callback does not throw exception');
}

{
  var filePath = process.cwd() + '/resources/tobeornottobe.txt';

  assert.doesNotThrow(function() {
    fs.exists(filePath);
  }, 'Checking fs.exists() for existing file does not throw exception');
}

{
  var filePath = process.cwd() + '/resources/empty.txt';

  fs.exists(filePath, function(exists) {
    assert.equal(exists, false, 'File should not exist' + filePath);
  });
}

{
  var filePath = '';

  fs.exists(filePath, function(exists) {
    assert.equal(exists, false, 'File with empty name should not exist');
  });
}

{
  var filePath = ' ';

  fs.exists(filePath, function(exists) {
    assert.equal(exists, false, 'File name with single whitespace check');
  });
}
