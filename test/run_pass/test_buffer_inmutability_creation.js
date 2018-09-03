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

/* Related issue: https://github.com/Samsung/iotjs/issues/1379 */

var assert = require('assert');

/* The global Buffer by default is a function */
assert.strictEqual(typeof(Buffer), "function");

var backup_buffer = Buffer;

/* Modify the global Buffer */
Buffer++;

/**
 * The ++ operation will change the value of the "Buffer" variable.
 * Thus the type shouldn't be a function now.
 */
assert.notStrictEqual(typeof(Buffer), "function");

/**
 * Still the creation of buffer should work.
 * Using an already saved buffer reference
 */
var new_buffer = backup_buffer("OK");

assert.equal(new_buffer.length, 2);
assert.equal(new_buffer.toString(), "OK");
