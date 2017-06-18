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
var Duplex = require('stream').Duplex;

var duplex = Duplex({ highWaterMark: 0 });

assert(duplex instanceof Duplex);
assert(duplex.write);
assert(duplex.read);

var msg1 = 'message 1';
var msg2 = 'message 2';

duplex._write = function(chunk, callback) {
  assert.equal(chunk == msg1, true);
  duplex.push(msg2);
  duplex.end();
}
duplex._readyToWrite();
duplex.write(msg1);

process.on('exit', function() {
  assert.equal(duplex.read() == msg2, true);
});
