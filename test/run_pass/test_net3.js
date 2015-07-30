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
var assert = require('assert');


var port = 9831;

var server = net.createServer();

server.listen(port, 5);

server.on('connection', function(socket) {
  var i = 0;
  var limit = 1000;
  var writing = function() {
    var ok;
    do {
      ok = socket.write("" + (i % 10));
      if (++i == limit) {
        socket.end();
        server.close();
        ok = false;
      }
    } while (ok);
  };
  socket.on('drain', writing);
  writing();
});


var socket = new net.Socket();
var msg = '';

socket.connect(port, '127.0.0.1');

socket.on('data', function(data) {
  msg += data;
});

process.on('exit', function(code) {
  assert(msg.length === 1000);
});
