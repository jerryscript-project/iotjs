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

server.listen({port:1234}, function() {
  console.log("[Server] listening");
});

server.on('connection', function(socket) {
  console.log('[Server] a client connected');
  socket.on('read', function(err, buffer) {
    if (err) {
      console.log(err.toString());
    } else {
      console.log('[Server - Client] recv: ' + buffer.toString());
    }
  });
  socket.on('close', function() {
    console.log('[Server] a client disconnected');
  })
});


var socket = new net.Socket();

socket.connect(1234, "127.0.0.1", function(status) {
  if (status == 0) {
    console.log("[Client] socket connect");
  } else {
    socket.destroy();
  }
});

socket.on('close', function() {
  console.log('[Client] socket closed');
});

socket.write("hello", function(status) {
  console.log("[Client] after write - status: " + status);
});
