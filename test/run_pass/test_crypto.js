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

var crypto = require('crypto');
var assert = require('assert');

var hash = crypto.createHash('sha1');

assert.throws(function() { var err_hash = crypto.createHash('sadf'); });

hash.update('Hello IoT.js');

assert.equal(hash.digest('hex'), '4f5cf1945efb60f400c23172edb4e36b47f5a25e');
assert.throws(function() { hash.digest('hex'); });

var hash2 = crypto.createHash('sha1');
hash2.update('Hello IoT.js');
hash2.update(' v2');

assert.equal(hash2.digest('base64'), 'DBnLTkxZ70AgUzCjZ7FTv91AWZw=');
