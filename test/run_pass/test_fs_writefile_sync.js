
/* Copyright 2016-present Samsung Electronics Co., Ltd. and other contributors
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

 /*
  @STDOUT=Pass
 */

var fs = require('fs');
var assert = require('assert');

var file = process.cwd() + '/resources/test';
var buff1 = new Buffer('test string1');
var buff2 = null;
var str = 'test string2';
var num = 1;

if (process.platform === 'tizenrt') {
  file = '/mnt/test';
}

fs.writeFileSync(file, buff1);
buff2 = fs.readFileSync(file);
assert.equal(buff2.equals(buff1), true);

fs.writeFileSync(file, str);
buff2 = fs.readFileSync(file);
assert.equal(str.valueOf(), buff2.toString('utf8'));


fs.writeFileSync(file, num);
buff2 = fs.readFileSync(file);
assert.equal(num, parseInt(buff2.toString('utf8'), 10));

console.log('Pass');

fs.unlinkSync(file);
