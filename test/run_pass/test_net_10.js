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

var server = net.createServer();
var port = 22709;

var timedout = false;
var connected = false;

server.listen(port, 1);

server.on('connection', function(newSocket) {
  connected = true;
  console.log("Connected!");

  newSocket.on('error', function() {
    assert.fail();
  });

  newSocket.setTimeout(500);

  newSocket.on('timeout', function() {
    console.log("Timeout");
    timedout = true;
    newSocket.destroy();
    socket.destroy();
    server.close();
  });
});

var socket = net.createConnection(port);

socket.on('error', function() {
  assert.fail();
});

process.on('exit', function() {
  assert(timedout);
  assert(connected);
});
