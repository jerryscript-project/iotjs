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

var trace = '';

process.on('uncaughtException', function (err) {
  trace += 'A';
});

process.on('uncaughtException', function (err) {
  trace += 'B';
});

process.on('exit', function() {
  trace += 'C';
  process.removeAllListeners('uncaughtException');
  assert.equal(trace, 'ABABC');
});


process.nextTick(function() {
  assert.fail();
});

assert.fail();
