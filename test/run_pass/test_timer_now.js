/* Copyright 2015 Samsung Electronics Co., Ltd.
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
  @TIMEOUT=5
*/

var assert = require('assert');
var Timer = process.binding(process.binding.timer);

var startTime = 0;
var endTime = 0;
var delay = 1000;

startTime = Timer.now();
var timerA = setTimeout(function() {
  endTime = Timer.now();
}, delay);


process.on('exit', function(code) {
  assert.equal(code, 0);
  assert((endTime-startTime) > delay);
});
