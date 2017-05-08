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

var module_cache = require('run_pass/require1/module_cache.js');
module_cache.i = 100;
module_cache = require('run_pass/require1/module_cache.js');

assert.equal(module_cache.i, 100);

{
  var test1 = require('run_pass/require1/node_modules/test_pkg');
  var test2 = require('run_pass/require1/node_modules/test_index');
}

assert.throws(function() {
  var test3 = require('run_pass/require1/babel-template');
}, Error);

assert.throws(function() {
  var test4 = require('tmp');
}, Error);
