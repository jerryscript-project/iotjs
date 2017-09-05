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
var dgram = require('dgram');

var port = 41234;
var msg = 'Hello IoT.js';
var server = dgram.createSocket('udp4');
var recvMsg = "";
var recvCnt = 0;

server.on('error', function(err) {
  assert.fail();
  server.close();
});

server.on('message', function(data, rinfo) {
  console.log('server got data : ' + data);
  console.log('client address : ' + rinfo.address);
  console.log('client port : ' + rinfo.port);
  console.log('client family : ' + rinfo.family);
  recvMsg = data;
  recvCnt++;
  server.send(msg, rinfo.port, rinfo.address, function (err, len) {
    assert.equal(err, null);
    assert.equal(len, msg.length);
  });
  setTimeout(function() { server.close() }, 4000);
});


server.on('listening', function() {
  console.log('listening');
});

server.bind(port);

process.on('exit', function(code) {
  assert.equal(code, 0);
  assert.equal(recvCnt, 1);
  assert.equal(recvMsg, msg);
});
