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
var port = 22704;
var timeout = 1000;
var writeat = 2000;

server.listen(port, 5);

server.on('connection', function(socket) {
  socket.setTimeout(timeout, function() {
    socket.end();
  });
  socket.on('data', function(data) {
    socket.end();
  });
  socket.on('close', function() {
    server.close();
  });
});


var socket = new net.Socket();

socket.connect(port, "127.0.0.1");

socket.on('connect', function() {
  timers.setTimeout(function() {
    assert.throws(
      function() {
        socket.write('Hello IoT.js');
      }, Error);
  }, writeat);
});

process.on('exit', function(code) {
  assert.equal(code, 0);
});
