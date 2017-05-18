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
var assert = require('assert');
var http = require('http');
var net = require('net');

var errors = 0;

// Create a TCP server
var server = net.createServer(function(socket) {
  socket.end('Bad http\r\n\r\n'); // This should trigger the Parse Error.
  server.close();
});

server.listen(3085, '127.0.0.1');

// Create a simple get request.
var request = http.request({
  host: '127.0.0.1',
  port: 3085,
  method: 'GET',
  path: '/'
});
request.on('error', function(err) {
  errors++;
});
request.end();

process.on('exit', function() {
  // The first error is a Parse Error.
  // The second error is the socket hang up.
  assert.equal(errors, 2);
});
