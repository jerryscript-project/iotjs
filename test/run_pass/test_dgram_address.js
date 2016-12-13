/* Copyright 2016-present Samsung Electronics Co., Ltd. and other contributors
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
var dgram = require('dgram');

var port = 41236;
var msg = 'Hello IoT.js';
var client = dgram.createSocket('udp4');
var server = dgram.createSocket('udp4');
var server_address, server_port, client_address, client_port;

server.on('error', function(err) {
  assert.fail();
  server.close();
});

server.on('message', function(data, rinfo) {
  var address = client.address();
  client_address = address.address;
  client_port = address.port;
  assert.equal('0.0.0.0', client_address);
  assert.equal(rinfo.port, client_port);
  server.send(msg, rinfo.port, 'localhost');
});

server.on('listening', function() {
  var address = server.address();
  server_address = address.address;
  server_port = address.port;
})

server.bind(port);

client.send(msg, port, 'localhost');

client.on('error', function(err) {
  assert.fail();
  client.close();
});

client.on('message', function(data, rinfo) {
  assert.equal('0.0.0.0', server_address);
  assert.equal(rinfo.port, server_port);
  client.close();
  server.close();
});

process.on('exit', function(code) {
  assert.equal(code, 0);
});
