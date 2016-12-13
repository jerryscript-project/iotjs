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
var util = require('util');


var timerFired = false;
var shouldnotFired = false;

setTimeout(function(a, b, c) {
  assert.equal(arguments.length, 3);
  assert.equal(arguments[0], 1);
  assert.equal(arguments[1], 2);
  assert.equal(arguments[2], 3);
  assert.equal(a, 1);
  assert.equal(b, 2);
  assert.equal(c, 3);
  timerFired = true;
}, 100, 1, 2, 3);


var i = 0;
setInterval(function(list) {
  assert.equal(arguments.length, 1);
  assert(util.isArray(list));
  assert.equal(list.length, 5);
  if (i >= list.length) {
    clearInterval(this);
  } else {
    assert.equal(list[i], i * i);
    i++;
  }
}, 100, [0, 1, 4, 9, 16]);


var t = setTimeout(function() {
  shouldnotFired = true;
}, 100);
clearTimeout(t);


process.on('exit', function(code) {
  assert.equal(code, 0);
  assert(timerFired);
  assert.equal(i, 5);
  assert.equal(shouldnotFired, false);
});
