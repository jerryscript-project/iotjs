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

var json_file = process.cwd() + "/resources/process/package.json";

// Load a JSON file.
var str = process.readSource(json_file);
var json = JSON.parse(str);

assert.equal(json.version, "2.9.1");
assert.equal(json.name, "npm");
assert.equal(json.main, "./lib/npm.js");
assert.equal(json.repository.type, "git");
