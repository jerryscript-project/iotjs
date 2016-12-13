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

 /*
 @STDOUT=Pass
 */

var fs = require('fs');
var assert = require('assert');

var file1 = "../resources/rename.txt";
var file2 = "../resources/rename.txt.async";

fs.rename(file1, file2, function(err) {
  assert.equal(err, null);
  assert.equal(fs.existsSync(file1), false);
  assert.equal(fs.existsSync(file2), true);
  fs.rename(file2, file1, function(err) {
    assert.equal(err, null);
    console.log("Pass");
  });
});
