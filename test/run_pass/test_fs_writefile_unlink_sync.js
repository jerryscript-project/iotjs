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

var fs = require('fs');
var assert = require('assert');

var file1 = "../resources/tobeornottobe.txt";
var file2 = "../resources/tobeornottobe_sync.txt";

/* make a new file2 from file1 */
var buf1 = fs.readFileSync(file1);
fs.writeFileSync(file2, buf1);

/* Does file2 exists ? */
var result = fs.existsSync(file2);
assert.equal(result, true);

/* Is file2 equal to file1 */
var buf2 = fs.readFileSync(file2);
assert(buf1.toString(), buf2.toString());

/* Remove file2 */
fs.unlinkSync(file2);

/* Is file2 removed? */
result = fs.existsSync(file2);
assert.equal(result, false);
