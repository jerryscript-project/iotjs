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

var dstFilePath = process.cwd() + '/tmp/test_fs3.txt';

if (process.platform === 'tizenrt') {
  dstFilePath = '/mnt/test_fs3.txt';
}

var buff1 = new Buffer('IoT');
var buff2 = new Buffer('.js');

fs.open(dstFilePath, 'w+', function(err, fd) {
  if (err) {
    throw err;
  }

  // Test the position argument of fs.write().
  fs.write(fd, buff1, 0, buff1.length, undefined, function(err, bytes, buffer) {
    if (err) {
      throw err;
    }

    fs.write(fd, buff2, 0, buff2.length, null, function(err, bytes, buffer) {
      if (err) {
        throw err;
      }

      // Check the result.
      var result = new Buffer(6);
      fs.read(fd, result, 0, result.length, 0, function(err, bytes, buffer) {
        if (err) {
          throw err;
        }
        var init_buffers = Buffer.concat([buff1, buff2]);
        assert.assert(result.equals(init_buffers));

        fs.close(fd, function(err) {
          if (err) {
            throw err;
          }
        });
      });
    });
  });
});
