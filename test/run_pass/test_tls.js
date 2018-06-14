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

var tls = require('tls');
var assert = require('assert');
var fs = require('fs');

var port = 8080;

var server_closed = false;
var expected_client_msg = 'Client hello';
var expected_server_msg = 'Server hello';
var client_message = '';
var server_message = '';
var server_handshake_done = false;
var tlsClientError_caught = false;
var socket_handshake_error_caught = false;

var options = {
  key: fs.readFileSync(process.cwd() + '/resources/my_key.key').toString(),
  cert: fs.readFileSync(process.cwd() + '/resources/my_crt.crt'),
  rejectUnauthorized: false,
  isServer: true,
};

var server = tls.createServer(options, function(socket) {
  socket.write('Server hello');

  socket.on('data', function(data) {
    client_message += data.toString();
  });

}).listen(port, function() { });

server.on('secureConnection', function() {
  server_handshake_done = true;
});

server.on('close', function() {
  server_closed = true;
});

var error_caught = false;
var handshake_done = false;

var sockOpts = {
  host: '127.0.0.1',
  port: 8080,
  rejectUnauthorized: false,
}

var socket = tls.connect(sockOpts, function() {
});

socket.on('secureConnect', function(){
  handshake_done = true;
});

socket.on('end', function() {
  server.close();
});

socket.on('data', function(data) {
  server_message += data.toString();
  socket.write('Client hello');
  socket.end();
});

var socket2 = tls.connect({host: '127.123.123.123', port: 444}, function() {
  socket2.end();
});

socket2.on('error', function(err) {
  error_caught = true;
});

var nocert_options = {
  rejectUnauthorized: false,
  isServer: true,
}

var server2_port = 8081;

sockOpts = {
  host: '127.0.0.1',
  port: server2_port,
  rejectUnauthorized: false,
}

var server2 = tls.createServer(nocert_options, function(socket) {
}).listen(server2_port, function() { });

server2.on('tlsClientError', function(error) {
  tlsClientError_caught = error instanceof Error;
  server2.close();
});

var socket3 = tls.connect(sockOpts, function() { });

socket3.on('error', function(error) {
  socket_handshake_error_caught = error instanceof Error;
});

var server3 = tls.createServer(options, function(socket) {
   socket.write('Server hello');

   socket.on('data', function(data) {
     client_message = data.toString();
   });

}).listen(9090, function() { });

server3.on('secureConnection', function() {
 server_handshake_done = true;
});

server3.on('close', function() {
 server_closed = true;
});

var opt = {
   rejectUnauthorized: false,
}

var socket4 = tls.connect(9090);

socket4.on('secureConnect', function(){
  handshake_done = true;
});

socket4.on('data', function(data) {
  server_message = data.toString();
  socket4.write('Client hello');
  socket4.end();
});

var socket5 = tls.connect(9090, 'localhost');

socket5.on('secureConnect', function(){
  handshake_done = true;
});

socket5.on('data', function(data) {
  server_message = data.toString();
  socket5.write('Client hello');
  socket5.end();
});

var socket6 = tls.connect(9090, 'localhost', opt);

socket6.on('secureConnect', function(){
  handshake_done = true;
});

socket6.on('data', function(data) {
  server_message = data.toString();
  socket6.write('Client hello');
  socket6.end();
});

var socket7 = tls.connect(9090, 'localhost', opt, function() {
});

socket7.on('secureConnect', function(){
  handshake_done = true;
});

socket7.on('data', function(data) {
  server_message = data.toString();
  socket7.write('Client hello');
  socket7.end();
});

var socket8 = tls.connect(9090, 'localhost', function() {
});

socket8.on('secureConnect', function(){
 handshake_done = true;
});

socket8.on('data', function(data) {
 server_message = data.toString();
 socket8.write('Client hello');
 socket8.end();
});

var socket9 = tls.connect(9090, function() {
});

socket9.on('secureConnect', function(){
 handshake_done = true;
});

socket9.on('end', function() {
 server3.close();
});

socket9.on('data', function(data) {
 server_message = data.toString();
 socket9.write('Client hello');
 socket9.end();
});

options = {
};

function createServerFailed(options) {
  assert.throws(function() {
    return tls.createServer(options);
  });
}

createServerFailed({ key:0 });
createServerFailed({ cert:null });
createServerFailed({ key:false, cert:7 });
createServerFailed({ ca:true });

process.on('exit', function() {
  assert.equal(error_caught, true);
  assert.equal(handshake_done, true);
  assert.equal(server_handshake_done, true);
  assert.equal(client_message === expected_client_msg, true);
  assert.equal(server_message === expected_server_msg, true);
  assert.equal(server_closed, true);
  assert.equal(tlsClientError_caught, true);
  assert.equal(socket_handshake_error_caught, true);
});
