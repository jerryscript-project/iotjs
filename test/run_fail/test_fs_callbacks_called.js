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


var assert = require('assert')
var invocation_count = 0;
var callback_count = 13;

var callback = function () {
    if (++invocation_count == callback_count) {
        assert.fail("pass") // All the callbacks were called
    }
}

var fs = require('fs');
fs.open("", "r", callback);
fs.close(0, callback);
fs.read(0, Buffer(1), 0, 0, 0, callback);
fs.write(0, Buffer(1), 0, 0, 0, callback);
fs.readFile("", callback);
fs.writeFile("", Buffer(1), callback);
fs.rename("", "", callback);
fs.stat("", callback);
fs.exists("", callback);
fs.unlink("", callback);
fs.mkdir("", callback);
fs.rmdir("", callback);
fs.readdir("", callback);
