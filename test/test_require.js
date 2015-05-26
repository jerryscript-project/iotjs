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


console.log("platform : " + process.platform);

var x = require("require_add");
console.log("require_add: " + x.add(1,4));
process.nextTick(function(){console.log("nextTick works!");});

var str = process.readSource("package.json");
var json = process.JSONParse(str);
console.log("package.json src:");
console.log(str);
console.log("package.json main: " + json.main);

var pkg = require('test_pkg');
console.log("package test: require('test_pkg').add(22,44)=" + pkg.add(22,44));
console.log("package test: require('test_pkg').multi(22,44)="
            + pkg.multi(22,44));
console.log("package test: require('test_pkg').add2(22,44)=" + pkg.add2(22,44));
