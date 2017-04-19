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
var EventEmitter = require('events').EventEmitter;


var uncaught_error = false;

process.on('uncaughtException', function(err) {
  assert.equal(err.message, "Uncaught 'error' event");
  uncaught_error = true;
});

process.on('exit', function(code) {
  process.removeAllListeners('uncaughtException');
  assert.equal(code, 0);
  assert(uncaught_error);
});

var ee = new EventEmitter();
ee.emit('error');
