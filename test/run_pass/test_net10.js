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

var net = require('net');
var assert = require('assert');

var timedout = false;
var connected = false;

// Try connect to host that is not exist (Reserved address of TEST-NET-1)
var socket1 = net.createConnection(11111, '192.0.2.1');

socket1.setTimeout(1000);

socket1.on('timeout', function() {
  timedout = true;
  socket1.destroy();
});

socket1.on('error', function() {
  assert.fail();
});

socket1.on('connect', function() {
  connected = true;
  socket1.destroy();
});

process.on('exit', function() {
  assert(timedout);
  assert(!connected);
});
