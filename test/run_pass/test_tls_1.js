/* Copyright 2018-present Samsung Electronics Co., Ltd. and other contributors
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
var fs = require('fs');
var tls = require('tls');

var tlsClientError_caught = false;
var socket_handshake_error_caught = false;

var port = 8080;

var server_options = {
  rejectUnauthorized: false,
  isServer: true
}

var server = tls.createServer(server_options, function(socket) {
}).listen(port, function() {});

server.on('tlsClientError', function(error) {
  tlsClientError_caught = error instanceof Error;
  server.close();
});

var socket_options = {
  host: '127.0.0.1',
  port: port,
  rejectUnauthorized: false
}

var socket = tls.connect(socket_options, function() {});

socket.on('error', function(error) {
  socket_handshake_error_caught = error instanceof Error;
});

function createServerFailed(server_options) {
  assert.throws(function() {
    return tls.createServer(server_options);
  });
}

createServerFailed({key: 0});
createServerFailed({cert: null});
createServerFailed({key: false, cert: 7});
createServerFailed({ca: true});

process.on('exit', function() {
  assert.equal(tlsClientError_caught, true);
  assert.equal(socket_handshake_error_caught, true);
});
