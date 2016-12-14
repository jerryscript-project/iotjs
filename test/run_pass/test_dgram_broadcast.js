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

var port = 41237;
var broadcast_address = '255.255.255.255';
var interval = 100;

var msg_count = 0, msg_count2 = 0, msg_count3 = 0, send_count = 0;

var msg = 'Hello IoT.js';
var socket = dgram.createSocket({type: 'udp4', reuseAddr: true});
var socket2 = dgram.createSocket({type: 'udp4', reuseAddr: true});
var socket3 = dgram.createSocket({type: 'udp4', reuseAddr: true});

socket.on('error', function(err) {
  assert.fail();
  socket.close();
});

socket2.on('error', function(err) {
  assert.fail();
  socket2.close();
});

socket3.on('error', function(err) {
  assert.fail();
  socket3.close();
});

socket.on('message', function(data, rinfo) {
  console.log('socket got data : ' + data);
  msg_count++;
  if (msg_count == 3) {
    socket.close();
  }
});

socket2.on('message', function(data, rinfo) {
  console.log('socket2 got data : ' + data);
  msg_count2++;
  if (msg_count2 == 3) {
    socket2.close();
  }
});

socket3.on('message', function(data, rinfo) {
  console.log('socket3 got data : ' + data);
  msg_count3++;
  if (msg_count3 == 3) {
    socket3.close();
  }
});

socket.bind(port, function() {
  socket.setBroadcast(true);
  var timer = setInterval(function () {
    send_count++;
    socket.send(msg, port, broadcast_address);
    if (send_count == 3) {
      clearInterval(timer);
    }
  }, interval);
});

socket2.bind(port);

socket3.bind(port);

process.on('exit', function(code) {
  assert.equal(code, 0);
  assert.equal(msg_count, 3);
  assert.equal(msg_count2, 3);
  assert.equal(msg_count3, 3);
  assert.equal(send_count, 3);
});
