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

var port = 41234;
var msg = 'Hello IoT.js';
var server = dgram.createSocket('udp4');

server.on('error', function(err) {
  assert.fail();
  server.close();
});

server.on('message', function(data, rinfo) {
  console.log('server got data : ' + data);
  console.log('client address : ' + rinfo.address);
  console.log('client port : ' + rinfo.port);
  console.log('client family : ' + rinfo.family);
  assert.equal(data, msg);
  server.send(msg, rinfo.port, 'localhost', function (err, len) {
    assert.equal(err, null);
    assert.equal(len, msg.length);
    server.close();
  });
});

server.on('listening', function() {
  console.log('listening');
});

server.bind(port);

var client = dgram.createSocket('udp4');

client.send(msg, port, 'localhost', function(err, len) {
  assert.equal(err, null);
  assert.equal(len, msg.length);
});

client.on('error', function(err) {
  assert.fail();
  client.close();
});

client.on('message', function(data, rinfo) {
  console.log('client got data : ' + data);
  console.log('server address : ' + rinfo.address);
  console.log('server port : ' + rinfo.port);
  console.log('server family : ' + rinfo.family);
  assert.equal(port, rinfo.port);
  assert.equal(data, msg);
  client.close();
});

process.on('exit', function(code) {
  assert.equal(code, 0);
});
