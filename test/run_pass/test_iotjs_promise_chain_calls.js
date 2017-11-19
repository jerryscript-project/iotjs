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

var fs = require('fs')
var assert = require('assert');

function readfile(fileName) {
  return new Promise(function (resolve, reject) {
    fs.readFile(fileName, function(error, data) {
      if (error) {
        reject(new Error('Cannot open file!'));
      } else {
        resolve(data.toString());
      }
    });
  });
};

function loadfi(files, idx) {
  var fileName = process.cwd()
                 + '/resources/promise_chain_calls/'
                 + files[idx];

  readfile(fileName).
    then(function(value) {
      assert.equal(value, 'Content of ' + files[idx] + '\n');
      idx++;
      loadfi(files, idx);
    }).catch(function (e) {
      // Note: assert cannot be used here, because exception cannot be thrown
      //       from Promise.prototype.catch
      if (e.message !== "Cannot open file!") {
        console.log('Error message does not match. Expected '
                    + '"Cannot open file!", but got "'
                    + e.message + '".');
        process.emitExit(1);
      } else if (idx !== 4) {
        console.log('Did not read every file. The expected number of files '
                    + 'read is "4", but got "' + idx + '".');
        process.emitExit(1);
      }
    });
};

var files = ['1.txt', '2.txt', '3.txt', '4.txt'];
loadfi(files, 0);
