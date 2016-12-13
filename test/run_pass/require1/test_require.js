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

assert.notEqual(process.platform, false);

var x = require("require_add");
assert.equal(x.add(1,4), 5);

var str = process.readSource("package.json");
var json = process.JSONParse(str);

assert.equal(json.version, "2.9.1");
assert.equal(json.name, "npm");
assert.equal(json.main, "./lib/npm.js");
assert.equal(json.repository.type, "git");

var pkg1 = require('test_pkg');
assert.equal(pkg1.add(22, 44), 66);
assert.equal(pkg1.multi(22, 44), 968);
assert.equal(pkg1.add2(22, 44), 66);

var pkg2 = require('test_index');
assert.equal(pkg2.add(22, 44), 66);
assert.equal(pkg2.multi(22, 44), 968);
assert.equal(pkg2.add2(22, 44), 66);
