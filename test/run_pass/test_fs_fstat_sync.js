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

var testfile = process.cwd() + "/run_pass/test_fs_fstat_sync.js";
var testdir = process.cwd() + "/resources";
var flags = "r";


// fstatSync - file
try {
  var fd = fs.openSync(testfile, flags);
  var stat = fs.fstatSync(fd);

  assert.equal(stat.isFile(), true);
  assert.equal(stat.isDirectory(), false);
  fs.closeSync(fd);
} catch (err) {
  throw err;
}


// fstatSync - directory
try {
  var fd = fs.openSync(testdir, flags);
  var stat = fs.fstatSync(fd);

  assert.equal(stat.isFile(), false);
  assert.equal(stat.isDirectory(), true);
  fs.closeSync(fd);
} catch (err) {
  throw err;
}
