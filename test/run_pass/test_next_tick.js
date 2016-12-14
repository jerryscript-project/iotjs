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

var assert = require('assert');


var tickTrace = "";

process.nextTick(function() {
  tickTrace += "1";
  process.nextTick(function() {
    tickTrace += "2";
    process.nextTick(function() {
      tickTrace += "3";
      process.nextTick(function() {
        tickTrace += "4";
        process.nextTick(function() {
          tickTrace += "5";
        });
      });
    });
  });
});


process.on('exit', function(code) {
  assert.equal(code, 0);
  assert.equal(tickTrace, "12345");
});
