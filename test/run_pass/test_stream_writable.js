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

var Writable = require('stream').Writable;
var writable = new Writable({highWaterMark: 0});

var writable2 = Writable({highWaterMark: 0});

assert.throws(function() { writable._write(0, 0, 0); }, Error);

writable._writableState.writing = true;
writable._writableState.needDrain = false;
writable._readyToWrite();

writable.end('', function(){});

writable2._writableState.ending = true;
writable2.end('', function(){});

var writable3 = new Writable({highWaterMark: 0});
writable3._writableState.buffer.length=0;
writable3._writableState.ending = true;
writable3._writableState.ended = true;
writable3._readyToWrite();


process.exit(0);
