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

var assert = require('assert');
var json = require(process.cwd() + '/resources/test.json');

assert.equal(json.name, 'IoT.js');
assert.equal(json.author, 'Samsung Electronics Co., Ltd.');
assert.equal(json.license, 'Apache-2.0');
assert.equal(json.number, 123);
assert.equal(Array.isArray(json.array), true);
assert.equal(json.array.toString(), [1,2,3,4].toString());
