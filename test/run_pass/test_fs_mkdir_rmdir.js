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

var testRoot = process.cwd() + "/resources";

// TizenRT tests are performed from ROM
// Files should be stored in other path
if (process.platform === 'tizenrt') {
  testRoot = "/mnt";
}

function unlink(path) {
  try {
    fs.rmdirSync(path);
  } catch (e) {
  }
  assert.equal(fs.existsSync(path), false);
}

{
  var root = testRoot + "/test_dir";
  var sub1 = root + "/file1";
  var sub2 = root + "/file2";

  unlink(sub1);
  unlink(sub2);
  unlink(root);

  fs.mkdir(root, function(err) {
    assert.equal(err, null);
    assert.equal(fs.existsSync(root), true);
    assert.equal(fs.mkdirSync(sub1), undefined);
    assert.equal(fs.mkdirSync(sub2), undefined);
    assert.equal(fs.existsSync(sub1), true);
    assert.equal(fs.existsSync(sub2), true);

    unlink(sub1);
    unlink(sub2);

    fs.rmdir(root, function() {
      assert.equal(fs.existsSync(root), false);
      var root2 = testRoot + "/test_dir2";

      fs.mkdir(root2, 777, function(err) {
        assert.equal(err, null);
        assert.equal(fs.existsSync(root2), true);

        fs.rmdir(root2, function() {
          assert.equal(fs.existsSync(root2), false);
        });

        // Run read-only directory test only on linux and Tizen
        // NuttX does not support read-only attribute.
        if (process.platform === 'linux' || process.platform === 'tizen') {
          var testMode = '0444';
          fs.mkdir(root, testMode, function(err) {
            assert.equal(err, null);
            assert.equal(fs.existsSync(root), true);

            var mode = fs.statSync(root).mode;
            assert.strictEqual(mode.toString(8).slice(-4), testMode);

            fs.rmdir(root, function() {
              assert.equal(fs.existsSync(root), false);
            });
          });
        }
      });
    });
  });
}
