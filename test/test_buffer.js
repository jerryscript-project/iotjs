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

var assert = require('assert');

var buff1 = new Buffer("test");
assert.equal(buff1.toString(), "test");
assert.equal(buff1.length, 4);

var buff2 = new Buffer(10);
buff2.write("abcde");
assert.equal(buff2.toString(), "abcde");
assert.equal(buff2.length ,10);

buff2.write("fgh", 5);
assert.equal(buff2.toString(), "abcdefgh");
assert.equal(buff2.length ,10);

var buff3 = Buffer.concat([buff1, buff2]);
assert.equal(buff3.toString(), "testabcdefgh");
assert.equal(buff3.length ,14);
