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

var port = 41239;
var multicast_address = '230.255.255.250';
var interval = 100;

var recv_count = 0, send_count = 0;

var msg = 'Hello IoT.js';
var client = dgram.createSocket('udp4');
var server = dgram.createSocket('udp4');

server.on('error', function(err) {
  assert.fail();
  server.close();
});

server.on('message', function(data, rinfo) {
  console.log('server got data : ' + data);
  recv_count++;
  if (recv_count == 1) {
    server.dropMembership(multicast_address);
  }
});

server.bind(port, function() {
  server.addMembership(multicast_address);
});

var timer = setInterval(function () {
  send_count++;
  client.send(msg, port, multicast_address);
  if (send_count == 3) {
    clearInterval(timer);
  }
}, interval);

setTimeout(function() {
  server.close();
  client.close();
}, 1000);

process.on('exit', function(code) {
  assert.equal(code, 0);
  assert.equal(recv_count, 1);
  assert.equal(send_count, 3);
});
