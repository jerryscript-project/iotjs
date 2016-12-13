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
var port = 1236;

server.listen(port, 5);

server.on('connection', function(socket) {
  socket.on('data', function(data) {
    socket.end('Hello IoT.js');
  });
  socket.on('close', function() {
    server.close();
  });
});


var socket = new net.Socket();
var msg = "";
var lookupHandled = false;

socket.on('lookup', function(err, ip, family) {
  lookupHandled = true;
});

socket.connect(port, "localhost");  // connect with hostname
socket.write("Hello IoT.js");

socket.on('data', function(data) {
  msg += data;
});

socket.on('end', function() {
  socket.end();
});

process.on('exit', function(code) {
  assert.equal(code, 0);
  assert.equal(msg, "Hello IoT.js");
  assert.equal(lookupHandled, true);
});
