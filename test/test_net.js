/* Copyright 2015 Samsung Electronics Co., Ltd.
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


var server = net.createServer();

server.listen(1234, 5);

server.on('connection', function(socket) {
  socket.on('data', function(data) {
    socket.end('Hello IoT.js');
  });
  socket.on('close', function() {
    server.close();
  });
});


var socket = new net.Socket();

socket.connect(1234, "127.0.0.1");
socket.end("Hello IoT.js");

socket.on('data', function(data) {
  console.log(data.toString());
});

