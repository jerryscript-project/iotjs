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
var fs = require('fs');

var dynamicmodule_dir = "dynamicmodule/build/" + process.platform + "/";
var dynamicmodule_name = "dynamicmodule";
var dynamicmodule_path = dynamicmodule_dir + dynamicmodule_name + ".iotjs";

assert.assert(fs.existsSync(dynamicmodule_path),
              "Dynamic module does not exists: " + dynamicmodule_path);

var testmodule = require(dynamicmodule_path);
assert.equal(testmodule.demokey, 3.4);
assert.equal(testmodule.counter, 1);
testmodule.new_value = "hello";

/* Loading the module via a differnt key.
 * This should not initialize the module again.
 */
var load_2 = require(dynamicmodule_dir + dynamicmodule_name)
assert.equal(load_2.demokey, 3.4);
/* if the counter is not one then the module initializer was invoked again. */
assert.equal(load_2.counter, 1);
assert.equal(load_2.new_value, "hello");
