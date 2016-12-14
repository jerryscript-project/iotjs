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
var timers = require('timers');


var server = net.createServer();
var port = 22706;

server.listen(port, 5);

server.on('connection', function(socket) {
  socket.on('data', function(data) {
    msg += data;
    socket.end();
  });
  socket.on('close', function() {
    server.close();
  });

  // as soon as connection established, pause the socket
  socket.pause();

  // resume after 2 secs
  timers.setTimeout(function() {
    socket.resume();
  }, 2000);
});


var socket = new net.Socket();
var msg = "";

socket.connect(port, "127.0.0.1");
socket.on('connect', function() {
  // client writes "1" first, but server is paused for 2 secs
  // server gets "1" after 2 secs
  socket.write("1");

  // "2" is appended to msg before "1"
  timers.setTimeout(function() {
    msg += "2";
  }, 1000);
});

socket.on('end', function() {
  socket.end();
});

process.on('exit', function(code) {
  assert.equal(code, 0);
  assert.equal(msg, "21");
});
