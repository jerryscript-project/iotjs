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


var timerACnt = 0;
var timerBCnt = 0;
var timerSequence = '';

var timerA = setTimeout(function() {
  timerSequence += 'A';
  timerACnt++;
}, 1000);

var timerB = setInterval(function() {
  timerSequence += 'B';
  timerBCnt++;
  if (timerBCnt > 5) {
    clearInterval(timerB);
  }
}, 300);

process.on('exit', function(code) {
  assert.equal(code, 0);
  assert.equal(timerACnt, 1);
  assert.equal(timerBCnt, 6);
  assert.equal(timerSequence.length, 7);
});
