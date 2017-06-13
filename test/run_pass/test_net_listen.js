/* Copyright 2017-present Samsung Electronics Co., Ltd. and other contributors
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

var server_1 = net.createServer();
server_1.listen(22701, 5);

var server_2 = net.createServer();
server_2.address();
assert.throws(function() {server_2.listen("80"); }, Error);
var server_2Listener = server_2.listen(22701, 5);
assert.notEqual(server_2Listener, 0);
server_2Listener = server_2.listen(22701, "127.0.0.1");
assert.notEqual(server_2Listener, 0);

var socket = new net.Socket();
socket.on('error', function(err){
  assert.equal(err instanceof Error, true);
});
socket.connect(22701, "127.0.0.1");
server_2.close();
server_1.close();
