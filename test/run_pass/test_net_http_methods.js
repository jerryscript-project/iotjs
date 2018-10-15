/* Copyright 2018-present Samsung Electronics Co., Ltd. and other contributors
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
var http = require('http');

assert(http.METHODS instanceof Array, 'http.METHODS should be an array');

for (var idx in http.METHODS) {
  assert(typeof(http.METHODS[idx]) === 'string',
         'Elements of the http.METHODS should be strings. ' +
         'Found an invalid element, index: ' + idx);
}

/* Test if at least the basic HTTP methods should be supported */
var main_methods = ['GET', 'POST', 'PUT', 'DELETE', 'HEAD'];
for (var idx in main_methods) {
  var method_name = main_methods[idx];
  assert.notEqual(http.METHODS.indexOf(method_name), -1,
                 'http.METHODS is missing the value: ' + method_name);
}
