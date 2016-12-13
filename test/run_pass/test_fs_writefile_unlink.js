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

var file1 = "../resources/tobeornottobe.txt";
var file2 = "../resources/tobeornottobe_async.txt";

fs.readFile(file1, function(err, buf1) {
  assert.equal(err, null);
  fs.writeFile(file2, buf1, function(err) {
    assert.equal(err, null);
    fs.exists(file2, function(exists) {
      assert.equal(exists, true);
      fs.readFile(file1, function(err, buf2) {
        assert.equal(err, null);
        assert.equal(buf1.toString(), buf2.toString());
        fs.unlink(file2, function(err) {
          assert.equal(err, null);
          fs.exists(file2, function(exists) {
            assert.equal(exists, false);
            console.log("Pass");
          });
        });
      });
    });
  });
});
