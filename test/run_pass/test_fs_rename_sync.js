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

var file1 = process.cwd() + '/resources/rename.txt';
var file2 = process.cwd() + '/resources/rename.txt.sync';

// TizenRT tests are performed from ROM
// Files should be stored in other path
if (process.platform === 'tizenrt') {
  file1 = '/mnt/rename.txt';
  file2 = '/mnt/rename.txt.sync';

  fs.closeSync(fs.openSync(file1, 'w'));
}

assert.equal(fs.existsSync(file1), true,
             'Test prerequisites: checking existance of ' + file1);

fs.renameSync(file1, file2);
assert.equal(fs.existsSync(file1), false, 'Source file exist after renaming');
assert.equal(fs.existsSync(file2), true,
             'Destination file not exist after renaming');
fs.renameSync(file2, file1);

// Cleanup after test
if (process.platform === 'tizenrt') {
  fs.unlinkSync(file1);
}
