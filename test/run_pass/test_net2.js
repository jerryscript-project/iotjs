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

var port = 22702;

var server = net.createServer({
  allowHalfOpen: true
});

server.listen(port, 5);

server.on('connection', function(socket) {
  var msg = '';
  socket.on('data', function(data) {
    msg += data;
  });
  socket.on('end', function() {
    socket.end(msg);
    server.close();
  });
});


var socket = new net.Socket();
var echo_msg = '';

socket.connect(port, '127.0.0.1', function() {
  socket.end('Hello IoT.js');
});

socket.on('data', function(data) {
  echo_msg += data;
});

socket.on('end', function() {
  assert.equal(echo_msg, "Hello IoT.js");
});
